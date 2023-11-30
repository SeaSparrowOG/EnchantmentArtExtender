#include "singletonHolder.h"
#include "configParser.h"
#include "helperFunctions.h"
#include "artSwap.h"

namespace ConfigParser {

	enum ConfigErrorFields {
		Empty,
		NeedsUpdate,
		UnexpectedFormat,
		Garbage,
		ArtErrors,
		MissingMasters,
		MissingWeapons
	};
	
	struct ConfigError {
		bool                     isCorrupted = false;      //I use a directory iterator, which throws this error.
		bool                     isEmpty = false;
		bool                     requiresUpdate = false;   //Config files may specify a minimum version.
		std::string              configName;       //Specified when the file is read
		std::string              unexpectedFormat; //Field contains bad value, such as object when we want string etc. 
		std::vector<std::string> garbage;          //Unexpected fields, or defining fields when you don't need to.
		std::vector<std::string> artErrors;        //Spell art is either missing, or (in the future) it won't show in game.
		std::vector<std::string> missingMasters;   //Required field specifies a mod that is not present in the load order.
		std::vector<std::string> missingWeapons;   //Required field specifies a weapon that does not exist.

		ConfigError() {
			this->isCorrupted = false;
			this->isEmpty = false;
			this->requiresUpdate = false;
			this->configName = "";
			this->unexpectedFormat = "";
			this->garbage = std::vector<std::string>();
			this->artErrors = std::vector<std::string>();
			this->missingMasters = std::vector<std::string>();
			this->missingWeapons = std::vector<std::string>();
		}

		//Prints the error log in the log file. Formats the string slightly (indentation, word wrap in the future).
		void LogErrors(ConfigErrorFields a_errorField) {
			switch (a_errorField) {
			case(Garbage):
				for (auto message : garbage) _logger::info("    {}", message);
				break;
			case(ArtErrors):
				for (auto message : artErrors) _logger::info("    {}", message);
				break;
			case(MissingMasters):
				for (auto message : missingMasters) _logger::info("    {}", message);
				break;
			case(MissingWeapons):
				for (auto message : missingWeapons) _logger::info("    {}", message);
				break;
			default:
				break;
			}
		}

		//Called to add stuff to the appropriate field.
		void AddError(ConfigErrorFields a_errorField, std::string a_message) {
			switch (a_errorField) {
			case(Garbage):
				if (garbage.empty()) garbage.push_back(">" + configName);
				garbage.push_back("    " + a_message);
				break;
			case(ArtErrors):
				if (garbage.empty()) artErrors.push_back(">" + configName);
				artErrors.push_back("    " + a_message);
				break;
			case(MissingMasters):
				if (garbage.empty()) missingMasters.push_back(">" + configName);
				missingMasters.push_back("    " + a_message);
				break;
			case(MissingWeapons):
				if (garbage.empty()) missingWeapons.push_back(">" + configName);
				missingWeapons.push_back("    " + a_message);
				break;
			default:
				break;
			}
		}
	};

	//Returns if the config is valid (all mods present and version code correct).
	bool IsConfigValid(std::filesystem::path a_path, ConfigError* a_errorHolder) {
		
		std::ifstream rawJSON(a_path.string());
		Json::Reader  JSONReader;
		Json::Value   JSONFile;
		JSONReader.parse(rawJSON, JSONFile);

		//Empty config. Ignore and stop parsing.
		if (JSONFile.empty()) {
			a_errorHolder->isEmpty = true;
			return false;
		}

		//Config that requires a new version of the mod. Ignore and stop parsing.
		int minVersion;

		try {
			minVersion = JSONFile["MinimumEnchanterVersion"].asInt();
		}
		catch (Json::Exception e) {
			a_errorHolder->AddError(Garbage, "Could not read the minimum version required.");
			return false;
		}

		if (minVersion > SingletonHolder::ConditionHolder::GetSingleton()->GetVersion()) {
			a_errorHolder->requiresUpdate = true;
			return false;
		}

		//From here: Try to parse all errors.
		bool validConfig = true;

		//Special flag error check.
		Json::Value artType;

		try {
			artType = JSONFile["SpecialFlag"].asString();

			if (!(artType.empty() || artType == "Additive" || artType == "Exclusive" || artType == "Low Priority")) {
				a_errorHolder->AddError(Garbage, "Unexpected value in SpecialFlag.");
				validConfig =  false;
			}
		}
		catch (Json::Exception e) {
			a_errorHolder->AddError(Garbage, "Exception caught while trying to read SpecialFlag.");
			validConfig =  false;
		}

		//Enchantment Keywords error check.
		Json::Value enchantmentKeywords;

		try {
			enchantmentKeywords = JSONFile["EnchantmentKeywords"];

			if (!enchantmentKeywords.isArray()) {
				a_errorHolder->AddError(Garbage, "EnchantmentKeywords is not an array.");
				validConfig =  false;
			}
		}
		catch (Json::Exception e) {
			a_errorHolder->AddError(Garbage, "Exception caught while trying to read EnchantmentKeywords");
			validConfig =  false;
		}

		//This variable is reset so we don't add the config name several times. Gets reset between loops.
		bool addedConfigName = false;
		std::string lastEntry = "";

		for (auto field : enchantmentKeywords) {
			if (!field.isString()) {
				a_errorHolder->AddError(Garbage, "An entry in EnchantmentKeywords is not a string. Last entry was: " + lastEntry);
				validConfig =  false;
				break;
			}
			lastEntry = field.asString();
		}

		//artSource error check.
		Json::String artSourceMod;

		try {
			artSourceMod = JSONFile["ArtSource"].asString();

			if (!(helperFunction::IsModPresent(artSourceMod))) {
				a_errorHolder->AddError(MissingMasters, "Required mod " + artSourceMod + " not found.");
				validConfig = false;
			}
		}
		catch (Json::Exception e) {
			a_errorHolder->AddError(Garbage, "Could not read ArtSource field.");
			validConfig =  false;
		}


		//From here, we are evaluating each swap within the config file.
		Json::Value swapField;

		try {
			swapField = JSONFile["SwapData"];
			if (!swapField.isArray()) {
				a_errorHolder->AddError(Garbage, "SwapData is not an array.");
				return false;
			}
		}
		catch (Json::Exception e) {
			a_errorHolder->AddError(Garbage, "Could not read the SwapData field.");
			return false;
		}

		for (auto swaps : swapField) {
			for (auto swap : swaps) {
				try {
					if (!swap.isObject()) {
						a_errorHolder->AddError(Garbage, "Unexpected value found in a swap definition. Expected an object.");
						continue;
					}

					//Check required weapons.
					Json::Value requiredWeapons;

					if (swap["WeaponID"]) {
						try {
							requiredWeapons = JSONFile["WeaponID"];
						}
						catch (Json::Exception e) {
							a_errorHolder->AddError(Garbage, "Could not read WeaponID");
							validConfig = false;
							continue;
						}

						std::vector<Json::Value> validWeapons;

						for (auto field : requiredWeapons) {
							if (!field.isObject()) {
								a_errorHolder->AddError(Garbage, "Unexpected value in a WeaponID swap. Expected an object.");
								validConfig = false;
								break;
							}
							validWeapons.push_back(field);
						}

						for (auto field : validWeapons) {
							if (!field.isObject()) {
								a_errorHolder->AddError(Garbage, "Unexpected value in a WeaponID swap. Expected an object.");
								validConfig = false;
								continue;
							}

							auto modNames = field.getMemberNames();

							if (modNames.empty()) {
								a_errorHolder->AddError(Garbage, "A WeaponID swap does not contain any specified weapons.");
								validConfig = false;
								break;
							}

							if (modNames.size() > 1) {
								a_errorHolder->AddError(Garbage, "A WeaponID swap contains too many specified weapons.");
								validConfig = false;
								break;
							}

							if (!helperFunction::IsModPresent(modNames.at(0))) {
								a_errorHolder->AddError(Garbage, "Required master " + modNames.at(0) + " is not present.");
								validConfig = false;
							}
							else {
								RE::TESObjectWEAP* foundWeap = helperFunction::GetWeaponFromID(helperFunction::GetFormIDFromString(field[modNames.at(0)].asString()), modNames.at(0));
								if (!foundWeap) {
									a_errorHolder->AddError(MissingWeapons, "FormID " + field[modNames.at(0)].asString() + " in mod " + modNames.at(0) + " is not a weapon.");
									validConfig = false;
								}
							}
						}
					}

					//Check ignored weapons.
					Json::Value ignoredWeapons;
					if (swap["IgnoredWeaponID"]) {
						try {
							requiredWeapons = JSONFile["IgnoredWeaponID"];
						}
						catch (Json::Exception e) {
							a_errorHolder->AddError(Garbage, "Could not read IgnoredWeaponID");
							validConfig = false;
							continue;
						}

						std::vector<Json::Value> validWeapons;

						for (auto field : requiredWeapons) {
							if (!field.isObject()) {
								a_errorHolder->AddError(Garbage, "Unexpected value in a IgnoredWeaponID swap. Expected an object.");
								validConfig = false;
								continue;
							}
							validWeapons.push_back(field);
						}

						for (auto field : validWeapons) {
							if (!field.isObject()) {
								a_errorHolder->AddError(Garbage, "Unexpected value in a IgnoredWeaponID swap. Expected an object.");
								validConfig = false;
								continue;
							}

							auto modNames = field.getMemberNames();

							if (modNames.empty()) {
								a_errorHolder->AddError(Garbage, "A IgnoredWeaponID swap does not contain any specified weapons.");
								validConfig = false;
								break;
							}

							if (modNames.size() > 1) {
								a_errorHolder->AddError(Garbage, "A IgnoredWeaponID swap contains too many specified weapons.");
								validConfig = false;
								break;
							}

							if (!helperFunction::IsModPresent(modNames.at(0))) {
								continue;
							}

							RE::TESObjectWEAP* foundWeap = helperFunction::GetWeaponFromID(helperFunction::GetFormIDFromString(field[modNames.at(0)].asString()), modNames.at(0));
							if (!foundWeap) {
								a_errorHolder->AddError(MissingWeapons, "FormID " + field[modNames.at(0)].asString() + " in mod " + modNames.at(0) + " is not a weapon.");
								validConfig = false;
							}
						}
					}

					//Check weapon keywords, if there are no required weapons.
					Json::Value weaponKeywords = swap["WeaponKeywords"];

					if (requiredWeapons && weaponKeywords) {
						a_errorHolder->AddError(Garbage, "WeaponIDs were specified, but so were weapon keywords.");
						validConfig = false;
					}
					else if (weaponKeywords) {
						if (!weaponKeywords.isArray()) {
							a_errorHolder->AddError(Garbage, "WeaponKeywords is not an array.");
							validConfig = false;
							continue;
						}

						for (auto keyword : weaponKeywords) {
							if (!keyword.isString()) {
								a_errorHolder->AddError(Garbage, "A keyword in WeaponKeywords is not a string.");
								validConfig = false;
								continue;
							}
						}
					}

					//Check ignored weapon keywords, if there are no required weapons.
					Json::Value ignoredWeaponKeywords = swap["IgnoredKeywords"];

					if (requiredWeapons && ignoredWeaponKeywords) {
						a_errorHolder->AddError(Garbage, "IgnoredKeywords were specified, but so were weapon keyword exclusions.");
						validConfig = false;
					}
					else if (ignoredWeaponKeywords) {
						if (!ignoredWeaponKeywords.isArray()) {
							a_errorHolder->AddError(Garbage, "IgnoredKeywords is not an array.");
							validConfig = false;
							continue;
						}

						for (auto keyword : ignoredWeaponKeywords) {
							if (!keyword.isString()) {
								a_errorHolder->AddError(Garbage, "A keyword in IgnoredKeywords is not a string.");
								validConfig = false;
								continue;
							}
						}
					}

					//Final check - check the arts.
					Json::Value artLeftAbility;
					if (!swap["Left"]) {
						a_errorHolder->AddError(Garbage, "Left art field is missing.");
						validConfig = false;
					}
					else {
						artLeftAbility = swap["Left"];
						if (!artLeftAbility.isString()) {
							a_errorHolder->AddError(Garbage, "Left ability is not a string.");
							validConfig = false;
						}
						else {
							try {
								RE::SpellItem* leftAbility = helperFunction::GetSpellFromFormID(helperFunction::GetFormIDFromString(artLeftAbility.asString()), artSourceMod);
								if (!leftAbility) {
									a_errorHolder->AddError(ArtErrors, "Left Ability is not a spell.");
									validConfig = false;
								}
							}
							catch (Json::Exception e) {
								a_errorHolder->AddError(Garbage, "Caught an exception while trying to read the Left art swap source.");
								validConfig = false;
							}
						}
					}

					Json::Value artRightAbility;
					if (!swap["Right"]) {
						a_errorHolder->AddError(Garbage, "Right art field is missing.");
						validConfig = false;
					}
					else {
						artRightAbility = swap["Right"];
						if (!artRightAbility.isString()) {
							a_errorHolder->AddError(Garbage, "Right ability is not a string.");
							validConfig = false;
						}
						else {
							try {
								RE::SpellItem* rightAbility = helperFunction::GetSpellFromFormID(helperFunction::GetFormIDFromString(artRightAbility.asString()), artSourceMod);
								if (!rightAbility) {
									a_errorHolder->AddError(ArtErrors, "Right Ability is not a spell.");
									validConfig = false;
								}
							}
							catch (Json::Exception e) {
								a_errorHolder->AddError(Garbage, "Caught an exception while trying to read the Right art swap source.");
								validConfig = false;
							}
						}
					}
				}
				catch (Json::Exception e) {
					a_errorHolder->AddError(Garbage, "If you see this, remove this config or fix, idk what broke and I am not writing more if statements.");
					validConfig = false;
				}
			}
		}
		return validConfig;
	}

	//Called externally, parses all valid configs.
	void ParseConfigs() {
		_logger::info("------------------------------------->    Report   <--------------------------------------");
		_logger::info("Begining reading config files.");
		const std::filesystem::path configDirectory{ R"(.\Data\Enchantment Effects Extender)" };
		std::vector<ConfigError> caughtErrors;

		if (!std::filesystem::exists(configDirectory)) {
			_logger::info("No config directory found in the Data folder.");
			return;
		}

		std::vector<std::string> validConfigs = std::vector<std::string>();
		int configCount = 0;
		std::string lastRead = "";

		try {
			for (const auto& file : std::filesystem::directory_iterator(configDirectory)) {
				if (file.path().extension() != ".json") {
					continue;
				}

				std::string configName = file.path().filename().string();
				lastRead = configName;
				++configCount;
				ConfigError tempConfigError = ConfigError();
				tempConfigError.configName = configName;
				tempConfigError.isCorrupted = false;
				tempConfigError.isEmpty = false;
				tempConfigError.requiresUpdate = false;

				if (!IsConfigValid(file, &tempConfigError)) {
					caughtErrors.push_back(tempConfigError);
					continue;
				}
				validConfigs.push_back(file.path().filename().string());	

				std::ifstream rawJSON(file.path().string());
				Json::Reader  JSONReader;
				Json::Value   JSONFile;
				JSONReader.parse(rawJSON, JSONFile);
				Json::Value swapData = JSONFile["SwapData"];

				for (auto swap : swapData) {
					ArtSwap::ArtSwap newSwap = ArtSwap::ArtSwap();
					std::vector<std::string> enchantmentKeywords;
					auto swapInfo = swap.getMemberNames().at(0);

					for (auto keyword : JSONFile["EnchantmentKeywords"]) enchantmentKeywords.push_back(keyword.asString());
					RE::SpellItem* leftAbility = helperFunction::GetSpellFromFormID(helperFunction::GetFormIDFromString(swap[swapInfo]["Left"].asString()), JSONFile["ArtSource"].asString());
					RE::SpellItem* rightAbility = helperFunction::GetSpellFromFormID(helperFunction::GetFormIDFromString(swap[swapInfo]["Right"].asString()), JSONFile["ArtSource"].asString());

					newSwap.SetAttributes(swap, JSONFile["SpecialFlag"].asString(), JSONFile["ArtSource"].asString(), enchantmentKeywords, leftAbility, rightAbility);
					SingletonHolder::ConditionHolder::GetSingleton()->Register(newSwap);
				}
			}
		}
		catch (std::exception e) {
			_logger::error("Critical error <{}> caught while trying to read config files. Report to the author of Enchantment Effects Extender.", e.what());
			_logger::error("Assuming broken application state. Configs will not be parsed, and no arts will be distributed.");
			_logger::error("Managed to read {} configs before failure. Last config read was {}.", configCount, lastRead);
			return;
		}
		_logger::info("Finished reading config files.");
		_logger::info("Read {} potential files.", configCount);
		_logger::info("Found {} valid files:", validConfigs.size());
		for (auto success : validConfigs) {
			_logger::info("    >{}", success);
		}

		_logger::info("Number of patches applied: {}.", SingletonHolder::ConditionHolder::GetSingleton()->GetNumberOfPatches());
		_logger::info("");
		SingletonHolder::ConditionHolder::GetSingleton()->CreateWeaponCache();
		_logger::info("");
		_logger::info("--------------------------------->    End of Report    <-----------------------------------");
		_logger::info("");
		_logger::info("");

		if (!caughtErrors.empty()) {
			for (auto error : caughtErrors) {
				error.LogErrors(UnexpectedFormat);
				error.LogErrors(Garbage);
				error.LogErrors(ArtErrors);
				error.LogErrors(MissingMasters);
				error.LogErrors(MissingWeapons);
			}
		}
		_logger::info("Enjoy your game!");
	}
}
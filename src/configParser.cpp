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
		return true;
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

					newSwap.SetAttributes(swap[swapInfo], JSONFile["SpecialFlag"].asString(), JSONFile["ArtSource"].asString(), enchantmentKeywords, leftAbility, rightAbility);
					newSwap.SetName(swapInfo);
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
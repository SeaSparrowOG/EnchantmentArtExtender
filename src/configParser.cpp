#include "configParser.h"
#include "singletonHolder.h"

namespace ConfigParser {

	//Used to hold the errors caught while parsing conflicts.
	struct ParseConfigsErrors {
		bool                     errorsExist;
		std::vector<std::string> caughtErrors;
		std::vector<std::string> caughtGarbage;
		std::vector<std::string> incompatibleConfigs;
		std::vector<std::string> invalidConfigs;
		std::vector<std::string> missingMasters;
		std::vector<std::string> invalidArts;

		void LogMessages() {
			_logger::info("----------------------------------------> Errors <-----------------------------------------");

			if (!caughtErrors.empty()) {
				_logger::info("    Exceptions should not be ignored. if an exception is caught,");
				_logger::info("    it is best to remove the config file it came from.");
				_logger::info("");
				_logger::info("    Exceptions:");
				for (auto caughtError : caughtErrors) {
					_logger::info("        {}", caughtError);
				}
				if (!caughtGarbage.empty()) _logger::info("    ========================================================================");
				_logger::info("");
			}

			if (!caughtGarbage.empty()) {
				_logger::info("    \"Garbage\" are considered any non-JSON files");
				_logger::info("    in the config directory.");
				_logger::info("");
				_logger::info("    Garbage:");
				for (auto garbage : caughtGarbage) {
					_logger::info("        {}", garbage);
				}
				if (!incompatibleConfigs.empty()) _logger::info("    ========================================================================");
				_logger::info("");
			}

			if (!incompatibleConfigs.empty()) {
				_logger::info("    Incompatible configs are configs that expect");
				_logger::info("    a newer version of Enchantment Art Extender than is installed.");
				_logger::info("");
				_logger::info("    Affected Configs:");
				for (auto invalidMessage : incompatibleConfigs) {
					_logger::info("        {}", invalidMessage);
				}
				if (!invalidConfigs.empty()) _logger::info("    ========================================================================");
				_logger::info("");
			}

			if (!invalidConfigs.empty()) {
				_logger::info("    Invalid configs will be ignored.");
				_logger::info("    You should report this to the mod author.");
				_logger::info("");
				_logger::info("    Affected Configs:");
				for (auto invalidMessage : invalidConfigs) {
					_logger::info("        {}", invalidMessage);
				}
				if (!missingMasters.empty()) _logger::info("    ========================================================================");
				_logger::info("");
			}

			if (!missingMasters.empty()) {
				_logger::info("    Missing masters are user-error. It means that a conflict");
				_logger::info("    expects a certain mod to be loaded, but it is missing.");
				_logger::info("");
				_logger::info("    Affected Configs:");
				for (auto invalidMessage : missingMasters) {
					_logger::info("        {}", invalidMessage);
				}
				if (!invalidArts.empty()) _logger::info("    ========================================================================");
				_logger::info("");
			}

			if (!invalidArts.empty()) {
				_logger::info("    Invalid arts are mod-author error. It means that the FormID provided");
				_logger::info("    for the art either does not exist, or does not point to a valid ability.");
				_logger::info("");
				_logger::info("    Affected Configs:");
				for (auto invalidMessage : invalidArts) {
					_logger::info("        {}", invalidMessage);
				}
				_logger::info("");
			}

			_logger::info("You should try to resolve these errors");
			_logger::info("However, this does not mean that your game will be broken if you don't.");
			_logger::info("------------------------------------>   Errors - End   <------------------------------------");
			_logger::info("");
			_logger::info("");
		}

		//This could very well just be the default constructor
		ParseConfigsErrors() {
			this->errorsExist =         false;
			this->caughtErrors =        std::vector<std::string>();
			this->caughtGarbage =       std::vector<std::string>();
			this->incompatibleConfigs = std::vector<std::string>();
			this->invalidConfigs =      std::vector<std::string>();
			this->missingMasters =      std::vector<std::string>();
			this->invalidArts =         std::vector<std::string>();
		}
	};

	//Returns true if the given vector contains the given string.
	bool VectorHasString(std::vector<std::string>* a_vector, std::string a_string) {

		for (std::string string : *a_vector) {
			if (string == a_string) {
				return true;
			}
		}
		return false;
	}
	
	void AddErrorToLog(ParseConfigsErrors* a_errorHolder, int a_pos, std::string a_msg) {
		a_errorHolder->errorsExist = true;

		switch (a_pos) {
		case (0):
			a_errorHolder->caughtErrors.push_back(a_msg);
			break;
		case (1):
			a_errorHolder->caughtGarbage.push_back(a_msg);
			break;
		case (2):
			a_errorHolder->incompatibleConfigs.push_back(a_msg);
			break;
		case (3):
			a_errorHolder->invalidConfigs.push_back(a_msg);
			break;
		case (4):
			a_errorHolder->missingMasters.push_back(a_msg);
			break;
		case (5):
			a_errorHolder->invalidArts.push_back(a_msg);
			break;
		default:
			break;
		}
	}

	//Returns true if a mod is installed.
	bool IsModPresent(std::string a_fileName) {
		std::string_view modName = std::string_view(a_fileName);

		RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();
		const RE::TESFile* mod;
		mod = dataHandler->LookupLoadedModByName(modName);

		if (mod) {
			return true;
		}

		mod = dataHandler->LookupLoadedLightModByName(modName);

		if (mod) {
			return true;
		}

		return false;

	}

	//Returns a FormID from a given string. Logs an error if the string cannot be parsed, and throws an exception.
	RE::FormID GetFormIDFromString(std::string a_string) {
		RE::FormID result;
		std::istringstream ss{ a_string };
		ss >> std::hex >> result;
		return result;
	}

	RE::SpellItem* GetSpellFromFormID(RE::FormID a_FormID, std::string a_ModName) {
		RE::SpellItem* response;
		RE::TESForm* form = RE::TESDataHandler::GetSingleton()->LookupForm(a_FormID, a_ModName);
		response = form ? form->As<RE::SpellItem>() : NULL;
		return response;
	}

	RE::SpellItem* ResolveHandSwapData(std::string a_FormID, std::string a_artSource) {
		RE::SpellItem* response = NULL;
		response = GetSpellFromFormID(GetFormIDFromString(a_FormID), a_artSource);
		return response;
	}

	bool IsSwapLegal(Json::Value a_Data, std::string a_ArtSource) {

		Json::Value left = a_Data["Left"];
		Json::Value right = a_Data["Right"];

		if (!(left && right)) {

			return false;
		}

		if (left) {
			std::string contents = left.asString();
			RE::FormID result;
			result = GetFormIDFromString(contents);

			if (!result) return false;
			RE::SpellItem* art = GetSpellFromFormID(result, a_ArtSource);
			if (!art) return false;
		}

		if (right) {
			std::string contents = right.asString();
			RE::FormID result;
			result = GetFormIDFromString(contents);

			if (!result) return false;
			RE::SpellItem* art = GetSpellFromFormID(result, a_ArtSource);
			if (!art) return false;
		}

		Json::Value weaponID = a_Data["WeaponID"];
		Json::Value keywordID = a_Data["WeaponKeywords"];

		//Neither present, one should.
		if (!(weaponID && keywordID)) return false;

		//Both present when they shouldn't.
		if (weaponID && keywordID) return false;

		if (weaponID) {
			Json::Value::Members weaponIDs = weaponID.getMemberNames();
			for (auto weaponID : weaponIDs) {
				if (!IsModPresent(weaponID)) return false;
				RE::FormID foundID = GetFormIDFromString(weaponID);
				if (!foundID) return false;
			}
		}
		return true;
	}

	//Returns if the config is valid (all mods present and version code correct).
	bool IsConfigValid(std::filesystem::path a_path, ParseConfigsErrors* a_errorHolder) {
		
		std::ifstream rawJSON(a_path.string());
		Json::Reader  JSONReader;
		Json::Value   JSONFile;
		JSONReader.parse(rawJSON, JSONFile);

		if (JSONFile.empty()) {
			AddErrorToLog(a_errorHolder, 3, ">" + a_path.filename().string());
			AddErrorToLog(a_errorHolder, 3, "    Empty config file.");
			return false;
		}

		int minVersion;

		try {
			minVersion = JSONFile["MinimumEnchanterVersion"].asInt();
		}
		catch (Json::Exception e) {
			AddErrorToLog(a_errorHolder, 3, ">" + a_path.filename().string());
			AddErrorToLog(a_errorHolder, 3, "    Could not read minimum requirement.");
			return false;
		}

		if (minVersion > SingletonHolder::ConditionHolder::GetSingleton()->GetVersion()) {
			AddErrorToLog(a_errorHolder, 2, ">" + a_path.filename().string());
			AddErrorToLog(a_errorHolder, 2, "    This config requires at least version " + std::to_string(minVersion));
			return false;
		}

		Json::Value artType;

		try {
			artType = JSONFile["SpecialFlag"].asString();

			if (!(artType == "Additive" || artType == "Exclusive" || artType == "Overwrite")) {
				AddErrorToLog(a_errorHolder, 3, ">" + a_path.filename().string());
				AddErrorToLog(a_errorHolder, 3, "    Unexpected value in SpecialFlag.");
				return false;
			}
		}
		catch (Json::Exception e) {
			AddErrorToLog(a_errorHolder, 3, ">" + a_path.filename().string());
			AddErrorToLog(a_errorHolder, 3, "    Failed to read the SpecialFlag field.");
			return false;
		}

		Json::Value enchantmentKeywords;

		try {
			enchantmentKeywords = JSONFile["EnchantmentKeywords"];

			if (!enchantmentKeywords.isArray()) {
				AddErrorToLog(a_errorHolder, 3, ">" + a_path.filename().string());
				AddErrorToLog(a_errorHolder, 3, "    Expected an array in EnchantmentKeywords, but did not find one.");
				return false;
			}
		}
		catch (Json::Exception e) {
			AddErrorToLog(a_errorHolder, 3, ">" + a_path.filename().string());
			AddErrorToLog(a_errorHolder, 3, "    Failed to read the EnchantmentKeywords field.");
			return false;
		}

		if (enchantmentKeywords.empty()) {
			AddErrorToLog(a_errorHolder, 3, ">" + a_path.filename().string());
			AddErrorToLog(a_errorHolder, 3, "    No keywords found in the EnchantmentKeywords field.");
			return false;
		}

		for (auto field : enchantmentKeywords) {
			if (!field.isString()) {
				AddErrorToLog(a_errorHolder, 3, ">" + a_path.filename().string());
				AddErrorToLog(a_errorHolder, 3, "    Incorrect data type within EnchantmentKeywords. Expected string.");
				return false;
			}
		}

		Json::String artSourceMod;

		try {
			artSourceMod = JSONFile["ArtSource"].asString();

			if (!(IsModPresent(artSourceMod))) {
				AddErrorToLog(a_errorHolder, 4, ">" + a_path.filename().string());
				AddErrorToLog(a_errorHolder, 4, "    Mod " + artSourceMod + " is missing, or extension is wrong.");
				return false;
			}
		}
		catch (Json::Exception e) {
			AddErrorToLog(a_errorHolder, 4, ">" + a_path.filename().string());
			AddErrorToLog(a_errorHolder, 4, "    Failed to read the ArtSource field.");
			return false;
		}
		return true;
	}

	//Returns true if the settings were successfully applied.
	bool ApplySettings(Json::Value a_JSON, ParseConfigsErrors* a_errorHolder) {

		const SingletonHolder::ConditionHolder* ConfigHolder;
		try {
			ConfigHolder = SingletonHolder::ConditionHolder::GetSingleton();
		} catch (std::exception e) {
			_logger::error("<{}> error occured.", e.what());
			return false;
		}

		Json::Value swaps = a_JSON["SwapData"];
		std::string artSource = a_JSON["ArtSource"].asString();
		std::vector<std::string> enchantmentKeywords;

		for (auto requiredKeyword : a_JSON["EnchantmentKeywords"]) {
			enchantmentKeywords.push_back(requiredKeyword.asString());
		}

		for (auto swap : swaps) {
			for (auto swapData : swap) {
				RE::SpellItem* leftSwap = ResolveHandSwapData(swapData["Left"].asString(), artSource);
				RE::SpellItem* rightSwap = ResolveHandSwapData(swapData["Right"].asString(), artSource);
				std::vector<std::string> weaponKeywords;

				for (auto requiredKeyword : swapData["WeaponKeywords"]) {
					weaponKeywords.push_back(requiredKeyword.asString());
				}

				ArtSwap newItem = ArtSwap(weaponKeywords, enchantmentKeywords, leftSwap, rightSwap);

				SingletonHolder::ConditionHolder::GetSingleton()->CreateSwap(newItem);
			}
		}

		return true;
	}

	//Called externally, parses all valid configs.
	void ParseConfigs() {
		_logger::info("------------------------------------->    Report   <--------------------------------------");
		_logger::info("Begining reading config files.");
		const std::filesystem::path configDirectory{ R"(.\Data\Enchantment Effects Extender)" };

		if (!std::filesystem::exists(configDirectory)) {
			_logger::info("No config directory found in the Data folder.");
			return;
		}

		ParseConfigsErrors errorHolder = ParseConfigsErrors();
		std::vector<std::string> validConfigs = std::vector<std::string>();
		int configCount = 0;

		try {
			for (const auto& file : std::filesystem::directory_iterator(configDirectory)) {
				if (file.path().extension() != ".json") {
					AddErrorToLog(&errorHolder, 0, file.path().string());
					continue;
				}

				++configCount;

				if (!IsConfigValid(file, &errorHolder)) {
					continue;
				}
				std::ifstream rawJSON(file.path().string());
				Json::Reader  JSONReader;
				Json::Value   JSONFile;
				JSONReader.parse(rawJSON, JSONFile);

				if (!ApplySettings(JSONFile, &errorHolder)) continue;
				validConfigs.push_back(file.path().filename().string());	
			}
		}
		catch (std::exception e) {
			_logger::error("Critical error <{}> caught while trying to read config files. Report to the author of Enchantment Effects Extender.", e.what());
			_logger::error("Assuming broken application state. Configs will not be parsed, and no arts will be distributed.");
			return;
		}
		_logger::info("Finished reading config files.");
		_logger::info("Read {} potential files.", configCount);
		_logger::info("Found {} valid files:", validConfigs.size());
		for (auto success : validConfigs) {
			_logger::info("    >{}", success);
		}
		auto swaps = SingletonHolder::ConditionHolder::GetSingleton()->GetSwaps();
		_logger::info("Number of patches applied: {}.", swaps->size());
		_logger::info("");
		_logger::info("--------------------------------->    End of Report    <-----------------------------------");
		_logger::info("");
		_logger::info("");
		if (errorHolder.errorsExist) {
			errorHolder.LogMessages();
		}
		_logger::info("Enjoy your game!");
	}
}
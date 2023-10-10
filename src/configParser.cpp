#include "configParser.h"

namespace ConfigParser {
	//Returns true if the given vector contains the given string.
	bool VectorHasString(std::vector<std::string>* a_vector, std::string a_string) {

		for (std::string string : *a_vector) {
			if (string == a_string) {
				return true;
			}
		}
		return false;
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
		return NULL;
	}

	//Returns if the config is valid (all mods present and version code correct).
	bool IsConfigValid(std::string a_path) {
		
		return true;
	}
	//Returns a vector with all valid configs.
	std::vector<std::string> GetAllConfigs() {
		std::vector<std::string> response;
		std::vector<std::string> paths = clib_util::distribution::get_configs("Data/SKSE/Plugins/Enchantment Effects Extender"sv, "_EFX"sv, ".json"sv);

		return response;
	}

	//Parses a given valid config. If the config has issues, throws an exception.
	void ParseConfig() {
		
	}

	//Called externally, parses all valid configs.
	void ParseConfigs() {
		GetAllConfigs();
	}

}
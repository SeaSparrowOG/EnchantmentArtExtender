#pragma once

namespace Settings {
#define stringVector std::vector<std::string>
	struct ErrorReport {
		bool         outdatedFrameworkVersion;
		bool         valid;
		std::string  configName;
		stringVector missingRequiredFields;
		stringVector foundGarbage;
		stringVector expectedString;
		stringVector expectedObject;
		stringVector expectedList;
		stringVector badSwapData;

		ErrorReport(std::string a_configName) {
			this->configName = a_configName;
			this->outdatedFrameworkVersion = false;
			this->valid = true;
		}
	};

	bool BuildIni();
	ErrorReport IsConfigValid(std::string config, Json::Value a_JSON);
}

namespace Utilities {
	/**
	* Returns a form from the given string, nullptr if something went wrong.
	* @param a_formIDstr The string of the hex of the FormID of the Form to look for.
	* @param a_filename The plugin name (with the extension) in which to querry for the form.
	* @return A pointer to a form matching the given FormID. Nullptr if not found.
	*/
	RE::TESForm* GetFormFromString(std::string a_formIDstr, std::string a_fileName);

	/**
	* Returns if the given mod (with the extension) is present and active in the load order.
	* @return True if found, false otherwise.
	*/
	bool IsModPresent(std::string a_modName);
}
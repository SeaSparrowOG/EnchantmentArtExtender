#include "Settings/JSONSettings.h"

#include "enchantmentManager/enchantmentManager.h"
#include "utilities/utilities.h"

namespace {
	static std::vector<std::string> findJsonFiles()
	{
		static constexpr std::string_view directory = R"(Data/SKSE/Plugins/EnchantmentArtExtender)";
		std::vector<std::string> jsonFilePaths;
		for (const auto& entry : std::filesystem::directory_iterator(directory)) {
			if (entry.is_regular_file() && entry.path().extension() == ".json") {
				jsonFilePaths.push_back(entry.path().string());
			}
		}

		std::sort(jsonFilePaths.begin(), jsonFilePaths.end());
		return jsonFilePaths;
	}
}


namespace Settings::JSON
{
	void Read()
	{
		std::vector<std::string> paths{};
		const auto singleton = EnchantmentManager::Manager::GetSingleton();
		try {
			paths = findJsonFiles();
		}
		catch (const std::exception& e) {
			logger::warn("Caught {} while reading files.", e.what());
			return;
		}
		if (paths.empty()) {
			logger::info("No settings found");
			return;
		}

		for (const auto& path : paths) {
			Json::Reader JSONReader;
			Json::Value JSONFile;
			try {
				std::ifstream rawJSON(path);
				JSONReader.parse(rawJSON, JSONFile);
			}
			catch (const Json::Exception& e) {
				logger::warn("Caught {} while reading files.", e.what());
				continue;
			}
			catch (const std::exception& e) {
				logger::error("Caught unhandled exception {} while reading files.", e.what());
				continue;
			}

			if (!JSONFile.isObject()) {
				logger::warn("Warning: <{}> is not an object. File will be ignored.", path);
				continue;
			}

			const auto& rules = JSONFile["rules"];
			if (!rules || !rules.isArray()) {
				logger::warn("<{}> contains non-array rules field, or rules field is missing.", path);
				continue;
			}

			for (const auto& rule : rules) {
				if (!rule.isObject()) {
					logger::warn("<{}> contains rule entry that is not an object.", path);
					continue;
				}

				const auto& weaponArt = rule["weaponArt"];
				const auto& enchantmentKeywords = rule["enchantmentKeywords"];
				if (!weaponArt || !weaponArt.isString() || !enchantmentKeywords || !enchantmentKeywords.isArray()) {
					logger::warn("<{}> contains rule entry that is missing or has invalid weaponArt and or enchantmentKeywords.", path);
					continue;
				}

				const auto enchantmentObject = Utilities::Forms::GetFormFromString<RE::BGSArtObject>(weaponArt.asString());
				if (!enchantmentObject) {
					logger::warn("<{}> - <{}> could not be resolved.", path, weaponArt.asString());
					continue;
				}

				bool hasBadEnchantmentKeyword = false;
				std::vector<std::string> newEnchantmentKeywords = std::vector<std::string>();
				for (auto& entry : enchantmentKeywords) {
					if (!entry.isString()) {
						logger::warn("<{}> - <{}> contains non string entry for enchantmentKeywords.", path, weaponArt.asString());
						hasBadEnchantmentKeyword = true;
						break;
					}
					newEnchantmentKeywords.push_back(entry.asString());
				}
				if (hasBadEnchantmentKeyword) {
					continue;
				}

				const auto& reverseWeaponKeywords = rule["!weaponKeywords"];
				auto newReverseWeaponKeywords = std::vector<std::string>();
				if (reverseWeaponKeywords) {
					if (reverseWeaponKeywords.isArray()) {
						bool isValid = true;
						for (auto entry : reverseWeaponKeywords) {
							if (!entry.isString()) {
								logger::warn("<{}> - <{}> contains reverseWeaponKeywords array with non string elements.", path, weaponArt.asString());
								isValid = false;
								break;
							}

							newReverseWeaponKeywords.push_back(entry.asString());
						}
						if (!isValid) {
							continue;
						}
					}
					else {
						logger::warn("<{}> - <{}> contains reverseWeaponKeywords field that is not an array.", path, weaponArt.asString());
						continue;
					}
				}

				const auto& weaponKeywords = rule["weaponKeywords"];
				auto newWeaponKeywords = std::vector<std::string>();
				if (weaponKeywords) {
					if (weaponKeywords.isArray()) {
						bool isValid = true;
						for (auto entry : weaponKeywords) {
							if (!entry.isString()) {
								logger::warn("<{}> - <{}> contains weaponKeywords array with non string elements.", path, weaponArt.asString());
								isValid = false;
								break;
							}

							newWeaponKeywords.push_back(entry.asString());
						}
						if (!isValid) {
							continue;
						}
					}
					else {
						logger::warn("<{}> - <{}> contains weaponKeywords field that is not an array.", path, weaponArt.asString());
						continue;
					}
				}

				const auto& weapons = rule["weapons"];
				auto newWeapons = std::vector<RE::TESObjectWEAP*>();
				if (weapons) {
					if (weapons.isArray()) {
						bool isValid = true;
						for (const auto& entry : weapons) {
							if (!entry.isString()) {
								logger::warn("<{}> - <{}> contains weapons array with non string elements.", path, weaponArt.asString());
								isValid = false;
								break;
							}

							const auto foundForm = Utilities::Forms::GetFormFromString<RE::TESObjectWEAP>(entry.asString());
							if (!foundForm) {
								logger::warn("<{}> - <{}> - <{}> could not resolve form.", path, weaponArt.asString(), entry.asString());
								isValid = false;
								break;
							}

							newWeapons.push_back(foundForm);
						}
						if (!isValid) {
							continue;
						}
					}
					else {
						logger::warn("<{}> - <{}> contains weapons field that is not an array.", path, weaponArt.asString());
						continue;
					}
				}

				const auto& reverseWeapons = rule["!weapons"];
				auto newForbiddenWeapons = std::vector<RE::TESObjectWEAP*>();
				if (reverseWeapons) {
					if (reverseWeapons.isArray()) {
						bool isValid = true;
						for (const auto& entry : reverseWeapons) {
							if (!entry.isString()) {
								logger::warn("<{}> - <{}> contains weapons array with non string elements.", path, weaponArt.asString());
								isValid = false;
								break;
							}

							const auto foundForm = Utilities::Forms::GetFormFromString<RE::TESObjectWEAP>(entry.asString());
							if (!foundForm) {
								logger::warn("<{}> - <{}> - <{}> could not resolve form.", path, weaponArt.asString(), entry.asString());
								isValid = false;
								break;
							}

							newForbiddenWeapons.push_back(foundForm);
						}
						if (!isValid) {
							continue;
						}
					}
					else {
						logger::warn("<{}> - <{}> contains weapons field that is not an array.", path, weaponArt.asString());
						continue;
					}
				}

				if (!(newForbiddenWeapons.empty() && newWeapons.empty() && newWeaponKeywords.empty() && newReverseWeaponKeywords.empty())) {
					singleton->CreateNewData(enchantmentObject, newEnchantmentKeywords, newWeaponKeywords, newReverseWeaponKeywords, newWeapons, newForbiddenWeapons);
				}
			}
		}
	}
}
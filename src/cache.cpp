#include "cache.h"
#include "settingsReader.h"

namespace {
	/**
	* Yay for StackOverflow.
	* Source: https://stackoverflow.com/questions/8899069/how-to-find-if-a-given-string-conforms-to-hex-notation-eg-0x34ff-without-regex
	* @param a_num The string which to check.
	* @return True if it is a hex, false otherwise.
	*/
	bool IsHex(std::string a_num) {
		return a_num.compare(0, 2, "0x") == 0
			&& a_num.size() > 2
			&& a_num.find_first_not_of("0123456789abcdefABCDEF", 2) == std::string::npos;
	}

	/**
	* Converts given string into a usable FormID. Credids to Colinswrath.
	* Nexus:
	* Github:
	* @param a_str The string to convert.
	* @return The FormID as RE::FormID. NULL if not a hex.
	*/
	RE::FormID StringToFormID(std::string a_str) {
		if (!IsHex(a_str)) return NULL;

		RE::FormID result;
		std::istringstream ss{ a_str };
		ss >> std::hex >> result;
		return result;
	}

	/**
	* Checks to see if a given mod (with the extension) is present.
	* @param a_modName: The name of the mod, including the extension.
	* @return True if it is active, false otherwise.
	*/
	bool IsModPresent(std::string a_modName) {
		auto* found = RE::TESDataHandler::GetSingleton()->LookupLoadedLightModByName(a_modName);
		if (!found) found = RE::TESDataHandler::GetSingleton()->LookupLoadedModByName(a_modName);
		if (found) return true;
		return false;
	}

	/**
	* Returns a weapon pointer to a weapon.
	* @param a_hex The formID of the weapon. If it is not a hex, it gets discarded.
	* @param a_source The mod which defines the weapon. If it is not active, it gets discarded.
	* @return The pointer to the weapon if found, nullptr otherwise.
	*/
	RE::TESObjectWEAP* GetWeaponFromString(std::string a_hex, std::string a_source) {
		if (IsModPresent(a_source)) {
			RE::FormID id = StringToFormID(a_hex);
			if (!id) return nullptr;

			return RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESObjectWEAP>(id, a_source);
		}
		return nullptr;
	}

	/**
	* Returns a pointer to an enchantment.
	* @param a_hex The formID of the enchantment. If it is not a hex, it gets discarded.
	* @param a_source The mod which defines the enchantment. If it is not active, it gets discarded.
	* @return The pointer to the enchantment if found, nullptr otherwise.
	*/
	RE::EnchantmentItem* GetEnchantmentFromString(std::string a_hex, std::string a_source) {
		if (IsModPresent(a_source)) {
			RE::FormID id = StringToFormID(a_hex);
			if (!id) return nullptr;

			return RE::TESDataHandler::GetSingleton()->LookupForm<RE::EnchantmentItem>(id, a_source);
		}
		return nullptr;
	}

	/**
	* Returns a pointer to a spell.
	* @param a_hex The formID of the spell. If it is not a hex, it gets discarded.
	* @param a_source The mod which defines the spell. If it is not active, it gets discarded.
	* @return The pointer to the spell if found, nullptr otherwise.
	*/
	RE::SpellItem* GetSpellFromString(std::string a_hex, std::string a_source) {
		if (IsModPresent(a_source)) {
			RE::FormID id = StringToFormID(a_hex);
			if (!id) return nullptr;
			return RE::TESDataHandler::GetSingleton()->LookupForm<RE::SpellItem>(id, a_source);
		}
		return nullptr;
	}

	/**
	* Prints out a small formatted message to the log with the information of a given swap.
	* @param swap The actual swap to test for.
	*/
	void SwapReport(Cache::SwapData swap) {
		_loggerInfo("    Report for swap {}:", swap.name);
		_loggerInfo("        Left Ability: {}", clib_util::editorID::get_editorID(swap.leftAbility));
		_loggerInfo("        Right Ability: {}", clib_util::editorID::get_editorID(swap.rightAbility));
		_loggerInfo("        Enchantment Keywords:");
		for (auto& keyword : swap.requiredEnchantmentKeywords)
			_loggerInfo("            >{}", keyword);

		if (!swap.requiredWeaponKeywords.empty())
			_loggerInfo("        Weapon Keywords:");
		for (auto& keyword : swap.requiredWeaponKeywords)
			_loggerInfo("            >{}", keyword);

		if (!swap.excludedWeaponKeywords.empty())
			_loggerInfo("        Ignored Weapon Keywords:");
		for (auto& keyword : swap.excludedWeaponKeywords)
			_loggerInfo("            >{}", keyword);

		if (!swap.excludedWeapons.empty())
			_loggerInfo("        Excluded Weapons:");
		for (auto& weapon : swap.excludedWeapons)
			_loggerInfo("            >{}", clib_util::editorID::get_editorID(weapon));
		_loggerInfo("==============================================");
	}

	/**
	* Doesn't work, don't use.
	*/
	bool DisableEnchantmentShader() {
		auto& enchantmentArray = RE::TESDataHandler::GetSingleton()->GetFormArray<RE::EnchantmentItem>();

		for (auto* enchantment : enchantmentArray) {
			for (auto effect : enchantment->effects) {
				effect->baseEffect->data.enchantShader = nullptr;
			}
		}
		return true;
	}
}

namespace Cache {
	spellVector Cache::StoredData::GetAllAbilities() { return this->storedAbilities; }

	void StoredData::DebugSwaps() {
		for (auto& swap : this->storedSwaps) {
			SwapReport(swap);
		}
	}

	void StoredData::RegisterSwaps(std::vector<Json::Value> a_validConfigs) {
		if (a_validConfigs.empty()) return;
		for (auto& config : a_validConfigs) {
			auto artSource = config["ArtSource"].asString();
			if (!IsModPresent(artSource)) continue;

			bool exclusive = false;
			if (config["Exclusive"])
				exclusive = config["Exclusive"].asBool();

			auto swapData = config["SwapData"];
			for (auto& data : swapData) {
				SwapData newSwap = SwapData();
				newSwap.artSource = artSource;
				newSwap.exclusive = exclusive;

				//New in version 2: Added the Enchantment field that can be used instead of EnchantmentKeywords.
				auto requiredKeywords = config["EnchantmentKeywords"];
				if (requiredKeywords) {
					for (auto& keyword : requiredKeywords) {
						newSwap.requiredEnchantmentKeywords.push_back(keyword.asString());
					}
				}
				else {
					auto requiredEnchantments = config["Enchantment"];
					for (auto& enchantment : requiredEnchantments) {
						auto sourceMod = enchantment["Source"].asString();
						auto id = enchantment["ID"].asString();

						auto* enchantment = GetEnchantmentFromString(id, sourceMod);
						if (!enchantment) continue;
						newSwap.requiredEnchantments.push_back(enchantment);
					}
				}

				bool isValid = false;
				auto name = data.getMemberNames().at(0);
				newSwap.name = name;
				auto swapInfo = data[name];

				auto left = swapInfo["Left"].asString();
				auto right = swapInfo["Right"].asString();
				auto* leftAbility = GetSpellFromString(left, artSource);
				auto* rightAbility = GetSpellFromString(right, artSource);
				if (!(leftAbility && rightAbility)) continue;

				newSwap.leftAbility = leftAbility;
				newSwap.rightAbility = rightAbility;

				if (swapInfo["RequiredWeapons"]) {
					auto requiredWeapons = swapInfo["RequiredWeapons"];
					for (auto& weaponInfo : requiredWeapons) {
						auto weaponSource = weaponInfo["Source"].asString();
						auto weaponID = weaponInfo["ID"].asString();

						auto* weapon = GetWeaponFromString(weaponID, weaponSource);
						if (!weapon) continue;
						newSwap.requiredWeapons.push_back(weapon);
						isValid = true;
					}
				}
				else {
					auto requiredWeaponKeywords = swapInfo["WeaponKeywords"];
					for (auto& keyword : requiredWeaponKeywords) {
						auto keywordString = keyword.asString();
						newSwap.requiredWeaponKeywords.push_back(keywordString);
						isValid = true;
					}

					if (swapInfo["IgnoredWeaponKeywords"]) {
						for (auto& keyword : swapInfo["IgnoredWeaponKeywords"]) {
							auto keywordString = keyword.asString();
							newSwap.excludedWeaponKeywords.push_back(keywordString);
						}
					}

					if (swapInfo["IgnoredWeapons"]) {
						auto excludedWeapons = swapInfo["IgnoredWeapons"];
						for (auto& weaponInfo : excludedWeapons) {
							auto weaponSource = weaponInfo["Source"].asString();
							auto weaponID = weaponInfo["ID"].asString();

							auto* weapon = GetWeaponFromString(weaponID, weaponSource);
							if (!weapon) continue;
							newSwap.excludedWeapons.push_back(weapon);
						}
					}
				}
				if (isValid)
					this->storedAbilities.push_back(leftAbility);
					this->storedAbilities.push_back(rightAbility);
					this->storedSwaps.push_back(newSwap);
			}
		}
	}

	swapDataVector StoredData::GetMatchingSwaps(RE::TESObjectWEAP* a_weap, RE::EnchantmentItem* a_enchantment) {
		auto response = swapDataVector();
		auto exclusiveResponse = SwapData();
		if (!(a_weap && a_enchantment)) return response;

		if (this->weaponCache.contains(a_weap)) return this->weaponCache.at(a_weap);
		if (this->invalidWeapons.contains(a_weap)) return response;

		auto* templateWeapon = a_weap->templateWeapon;
		size_t previousMaxMatch = 0;
		bool hasExclusiveMatch = false;
		bool hasExclusiveWeaponMatch = false;
		bool foundExclusiveMatch = false;

		for (auto& swapEntry : this->storedSwaps) {
			bool isSwapExclusive = swapEntry.exclusive;
			size_t matchingDegree = 0;
			if (!isSwapExclusive && hasExclusiveMatch) continue;

			size_t matches = swapEntry.requiredEnchantmentKeywords.size();
			if (matches > 0) {
				matchingDegree += matches;

				std::vector<std::string> matchAgainst = swapEntry.requiredEnchantmentKeywords;
				for (auto& requiredEnchantmentKeyword : swapEntry.requiredEnchantmentKeywords) {
					bool foundMatch = false;
					for (auto effectIterator = a_enchantment->effects.begin(); !foundMatch && effectIterator != a_enchantment->effects.end(); ++effectIterator) {
						if ((*effectIterator)->baseEffect->HasKeywordString(requiredEnchantmentKeyword)) {
							for (auto it = matchAgainst.begin(); it != matchAgainst.end(); ++it) {
								if (requiredEnchantmentKeyword == *it) {
									matchAgainst.erase(it);
									break;
								}
							}
						}
					}
				}
				if (!matchAgainst.empty())
					continue;
			}
			//Version 2 added "Enchantment" as a field.
			else {
				bool hasMatch = false;
				matches = swapEntry.requiredEnchantments.size();
				if (matches > 0) {
					for (auto it = swapEntry.requiredEnchantments.begin(); !hasMatch && it != swapEntry.requiredEnchantments.end(); ++it) {
						if (*it == a_enchantment) {
							hasMatch = true;
						}
					}
				}
				if (!hasMatch)
					continue;
			}

			if (!swapEntry.requiredWeapons.empty()) {
				for (auto& validWeapon : swapEntry.requiredWeapons) {
					if (validWeapon == a_weap) {
						if (!hasExclusiveWeaponMatch && isSwapExclusive) {
							previousMaxMatch = 0;
							hasExclusiveWeaponMatch = true;
							previousMaxMatch = matchingDegree;
						}

						if (isSwapExclusive && matchingDegree > previousMaxMatch) {
							exclusiveResponse = swapEntry;
							foundExclusiveMatch = true;
							previousMaxMatch = matchingDegree;
						}
						response.push_back(swapEntry);
					}
					else if (templateWeapon && validWeapon == templateWeapon) {
						if (!hasExclusiveWeaponMatch && isSwapExclusive) {
							previousMaxMatch = 0;
							hasExclusiveWeaponMatch = true;
							previousMaxMatch = matchingDegree;
						}

						if (isSwapExclusive && matchingDegree > previousMaxMatch) {
							exclusiveResponse = swapEntry;
							foundExclusiveMatch = true;
							previousMaxMatch = matchingDegree;
						}
						response.push_back(swapEntry);
					}
				}
			}
			else {
				if (hasExclusiveWeaponMatch) continue;
				if (!swapEntry.excludedWeapons.empty()) {
					matchingDegree += swapEntry.excludedWeapons.size();
					bool isExcludedWeapon = false;
					for (auto& badWeapon : swapEntry.excludedWeapons) {
						if (a_weap == badWeapon) {
							isExcludedWeapon = true;
						}
						else if (templateWeapon && templateWeapon == a_weap) {
							isExcludedWeapon = true;
						}
					}
					if (isExcludedWeapon)
						continue;
				}

				if (!swapEntry.excludedWeaponKeywords.empty()) {
					matchingDegree += swapEntry.excludedWeaponKeywords.size();
					bool hasBadKeyword = false;
					for (auto& keyword : swapEntry.excludedWeaponKeywords) {
						if (a_weap->HasKeywordString(keyword))
							hasBadKeyword = true;
					}
					if (hasBadKeyword) continue;
				}
				bool hasAllKeywords = true;
				matchingDegree += swapEntry.requiredWeaponKeywords.size();

				for (auto& requiredKeyword : swapEntry.requiredWeaponKeywords) {
					if (!a_weap->HasKeywordString(requiredKeyword))
						hasAllKeywords = false;
				}
				if (!hasAllKeywords) continue;
				if (isSwapExclusive && matchingDegree > previousMaxMatch) {
					foundExclusiveMatch = true;
					exclusiveResponse = swapEntry;
					previousMaxMatch = matchingDegree;
				}
				response.push_back(swapEntry);
			}
		}

		if (foundExclusiveMatch) {
			return swapDataVector({ exclusiveResponse });
		}
		return response;
	}

	bool StoredData::RegisterReadyWeapons() {
		auto start = std::chrono::steady_clock::now();
		auto& weaponArray = RE::TESDataHandler::GetSingleton()->GetFormArray<RE::TESObjectWEAP>();

		for (auto* weapon : weaponArray) {
			SwapData matchingSwap = SwapData();
			RE::EnchantmentItem* weaponEnchant = weapon->formEnchanting;
			if (!weaponEnchant) continue;
			if (weaponEnchant->effects.empty()) {
				this->invalidWeapons[weapon] = true;
				continue;
			}

			auto foundSwaps = this->GetMatchingSwaps(weapon, weaponEnchant);
			if (!foundSwaps.empty()) {
				this->weaponCache[weapon] = foundSwaps;
			}
			else {
				this->invalidWeapons[weapon] = true;
			}
		}

		auto end = std::chrono::steady_clock::now();
		auto execTime = end - start;
		_loggerInfo("Finished generating the weapon cache in {}ms.", std::chrono::duration<double, std::milli>(execTime).count(), weaponCache.size());
		return true;
	}

	bool Cache::StoredData::InitiateCache() {
		Settings::BuildIni();
		ApplyINISettings();

		std::vector<std::string> configFiles = std::vector<std::string>();
		std::vector<Json::Value> validConfigs = std::vector<Json::Value>();
		std::vector<Settings::ErrorReport> caughtErrors = std::vector<Settings::ErrorReport>();

		try {
			configFiles = clib_util::distribution::get_configs(configPath, ""sv, ".json"sv);
		}
		//Directory not existing, for example?
		catch (std::exception e) {
			SKSE::log::warn("WARNING - caught exception {} while attempting to fetch configs.", e.what());
			return false;
		}
		if (configFiles.empty()) return true;

		//Validation block. Post here, validConfigs and caughtErrors are filled.
		for (const auto& config : configFiles) {
			try {
				std::ifstream rawJSON(config);
				Json::Reader  JSONReader;
				Json::Value   JSONFile;
				JSONReader.parse(rawJSON, JSONFile);
				std::string configName = config.substr(config.rfind("/") + 1, config.length() -1);
				auto response = Settings::IsConfigValid(configName, JSONFile);

				if (response.valid) {
					validConfigs.push_back(JSONFile);
				}
				else {
					caughtErrors.push_back(response);
				}
			}
			//Unlikely to be thrown, unless there are some weird characters involved.
			catch (std::exception e) {
				SKSE::log::warn("WARNING - caught exception {} while reading a file.", e.what());
			}
		}

		//Report on successes and failures:
		_loggerInfo("Found {} configuration files.", configFiles.size());
		if (!validConfigs.empty())
			_loggerInfo("    >{} were valid.", validConfigs.size());
		if (!caughtErrors.empty())
			_loggerInfo("    >{} had problems.", caughtErrors.size());

		_loggerInfo("");
		if (!caughtErrors.empty())
			_loggerInfo("Error Report:");

		for (auto& error : caughtErrors) {
			_loggerInfo("    {}", error.configName);
			if (error.outdatedFrameworkVersion) {
				_loggerInfo("        >Config requires a newer version of Enchantment Arts Extender.");
				continue;
			}

			if (!error.missingRequiredFields.empty())
				_loggerInfo("        >Missing fields:");
			for (auto& missingField : error.missingRequiredFields) {
				_loggerInfo("            >{}", missingField);
			}

			if (!error.foundGarbage.empty())
				_loggerInfo("        >Caught {} garbage fields:", error.foundGarbage.size());
			for (auto& garbage : error.foundGarbage) {
				if (garbage.empty()) continue;
				_loggerInfo("            {}", garbage);
			}

			if (!error.expectedBool.empty())
				_loggerInfo("        >Found {} non-bool fields.", error.expectedBool.size());

			if (!error.expectedString.empty())
				_loggerInfo("        >Found {} non-string fields.", error.expectedString.size());

			if (!error.expectedList.empty())
				_loggerInfo("        >Found {} non-array fields.", error.expectedList.size());

			if (!error.expectedObject.empty())
				_loggerInfo("        >Found {} non-object fields.", error.expectedObject.size());

			if (!error.badSwapData.empty())
				_loggerInfo("        >The following bad entries in the ArtSwap field were found:");
			for (auto& badData : error.badSwapData) {
				_loggerInfo("            {}", badData);
			}
		}

		//Apply the settings:
		RegisterSwaps(validConfigs);
		_loggerInfo("Applied {} patches.", this->storedSwaps.size());
		_loggerInfo("==============================================");
		this->RegisterReadyWeapons();
		_loggerInfo("Found {} enchanted weapon, {} of which will have new effects.", this->weaponCache.size() + this->invalidWeapons.size(), this->weaponCache.size());
		this->lightObject = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESObjectLIGH>(StringToFormID("0x800"), "EnchantmentArtExtender.esl");
		this->enchantmentLightRight = RE::TESDataHandler::GetSingleton()->LookupForm<RE::SpellItem>(StringToFormID("0x801"), "EnchantmentArtExtender.esl");
		this->enchantmentLightLeft = RE::TESDataHandler::GetSingleton()->LookupForm<RE::SpellItem>(StringToFormID("0x802"), "EnchantmentArtExtender.esl");
		this->enchantmentLightBoth = RE::TESDataHandler::GetSingleton()->LookupForm<RE::SpellItem>(StringToFormID("0x803"), "EnchantmentArtExtender.esl");
		return true;
	}

	bool Cache::StoredData::ApplyINISettings() {
		std::filesystem::path f{ "Data/SKSE/Plugins/EnchantmentArtExtender.ini" };
		CSimpleIniA ini;
		ini.SetUnicode();
		ini.LoadFile(f.c_str());

		if (ini.GetBoolValue("General", "bSuppressOriginalShader", false)) {
			DisableEnchantmentShader();
		}

		this->shouldAddLights = ini.GetBoolValue("General", "bShouldAddLight", true);

		if (!this->shouldAddLights) {
			for (auto* ability : this->storedAbilities) {
				auto effects = ability->effects;
				for (auto* effect : effects) {
					auto* baseEffect = effect->baseEffect;
					if (!baseEffect) continue;
					if (baseEffect->GetArchetype() != RE::EffectSetting::Archetype::kLight)
						continue;
					if (!baseEffect->data.associatedForm) continue;
					baseEffect->data.associatedForm = nullptr;
				}
			}
		}
	}

	bool Cache::StoredData::GetShouldAddLight() { return this->shouldAddLights; }
}
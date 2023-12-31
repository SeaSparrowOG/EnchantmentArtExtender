#include "artSwap.h"
#include "helperFunctions.h"

namespace ArtSwap {
	bool ArtSwap::IsMatch(RE::TESObjectWEAP* a_weapon, RE::EnchantmentItem* a_enchantment) {
		if (!(a_enchantment && a_weapon)) return false;

		for (auto ignoredWeapon : ignoredWeapons) {
			if (a_weapon == ignoredWeapon) return false;
		}

		for (auto requiredWeaponKeyword : requiredWeaponKeywords) {
			if (!a_weapon->HasKeywordString(requiredWeaponKeyword)) return false;
		}

		for (auto requiredKeyword : requiredEnchantmentKeywords) {
			bool match = false;

			for (auto effect : a_enchantment->effects) {
				if (effect->baseEffect->HasKeywordString(requiredKeyword)) match = true;
			}

			if (!match) return false;
		}

		for (auto ignoredKeyword : excludeWeaponKeywords) {
			if (a_weapon->HasKeywordString(ignoredKeyword)) return false;
		}

		if (requiredWeapons.empty()) return true;

		bool weaponMatch = false;
		for (auto requiredWeapon : requiredWeapons) {
			if (a_weapon == requiredWeapon) weaponMatch = true;
		}

		return weaponMatch;
	}

	//Should be called after the class is instantiated and be provided with a validated JSON. Sets all fields appropriately.
	void ArtSwap::SetAttributes(Json::Value a_swapData, std::string a_mode, std::string a_artSource, std::vector<std::string> a_requiredEnchantmentKeywords, RE::SpellItem* a_leftAbility, RE::SpellItem* a_rightAbility) {

		std::string artSource = a_artSource;
		SetMode(a_mode);

		SetAbility(a_leftAbility);
		SetAbility(a_rightAbility, false);

		for (auto requiredKeyword : a_requiredEnchantmentKeywords) {
			AddRequiredEnchantmentKeyword(requiredKeyword);
		}

		if (a_swapData["WeaponID"]) {
			auto modNames = a_swapData["WeaponID"].getMemberNames();
			for (auto modName : modNames) {
				RE::TESObjectWEAP* foundWeapon = helperFunction::GetWeaponFromID(helperFunction::GetFormIDFromString(a_swapData["Weapons"][modName].asString()), modName);
				AddRequiredWeapon(foundWeapon);
			}
		}
		else {
			for (auto requiredKeyword : a_swapData["WeaponKeywords"]) AddRequiredWeaponKeyword(requiredKeyword.asString());

			if (a_swapData["IgnoredKeywords"]) {
				for (auto ignoredKeyword : a_swapData["IgnoredKeywords"]) AddRequiredWeaponKeyword(ignoredKeyword.asString());
			}

			if (a_swapData["IgnoredWeaponID"]) {
				auto modNames = a_swapData["IgnoredWeaponID"].getMemberNames();
				for (auto modName : modNames) {
					if (!helperFunction::IsModPresent(modName)) continue;
					RE::FormID weaponFormID = helperFunction::GetFormIDFromString(a_swapData["Weapons"][modName].asString()) ? helperFunction::GetFormIDFromString(a_swapData["Weapons"][modName].asString()) : NULL;
					RE::TESObjectWEAP* foundWeapon = weaponFormID ? helperFunction::GetWeaponFromID(weaponFormID, modName) : nullptr;
					if (foundWeapon) AddExcludedWeapon(foundWeapon);
				}
			}
		}				
	}

	//Setters - Set the class member value.
	void ArtSwap::SetAbility(RE::SpellItem* a_ability, bool a_bLeftHand) {
		if (a_bLeftHand) { this->leftHandAbility = a_ability; }
		else { this->rightHandAbility = a_ability; }
	}

	void ArtSwap::SetMode(std::string a_mode) {
		if (a_mode == "Additive") {
			this->mode = ArtSwapMode::Additive;
		}
		else if (a_mode == "Exclusive") {
			this->mode = ArtSwapMode::Exclusive;
		}
		else {
			this->mode = ArtSwapMode::LowPriority;
		}
	}
	void ArtSwap::SetName(std::string a_name) { this->swapName = a_name; }

	//Adders - These do not allow duplicate entries
	void ArtSwap::AddRequiredEnchantmentKeyword(std::string a_keywordString) {
		if (a_keywordString.empty()) return;
		for (auto keyword : requiredEnchantmentKeywords) {
			if (keyword == a_keywordString) return;
		}

		this->requiredEnchantmentKeywords.push_back(a_keywordString);
	}

	void ArtSwap::AddExcludedWeaponKeyword(std::string a_keywordString) {
		if (a_keywordString.empty()) return;
		for (auto keyword : excludeWeaponKeywords) {
			if (keyword == a_keywordString) return;
		}
		this->excludeWeaponKeywords.push_back(a_keywordString);
	}
	
	void ArtSwap::AddRequiredWeaponKeyword(std::string a_keywordString) {
		if (a_keywordString.empty()) return;
		for (auto keyword : this->requiredWeaponKeywords) {
			if (keyword == a_keywordString) return;
		}
		this->requiredWeaponKeywords.push_back(a_keywordString);
	}

	void ArtSwap::AddRequiredWeapon(RE::TESObjectWEAP* a_weapon) {
		for (auto* weapon : requiredWeapons) {
			if (weapon == a_weapon) return;
		}
		this->requiredWeapons.push_back(a_weapon);
	}

	void ArtSwap::AddExcludedWeapon(RE::TESObjectWEAP* a_weapon) {
		for (auto* weapon : requiredWeapons) {
			if (weapon == a_weapon) return;
		}
		this->ignoredWeapons.push_back(a_weapon);
	}

	//Removers - no duplicates allowed, so we only remove the first instance.
	//Removers are here for a future function where configs can be made to remove stuff from other configs.
	void ArtSwap::RemoveRequiredWeaponKeyword(std::string a_keywordString) {
		for (std::vector<std::string>::iterator it = requiredWeaponKeywords.begin(); it != requiredWeaponKeywords.end(); ++it) {
			if (*it == a_keywordString) {
				this->requiredWeaponKeywords.erase(it);
				break;
			}
		}
	}

	void ArtSwap::RemoveExcludedWeaponKeyword(std::string a_keywordString) {
		for (std::vector<std::string>::iterator it = excludeWeaponKeywords.begin(); it != excludeWeaponKeywords.end(); ++it) {
			if (*it == a_keywordString) {
				this->excludeWeaponKeywords.erase(it);
				break;
			}
		}
	}

	void ArtSwap::RemoveRequiredEnchantmentKeyword(std::string a_keywordString) {
		for (std::vector<std::string>::iterator it = requiredEnchantmentKeywords.begin(); it != requiredEnchantmentKeywords.end(); ++it) {
			if (*it == a_keywordString) {
				this->requiredEnchantmentKeywords.erase(it);
				break;
			}
		}
	}

	void ArtSwap::RemoveRequiredWeapon(RE::TESObjectWEAP* a_weapon) {
		for (std::vector<RE::TESObjectWEAP*>::iterator it = requiredWeapons.begin(); it != requiredWeapons.end(); ++it) {
			if (*it == a_weapon) {
				this->requiredWeapons.erase(it);
				break;
			}
		}
	}

	void ArtSwap::RemoveExcludedWeapon(RE::TESObjectWEAP* a_weapon) {
		for (std::vector<RE::TESObjectWEAP*>::iterator it = ignoredWeapons.begin(); it != ignoredWeapons.end(); ++it) {
			if (*it == a_weapon) {
				this->ignoredWeapons.erase(it);
				break;
			}
		}
	}

	//Getters that have a condition. NORMALLY I'd throw. But I cannot be bothered.
	RE::SpellItem* ArtSwap::GetAbility(bool a_left) {
		if (a_left) return this->leftHandAbility;
		return this->rightHandAbility;
	}

	ArtSwapMode ArtSwap::GetMode()                                     { return this->mode; }
	std::string ArtSwap::GetName()                                     { return this->swapName; }
	std::vector<std::string> ArtSwap::GetRequiredEnchantmentKeywords() { return this->requiredEnchantmentKeywords; }
	std::vector<std::string> ArtSwap::GetRequiredWeaponKeywords()      { return this->requiredWeaponKeywords; }
	std::vector<std::string> ArtSwap::GetExcludedWeaponKeywords()      { return this->excludeWeaponKeywords; }
	std::vector <RE::TESObjectWEAP*> ArtSwap::GetIgnoredWeapons()      { return this->ignoredWeapons; }
	std::vector <RE::TESObjectWEAP*> ArtSwap::GetRequiredWeapons()     { return this->requiredWeapons; }
}
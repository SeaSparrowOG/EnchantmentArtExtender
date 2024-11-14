#include "enchantmentManager/enchantmentManager.h"

namespace EnchantmentManager
{

	bool Manager::WeaponCondition::IsApplicable(RE::TESObjectWEAP* a_data) const
	{
		for (const auto& internalForm : weapons) {
			if (internalForm == a_data) {
				return !inverted;
			}
		}

		auto templateWeapon = a_data->templateWeapon;
		while (templateWeapon) {
			for (const auto& internalForm : weapons) {
				if (internalForm == templateWeapon) {
					return !inverted;
				}
			}

			templateWeapon = templateWeapon->templateWeapon;
		}

		return inverted;
	}

	bool Manager::WeaponKeywordCondition::IsApplicable(RE::TESObjectWEAP* a_data) const
	{
		for (const auto& internalForm : keywords) {
			if (!a_data->HasKeywordString(internalForm)) {
				if (!inverted) {
					return false;
				}
			}
			else {
				if (inverted) {
					return false;
				}
			}
		}
		return true;
	}

	bool Manager::EnchantmentKeywordCondition::IsApplicable(RE::EnchantmentItem* a_data) const
	{
		for (const auto& keywordString : keywords) {
			bool found = false;
			for (const auto& effect : a_data->effects) {
				if (effect->baseEffect ? effect->baseEffect->HasKeywordString(keywordString) : false) {
					found = true;
					break;
				}
			}

			if (!found) {
				return false;
			}
		}
		return true;
	}

	RE::BGSArtObject* Manager::GetBestMatchingArt(RE::TESObjectWEAP* a_weap, RE::EnchantmentItem* a_enchantment)
	{
		bool hasHighPriority = false;
		int lastWeight = 0;
		RE::BGSArtObject* bestMatchingArt = nullptr;


		for (const auto& art : storedArt) {
			if (hasHighPriority && lastWeight > art.weight) {
				return bestMatchingArt;
			}
			if (hasHighPriority && art.priority == Priority::kLow) {
				continue;
			}
			if (!art.enchantmentCondition.IsApplicable(a_enchantment)) {
				continue;
			}

			bool shouldSkip = false;
			for (auto it = art.conditions.begin(); !shouldSkip && it != art.conditions.end(); ++it) {
				if (!(*it)->IsApplicable(a_weap)) {
					shouldSkip = true;
				}
			}
			if (shouldSkip) {
				continue;
			}

			if (!hasHighPriority && art.priority == Priority::kHigh) {
				hasHighPriority = true;
				lastWeight = art.weight;
				bestMatchingArt = art.artObject;
			}
			else if (art.weight > lastWeight) {
				lastWeight = art.weight;
				bestMatchingArt = art.artObject;
			}
		}

		return bestMatchingArt;
	}

	void Manager::CreateNewData(RE::BGSArtObject* a_enchantmentArt, std::vector<std::string> a_enchantmentKeywords, std::vector<std::string> a_weaponKeywords, std::vector<std::string> a_excludedWeaponKeywords, std::vector<RE::TESObjectWEAP*> a_weapons, std::vector<RE::TESObjectWEAP*> a_excludedWeapons)
	{
		auto temp = ConditionalArt();
		temp.artObject = a_enchantmentArt;
		temp.enchantmentCondition = EnchantmentKeywordCondition();
		temp.enchantmentCondition.keywords = a_enchantmentKeywords;

		int newWeight = 0;
		if (!a_weapons.empty()) {
			newWeight++;
			auto newWeaponCondition = WeaponCondition();
			newWeaponCondition.weapons = a_weapons;
			auto newWeapons = std::make_unique<WeaponCondition>(newWeaponCondition);
			temp.conditions.push_back(std::move(newWeapons));
		}

		if (!a_excludedWeapons.empty()) {
			auto newReverseWeaponCondition = WeaponCondition();
			newReverseWeaponCondition.inverted = true;
			newReverseWeaponCondition.weapons = a_excludedWeapons;
			auto excludedWeapons = std::make_unique<WeaponCondition>(newReverseWeaponCondition);
			temp.conditions.push_back(std::move(excludedWeapons));
		}

		if (!a_weaponKeywords.empty()) {
			newWeight += a_weaponKeywords.size();
			auto newWeaponKeywordCondition = WeaponKeywordCondition();
			newWeaponKeywordCondition.keywords = a_weaponKeywords;
			auto newKeywords = std::make_unique<WeaponKeywordCondition>(newWeaponKeywordCondition);
			temp.conditions.push_back(std::move(newKeywords));
		}

		if (!a_excludedWeaponKeywords.empty()) {
			auto newExcludedWeaponKeywords = WeaponKeywordCondition();
			newExcludedWeaponKeywords.inverted = true;
			newExcludedWeaponKeywords.keywords = a_excludedWeaponKeywords;
			auto newExcludedKeywords = std::make_unique<WeaponKeywordCondition>(newExcludedWeaponKeywords);
			temp.conditions.push_back(std::move(newExcludedKeywords));
		}

		temp.weight = newWeight;
		storedArt.push_back(std::move(temp));
	}
}
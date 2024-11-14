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
		bool matchedAllKeywords = true;
		for (const auto& internalForm : keywords) {
			if (!a_data->HasKeywordString(internalForm)) {
				if (!inverted) {
					matchedAllKeywords = false;
				}
			}
			else {
				if (inverted) {
					return false;
				}
			}
		}
		return matchedAllKeywords;
	}

	bool Manager::EnchantmentKeywordCondition::IsApplicable(RE::EnchantmentItem* a_data) const
	{
		for (const auto& keywordString : keywords) {
			if (!a_data->HasKeywordString(keywordString)) {
				return false;
			}
		}
		return true;
	}

	RE::BGSArtObject* Manager::GetBestMatchingArt( RE::TESObjectWEAP* a_weap, RE::EnchantmentItem* a_enchantment)
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
			else if (art.weight > lastWeight){
				lastWeight = art.weight;
				bestMatchingArt = art.artObject;
			}
		}

		return bestMatchingArt;
	}

	void Manager::AddWeaponCondition(bool a_inverted, const std::vector<RE::TESObjectWEAP*>& a_weapons)
	{
		if (a_inverted) {
			tempWeaponConditionInverted.weapons = a_weapons;
			tempWeaponConditionInverted.inverted = true;
		}
		else {
			tempWeaponCondition.weapons = a_weapons;
			tempWeaponCondition.inverted = false;
		}
	}

	void Manager::AddWeaponKeywordCondition(bool a_inverted, const std::vector<std::string_view>& a_keywords)
	{
		if (a_inverted) {
			tempKeywordConditionsInverted.keywords = a_keywords;
			tempKeywordConditionsInverted.inverted = true;
		}
		else {
			tempKeywordConditions.keywords = a_keywords;
			tempKeywordConditions.inverted = false;
		}
	}

	void Manager::AddEnchantmentCondition(const std::vector<std::string_view>& a_keywords)
	{
		tempEnchantmentKeywordCondition.keywords = a_keywords;
	}

	void Manager::ReleaseNewCondition(bool a_matchAll, RE::BGSArtObject* a_artObject)
	{
		auto tempArt = ConditionalArt();
		tempArt.matchAll = a_matchAll;
		tempArt.artObject = a_artObject;
		tempArt.enchantmentCondition = tempEnchantmentKeywordCondition;

		if (tempWeaponCondition.weapons.empty()) {
			tempArt.priority = Priority::kLow;
		}
		else {
			tempArt.priority = Priority::kHigh;
		}

		if (!tempWeaponCondition.weapons.empty()) {
			auto newPtr = std::make_unique<WeaponCondition>(tempWeaponCondition);
			tempArt.conditions.push_back(std::move(newPtr));
		}

		if (!tempKeywordConditions.keywords.empty()) {
			auto newPtr = std::make_unique<WeaponKeywordCondition>(tempKeywordConditions);
			tempArt.conditions.push_back(std::move(newPtr));
		}

		if (!tempKeywordConditionsInverted.keywords.empty()) {
			auto newPtr = std::make_unique<WeaponKeywordCondition>(tempKeywordConditionsInverted);
			tempArt.conditions.push_back(std::move(newPtr));
		}

		if (!tempWeaponConditionInverted.weapons.empty()) {
			auto newPtr = std::make_unique<WeaponCondition>(tempWeaponConditionInverted);
			tempArt.conditions.push_back(std::move(newPtr));
		}

		storedArt.push_back(std::move(tempArt));

		tempEnchantmentKeywordCondition = EnchantmentKeywordCondition();
		tempKeywordConditions = WeaponKeywordCondition();
		tempWeaponCondition = WeaponCondition();
		tempKeywordConditionsInverted = WeaponKeywordCondition();
		tempWeaponConditionInverted = WeaponCondition();
	}
}
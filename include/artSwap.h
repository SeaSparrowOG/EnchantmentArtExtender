#pragma once

namespace ArtSwap {

	class ArtSwap {
	private:
		ArtSwapMode                      mode;                        //See the enum below for more information.
		std::string                      swapName;                    //Mostly used for debuging. 
		std::vector<std::string>         requiredWeaponKeywords;      //All the keyword strings here must be present on the weapon in order for it to receive the new VFX.
		std::vector<std::string>         excludeWeaponKeywords;       //The weapon must NOT have these keyword strings in order to receive the new VFX.
		std::vector<std::string>         requiredEnchantmentKeywords; //The enchantment on the weapon must have all these keywords in order for it to reveive the new VFX.
		RE::SpellItem*                   leftHandAbility;             //The spell (Ability) that applies the VFX to the actor (left hand).
		RE::SpellItem*                   rightHandAbility;
		std::vector <RE::TESObjectWEAP*> requiredWeapons;             //If the weapon matches any of these, the effect will be applied. If it is present, then weapon keywords are not taken into account.
		std::vector <RE::TESObjectWEAP*> ignoredWeapons;              //Any weapon that matches any of these will not have VFX applied.

		//Setters for adding/replacing/removing REQUIRED keywords
		void                             AddRequiredWeaponKeyword(std::string a_keywordString);
		void                             RemoveRequiredWeaponKeyword(std::string a_keywordString);

		//Setters for adding/replacing/removing EXCLUDED keywords
		void                             AddExcludedWeaponKeyword(std::string a_keywordString);
		void                             RemoveExcludedWeaponKeyword(std::string a_keywordString);

		//Setters for adding/replacing/removing individual enchantment requirements
		void                             AddRequiredEnchantmentKeyword(std::string a_keywordString);
		void                             RemoveRequiredEnchantmentKeyword(std::string a_keywordString);

		//Setters for adding/replacing/removing individual weapon requirements
		void                             AddRequiredWeapon(RE::TESObjectWEAP* a_weapon);
		void                             RemoveRequiredWeapon(RE::TESObjectWEAP* a_weapon);
		void                             AddExcludedWeapon(RE::TESObjectWEAP* a_weapon);
		void                             RemoveExcludedWeapon(RE::TESObjectWEAP* a_weapon);

		//Setters for the VFX abilities
		void                             SetAbility(RE::SpellItem* a_ability, bool a_bLeft = true);

	public:
		//Basic getters - these probably don't need to be public. But they will be for now.
		ArtSwapMode                      GetMode();
		RE::SpellItem*                   GetAbility(bool a_left = true);
		std::string                      GetName();
		std::vector<std::string>         GetRequiredWeaponKeywords();
		std::vector<std::string>         GetExcludedWeaponKeywords();
		std::vector<std::string>         GetRequiredEnchantmentKeywords();
		std::vector <RE::TESObjectWEAP*> GetRequiredWeapons();
		std::vector <RE::TESObjectWEAP*> GetIgnoredWeapons();
		void                             SetName(std::string a_name);

		//Returns the degree to which the given weapon and enchantment matches this swap, represented as a pair of bool/int. The int is the degree
		//of the match. The higher the better. The bool represents if the match is a "required weapon" match (valued more highly than the keywords).
		bool IsMatch(RE::TESObjectWEAP* a_weapon, RE::EnchantmentItem* a_enchantment);
		void SetAttributes(Json::Value a_swapData, std::string a_mode, std::string a_artSource, std::vector<std::string> a_requiredEnchantmentKeywords, RE::SpellItem* a_leftAbility, RE::SpellItem* a_rightAbility);
		void SetMode(std::string a_mode);
	};
}
#pragma once

namespace Cache {
//I might edit this in the future to be just in Data
#define configPath R"(Data/Enchantment Art Extender/)"

//The following are just here for some legibility.
#define swapDataVector std::vector<SwapData>
#define stringVector std::vector<std::string>
#define weaponVector std::vector<RE::TESObjectWEAP*>
#define spellVector std::vector<RE::SpellItem*>
#define enchantmentVector std::vector<RE::EnchantmentItem*>
#define abilityCache std::unordered_map<RE::TESObjectWEAP*, swapDataVector>

	/*
	This holds the data for a swap. It is only referenced in gameplay
	when an actor uses a weapon with an enchantment made by the player.
	Otherwise, it is just used once at startup to create the weapon cache.
	*/
	struct SwapData {
		/*
		We can fake each weapon having its own enchantment effect by assigning
		an ability to it to be played when it is equipped.
		*/
		RE::SpellItem*    leftAbility;
		RE::SpellItem*    rightAbility;
		std::string       artSource;
		std::string       name;
		bool              exclusive;
		stringVector      requiredEnchantmentKeywords;
		stringVector      requiredWeaponKeywords;
		stringVector      excludedWeaponKeywords;
		weaponVector      requiredWeapons;
		weaponVector      excludedWeapons;
		enchantmentVector requiredEnchantments;
	};

	/*
	The actual cache. It stores settings in the INI files and any configuration files
	present. Built when the data loads.
	*/
	class StoredData : public clib_util::singleton::ISingleton<StoredData> {
	public:
		bool           InitiateCache();
		bool           GetShouldAddLight();
		swapDataVector GetMatchingSwaps(RE::TESObjectWEAP* a_weap, RE::EnchantmentItem* a_enchantment);
		spellVector    GetAllAbilities();

		RE::TESObjectLIGH* lightObject;
		RE::SpellItem*     enchantmentLightRight;
		RE::SpellItem*     enchantmentLightLeft;
		RE::SpellItem*     enchantmentLightBoth;
	private:
		bool               shouldDisableShaders;
		bool               shouldAddLights;
		spellVector        storedAbilities;
		swapDataVector     storedSwaps;
		abilityCache       weaponCache;
		std::unordered_map<RE::TESObjectWEAP*, bool> invalidWeapons;

		/**
		* Registers pre-enchanted weapons to speed up the framework.
		* @return True on success, false on failure.
		*/
		bool RegisterReadyWeapons();

		/**
		* Given an array of validated configs, applies all valid settings.
		* @param a_validConfigs The vector of the configs.
		*/
		void RegisterSwaps(std::vector<Json::Value> a_validConfigs);

		/**
		* Applies the settings found in the INI file.
		* @return True if the operation succeeds, false otherwise.
		*/
		bool ApplyINISettings();

		/**
		* Prints all stored swaps. Allows for easier debugging.
		* @return Nothing.
		*/
		void DebugSwaps();
	};
}
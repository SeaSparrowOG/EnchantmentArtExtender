#pragma once

namespace EffectManager
{
	struct ArtSwapProxy
	{
		RE::BGSArtObject* _artObject = nullptr;
		std::vector<RE::BGSKeyword*> _requiredWeaponKeywords{};
		std::vector<RE::BGSKeyword*> _requiredEnchantmentKeywords{};

		std::vector<RE::EnchantmentItem*> _allowedEnchantments{};
		std::vector<RE::TESObjectWEAP*>   _allowedWeapons{};

		std::vector<RE::BGSKeyword*> _forbiddenEnchantmentKeywords;
		std::vector<RE::BGSKeyword*> _forbiddenWeaponKeywords;
		std::vector<RE::TESObjectWEAP*>   _forbiddenWeapons;
		std::vector<RE::EnchantmentItem*> _forbiddenEnchantments;
	};

	class EffectDistributor : 
		public REX::Singleton<EffectDistributor>
	{
	public:
		void TryApplyEffect(RE::WeaponEnchantmentController* controller, RE::EnchantmentItem* enchant) const;
		bool AppendArtSwap(ArtSwapProxy& proxy);

	private:
		enum class PriorityLevel
		{
			None,
			EnchantOfTypeToWeapOfType,
			EnchantOfTypeToSpecificWeap,
			SpecificEnchantToWeapOfType,
			SpecificEnchantToSpecificWeap
		};

		class ArtSwap
		{
		public:
			[[nodiscard]] bool Finalize();
			void SetArtObject(RE::BGSArtObject* obj);
			void AddWeaponCondition(RE::TESObjectWEAP* weap);
			void AddEnchantmentCondition(RE::EnchantmentItem* enchant);
			void AddEnchKeywordCondition(RE::BGSKeyword* keyword);
			void AddWeapKeywordCondition(RE::BGSKeyword* keyword);

			void AddForbiddenEnchantment(RE::EnchantmentItem* ench);
			void AddForbiddenEnchantmentKeyword(RE::BGSKeyword* kwd);
			void AddForbiddenWeapon(RE::TESObjectWEAP* weap);
			void AddForbiddenWeaponKeyword(RE::BGSKeyword* kwd);

			[[nodiscard]] int GetStrictnessLevel() const;
			[[nodiscard]] bool CanApply(const RE::TESObjectWEAP* weap,
				const RE::EnchantmentItem* ench) const;

			[[nodiscard]] PriorityLevel GetPriorityLevel() const;
			[[nodiscard]] RE::BGSArtObject* GetArtObject() const;
		private:
			RE::BGSArtObject* _artObject = nullptr;
			std::unordered_set<RE::FormID> _requiredWeapons;
			std::unordered_set<RE::FormID> _requiredEnchantments;
			std::unordered_set<RE::FormID> _forbiddenWeapons;
			std::unordered_set<RE::FormID> _forbiddenEnchantments;

			std::vector<RE::FormID> _requiredEnchantKeywords;
			std::vector<RE::FormID> _requiredWeaponKeywords;
			std::vector<RE::FormID> _forbiddenEnchantmentKeywords;
			std::vector<RE::FormID> _forbiddenWeaponKeywords;

			int           _strictness = -1;
			PriorityLevel _level = PriorityLevel::None;
		};

		std::vector<ArtSwap> _swaps;
	};

	inline void TryToSwapArt(RE::WeaponEnchantmentController* controller, 
		RE::MagicItem* enchantment)
	{
		if (!controller || !enchantment) {
			LOG_DEBUG("  - No Controller, or no Enchantment."sv);
			return;
		}
		static const auto* distributor = EffectDistributor::GetSingleton();
		if (!distributor) {
			LOG_DEBUG("  - Massive internal error."sv);
			return;
		}
		
		auto* asEnchant = skyrim_cast<RE::EnchantmentItem*>(enchantment);
		if (!asEnchant) {
			LOG_DEBUG("  - Couldn't cast as enchantment"sv);
			return;
		}
		distributor->TryApplyEffect(controller, asEnchant);
	}

	[[nodiscard]] bool ReadConfigs();
}
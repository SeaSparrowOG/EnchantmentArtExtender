#pragma once

#include "utilities/utilities.h"

namespace EnchantmentManager
{
	class Manager : public Utilities::Singleton::ISingleton<Manager> {
	public:
		RE::BGSArtObject* GetBestMatchingArt(RE::TESObjectWEAP* a_actor, RE::EnchantmentItem* a_enchantment);
		void CreateNewData(
			RE::BGSArtObject* a_enchantmentArt,
			std::vector<std::string> a_enchantmentKeywords,
			std::vector<std::string> a_weaponKeywords,
			std::vector<std::string> a_excludedWeaponKeywords,
			std::vector<RE::TESObjectWEAP*> a_weapons,
			std::vector<RE::TESObjectWEAP*> a_excludedWeapons
		);

	private:
		enum Priority {
			kLow,
			kHigh
		};

		class Condition {
		public:
			virtual bool IsApplicable(RE::TESObjectWEAP* a_data) const = 0;
			bool inverted;
		};

		class WeaponCondition : public Condition {
		public:
			bool IsApplicable(RE::TESObjectWEAP* a_data) const override;
			std::vector<RE::TESObjectWEAP*> weapons;

			WeaponCondition() {
				weapons = std::vector<RE::TESObjectWEAP*>();
			}
		};

		class WeaponKeywordCondition : public Condition {
		public:
			bool IsApplicable(RE::TESObjectWEAP* a_data) const override;
			std::vector<std::string> keywords;

			WeaponKeywordCondition() {
				keywords = std::vector<std::string>();
			}
		};

		class EnchantmentKeywordCondition {
		public:
			bool IsApplicable(RE::EnchantmentItem* a_data) const;
			std::vector<std::string> keywords;

			EnchantmentKeywordCondition() {
				keywords = std::vector<std::string>();
			}
		};

		class ConditionalArt {
		public:
			int weight;
			bool matchAll;
			Priority priority;
			RE::BGSArtObject* artObject;
			std::vector<std::unique_ptr<Condition>> conditions;
			EnchantmentKeywordCondition enchantmentCondition;

			ConditionalArt() {
				weight = 0;
				matchAll = false;
				priority = Priority::kLow;
				artObject = nullptr;
				conditions = std::vector<std::unique_ptr<Condition>>();
				enchantmentCondition = EnchantmentKeywordCondition();
			}
		};

		std::vector<ConditionalArt> storedArt;
	};
}
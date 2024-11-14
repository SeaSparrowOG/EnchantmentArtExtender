#pragma once

#include "utilities/utilities.h"

namespace EnchantmentManager
{
	class Manager : public Utilities::Singleton::ISingleton<Manager> {
	public:
		RE::BGSArtObject* GetBestMatchingArt(RE::TESObjectWEAP* a_weap, RE::EnchantmentItem* a_enchantment);
		void AddWeaponCondition(bool a_inverted, const std::vector<RE::TESObjectWEAP*>& a_weapons);
		void AddWeaponKeywordCondition(bool a_inverted, const std::vector<std::string_view>& a_keywords);
		void AddEnchantmentCondition(const std::vector<std::string_view>& a_keywords);
		void ReleaseNewCondition(bool a_matchAll, RE::BGSArtObject* a_artObject);

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
			std::vector<std::string_view> keywords;

			WeaponKeywordCondition() {
				keywords = std::vector<std::string_view>();
			}
		};

		class EnchantmentKeywordCondition {
		public:
			bool IsApplicable(RE::EnchantmentItem* a_data) const;
			std::vector<std::string_view> keywords;

			EnchantmentKeywordCondition() {
				keywords = std::vector<std::string_view>();
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

		EnchantmentKeywordCondition tempEnchantmentKeywordCondition;
		WeaponKeywordCondition tempKeywordConditions;
		WeaponCondition tempWeaponCondition;
		WeaponKeywordCondition tempKeywordConditionsInverted;
		WeaponCondition tempWeaponConditionInverted;
	};
}
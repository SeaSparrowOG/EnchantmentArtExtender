#pragma once

#include "utilities/utilities.h"

namespace EnchantmentManager
{
	class Manager : public Utilities::Singleton::ISingleton<Manager> {
	public:
		void ProcessActor(RE::Actor* a_actor, bool a_clear);

	private:
		enum Priority {
			kLow,
			kHigh
		};

		struct Condition {
			virtual bool IsApplicable(RE::InventoryEntryData* a_data) const = 0;

			bool inverted;
			int weight;
			Priority priority;
		};

		struct WeaponCondition : public Condition{
			bool IsApplicable(RE::InventoryEntryData* a_data) const override;

			WeaponCondition() {
				priority = kHigh;
			}
			bool inverted;
		};

		struct WeaponKeywordCondition : public Condition {
			bool IsApplicable(RE::InventoryEntryData* a_data) const override;

			WeaponKeywordCondition() {
				priority = kLow;
			}
			std::vector<std::string_view> keywords;
		};

		struct EnchantmentKeywordCondition {
			bool IsApplicable(RE::EnchantmentItem* a_data) const;

			std::vector<std::string_view> keywords;
		};

		struct ConditionalArt {
			bool IsApplicable(RE::EnchantmentItem* a_enchantment, RE::TESObjectWEAP* a_weapon);

			bool matchAll;
			RE::BGSArtObject* artObject;
			std::vector<Condition> conditions;
		};

		std::vector<ConditionalArt> storedArt;
	};
}
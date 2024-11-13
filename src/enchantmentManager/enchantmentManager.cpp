#include "enchantmentManager/enchantmentManager.h"

namespace EnchantmentManager
{
	void Manager::ProcessActor(RE::Actor* a_actor, bool a_clear)
	{
		if (a_clear) {
			
		}
		else {
		}
	}

	bool Manager::WeaponCondition::IsApplicable(RE::InventoryEntryData* a_data) const
	{
		return false;
	}

	bool Manager::WeaponKeywordCondition::IsApplicable(RE::InventoryEntryData* a_data) const
	{
		return false;
	}

	bool Manager::EnchantmentKeywordCondition::IsApplicable(RE::EnchantmentItem* a_data) const
	{
		return false;
	}

	bool Manager::ConditionalArt::IsApplicable(RE::EnchantmentItem* a_enchantment, RE::TESObjectWEAP* a_weapon)
	{
		return false;
	}
}
#include "Hooks/hooks.h"

#include "enchantmentManager/enchantmentManager.h"
#include "Settings/INISettings.h"

namespace Hooks {
	void Install()
	{
		AttachEnchantmentVisuals::Install();
	}
	RE::WeaponEnchantmentController* AttachEnchantmentVisuals::AttachArt(
		RE::WeaponEnchantmentController* a_this,
		RE::ActorMagicCaster* a_magicCaster,
		RE::Actor* a_caster,
		RE::MagicItem* a_enchantment)
	{
		const auto response = _attachArt(a_this, a_magicCaster, a_caster, a_enchantment);
		if (response->artObject) {
			return response;
		}

		const auto left = a_magicCaster->castingSource == RE::MagicSystem::CastingSource::kLeftHand;
		const auto entry = a_caster->GetEquippedEntryData(left);
		const auto weap = entry && entry->object ? entry->object->As<RE::TESObjectWEAP>() : nullptr;
		const auto enchantment = entry ? entry->GetEnchantment() : nullptr;
		if (weap && enchantment) {
			const auto art = EnchantmentManager::Manager::GetSingleton()->GetBestMatchingArt(weap, enchantment);
			response->artObject = art;

			if (art) {
				const auto settingsSingleton = Settings::INI::SettingsHolder::GetSingleton();
				if (const auto emptyShader = settingsSingleton->GetEmptyShader(); settingsSingleton->useEmptyShader && emptyShader) {
					response->effectShader = emptyShader;
				}
			}
		}
		return response;
	}
}
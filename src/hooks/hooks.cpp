#include "Hooks/hooks.h"

#include "enchantmentManager/enchantmentManager.h"

namespace Hooks {
	void Install()
	{
		AttachEnchantmentVisuals::Install();
	}
	RE::WeaponEnchantmentController* AttachEnchantmentVisuals::AttachArt(
		RE::ReferenceEffectController* a_this,
		RE::ActorMagicCaster* a_magicCaster,
		RE::Actor* a_caster,
		RE::MagicItem* a_enchantment)
	{
		const auto response = _attachArt(a_this, a_magicCaster, a_caster, a_enchantment);
		const auto enchantment = a_enchantment ? a_enchantment->As<RE::EnchantmentItem>() : nullptr;
		if (enchantment) {
			const auto newArt = EnchantmentManager::Manager::GetSingleton()->GetBestMatchingArt(response->weapon, enchantment);
			if (newArt) {
				response->artObject = newArt;
			}
		}
		return response;
	}
}
#include "Hooks/hooks.h"

#include "enchantmentManager/enchantmentManager.h"

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
		const auto enchantment = a_enchantment ? a_enchantment->As<RE::EnchantmentItem>() : nullptr;
		const auto attachRoot = response->GetAttachRoot();
		const auto attachRootName = attachRoot ? attachRoot->name : "";
		bool isLeft = attachRootName.contains("Shield");
		const auto equippedObject = a_caster->GetEquippedObject(isLeft);
		const auto weap = equippedObject ? equippedObject->As<RE::TESObjectWEAP>() : nullptr;
		logger::debug("WEAP: {}\n  >Root: {}", weap ? weap->GetName() : "NULL", attachRootName.c_str());
		if (enchantment && weap) {
			logger::debug("Requesting new art");
			const auto newArt = EnchantmentManager::Manager::GetSingleton()->GetBestMatchingArt(weap, enchantment);
			if (newArt) {
				response->artObject = newArt;
			}
		}
		return response;
	}
}
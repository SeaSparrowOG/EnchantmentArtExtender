#include "Hooks/hooks.h"

#include "EffectManager/EffectManager.h"
#include "RE/Offset.h"

namespace Hooks {
	bool Install() {
		REX::INFO("Installing hooks..."sv);
		bool result = true;
		result &= AttachEnchantmentVisuals::InstallAttachPatch();
		return result;
	}

	bool AttachEnchantmentVisuals::InstallAttachPatch() {
		REX::INFO("  - Installing Attach Enchantment Visuals patch"sv);
		REL::Relocation<std::uintptr_t> target = RE::Offset::WeaponEnchantmentController::AttachArt;
		if (!REL::Pattern<"E8">().match(target.address())) {
			REX::CRITICAL("    >Failed to match pattern."sv);
			return false;
		}

		auto& trampoline = REL::GetTrampoline();
		_attachArt = trampoline.write_call<5>(target.address(), &AttachArt);
		return true;
	}

	RE::WeaponEnchantmentController* AttachEnchantmentVisuals::AttachArt(RE::WeaponEnchantmentController* a_controller,
		RE::ActorMagicCaster* caster,
		RE::Actor* actor,
		RE::MagicItem* enchantment)
	{
		auto* controller = _attachArt(a_controller, caster, actor, enchantment);
		LOG_DEBUG("Fired!"sv);
		EffectManager::TryToSwapArt(controller, enchantment);
		LOG_DEBUG("Finished... {}"sv, controller ? "VALID" : "NULL");
		return controller;
	}
}
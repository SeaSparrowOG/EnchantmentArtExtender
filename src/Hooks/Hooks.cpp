#include "Hooks/hooks.h"

#include "EffectManager/EffectManager.h"
#include "RE/Offset.h"

namespace Hooks {
	bool Install() {
		logger::info("Installing hooks..."sv);
		size_t allocSize = 14u * 1u;
		if (allocSize > 0u) {
			logger::info("  - Allocated {} bytes to the trampoline."sv, allocSize);
			SKSE::AllocTrampoline(allocSize);
		}
		else {
			logger::info("  - Did not need to allocate trampoline space."sv);
		}

		bool result = true;
		result &= AttachEnchantmentVisuals::InstallAttachPatch();
		return result;
	}

	bool AttachEnchantmentVisuals::InstallAttachPatch() {
		logger::info("  - Installing Attach Enchantment Visuals patch"sv);
		REL::Relocation<std::uintptr_t> target = RE::Offset::WeaponEnchantmentController::AttachArt;
		if (!REL::make_pattern<"E8">().match(target.address())) {
			logger::critical("    >Failed to match pattern."sv);
			return false;
		}

		auto& trampoline = SKSE::GetTrampoline();
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
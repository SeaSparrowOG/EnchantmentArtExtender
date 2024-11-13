#pragma once

#include "RE/WeaponEnchantmentController.h"

namespace Hooks {
	void Install();

	class AttachEnchantmentVisuals {
	public:
		static void Install() {
			auto& trampoline = SKSE::GetTrampoline();

			REL::Relocation<std::uintptr_t> target{ REL::ID(34154), 0xDB };
			_attachArt = trampoline.write_call<5>(target.address(), &AttachArt);
		}

	private:
		static RE::WeaponEnchantmentController* AttachArt(
			RE::ReferenceEffectController* a_speaker,
			RE::ActorMagicCaster* a_magicCaster,
			RE::Actor* a_caster,
			RE::MagicItem* a_enchantment);

		inline static REL::Relocation<decltype(&AttachArt)> _attachArt;
	};
}
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

			//REL::Relocation<std::uintptr_t> VTBL{ RE::ShaderReferenceEffect::VTABLE[0] };
			//_detatchLight = VTBL.write_vfunc(0x3E, &DetatchImpl);
		}

	private:
		static RE::WeaponEnchantmentController* AttachArt(
			RE::WeaponEnchantmentController* a_speaker,
			RE::ActorMagicCaster* a_magicCaster,
			RE::Actor* a_caster,
			RE::MagicItem* a_enchantment);
		static void DetatchImpl(RE::ShaderReferenceEffect* a_this);

		inline static REL::Relocation<decltype(&AttachArt)> _attachArt;
		inline static REL::Relocation<decltype(&DetatchImpl)> _detatchLight;
	};
}
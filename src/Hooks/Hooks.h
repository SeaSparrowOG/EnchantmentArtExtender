#pragma once

namespace Hooks {
	bool Install();

	struct AttachEnchantmentVisuals
	{
		static bool InstallAttachPatch();
		static RE::WeaponEnchantmentController* AttachArt(
			RE::WeaponEnchantmentController* a_controller,
			RE::ActorMagicCaster* caster,
			RE::Actor* actor,
			RE::MagicItem* enchantment);
		inline static REL::Relocation<decltype(&AttachArt)> _attachArt;
	};
}
#pragma once

namespace ActorEvents {

	struct DrawWeaponMagicHands
	{
		static bool Install() {
			stl::write_vfunc<RE::PlayerCharacter, DrawWeaponMagicHands>();
			return true;
		}

		static void thunk(RE::Actor* a_this, bool a_draw);
		inline static REL::Relocation<decltype(DrawWeaponMagicHands::thunk)> func;
		static constexpr std::size_t idx{ 166 }; //0xA6
	};

	void Install();
}
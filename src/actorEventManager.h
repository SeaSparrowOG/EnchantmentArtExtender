#pragma once

namespace ActorEvents {
#define continueEvent RE::BSEventNotifyControl::kContinue

	struct PlayShader
	{
		static bool   Install() {
			REL::Relocation<std::uintptr_t> target{ REL::ID(34154), 0xDB };;
			stl::write_thunk_call<PlayShader>(target.address());
			return true;
		}

		//ReferenceEffectController*, ActorMagicCaster, Actor* (target?), HighProcessData*
		static void thunk(void* arg1, RE::ActorMagicCaster* arg2, void* arg3, void* arg4);
		inline static REL::Relocation<decltype(PlayShader::thunk)> func;
	};

	struct ClearShader
	{
		static bool Install() {
			REL::Relocation<std::uintptr_t> target1{ REL::ID(38784), 0xB1 };
			stl::write_thunk_call<ClearShader>(target1.address());

			REL::Relocation<std::uintptr_t> target2{ REL::ID(34194), 0x45 };
			stl::write_thunk_call<ClearShader>(target2.address());

			REL::Relocation<std::uintptr_t> target3{ REL::ID(34156), 0x31 };
			stl::write_thunk_call<ClearShader>(target3.address());

			REL::Relocation<std::uintptr_t> target4{ REL::ID(34135), 0x3E };
			stl::write_thunk_call<ClearShader>(target4.address());
			return true;
		}

		static void thunk(RE::ActorMagicCaster* arg1, bool arg2, void* arg3, void* arg4);
		inline static REL::Relocation<decltype(ClearShader::thunk)> func;
	};
}
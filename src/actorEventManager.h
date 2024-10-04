#pragma once

namespace ActorEvents {

	struct Init
	{
		static bool Install() {
			stl::write_vfunc<RE::ShaderReferenceEffect, Init>();
			return true;
		}

		static bool thunk(RE::ShaderReferenceEffect* a_this);
		inline static REL::Relocation<decltype(Init::thunk)> func;
		static constexpr std::size_t idx{ 54 };
	};

	struct Attach
	{
		static bool Install() {
			stl::write_vfunc<RE::ShaderReferenceEffect, Attach>();
			return true;
		}

		static bool thunk(RE::ShaderReferenceEffect* a_this);
		inline static REL::Relocation<decltype(Attach::thunk)> func;
		static constexpr std::size_t idx{ 57 };
	};

	void Install();
}
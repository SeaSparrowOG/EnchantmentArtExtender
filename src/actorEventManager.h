#pragma once

namespace ActorEvents {

	struct Init
	{
		static bool Install() {
			stl::write_vfunc<RE::ShaderReferenceEffect, Init>();
			return true;
		}

		static void thunk(RE::ShaderReferenceEffect* a_this);
		inline static REL::Relocation<decltype(Init::thunk)> func;
		static constexpr std::size_t idx{ 54 }; //0x36
	};

	struct ClearShader
	{
		static bool Install() {
			SKSE::AllocTrampoline(14);
			REL::Relocation<std::uintptr_t> target{ REL::ID(38784), 0xB1 };

			auto& trampoline = SKSE::GetTrampoline();
			func = trampoline.write_call<5>(target.address(), thunk);
			return true;
		}

		static void thunk(RE::ActorMagicCaster* a_this, bool arg2, void* arg3, void* arg4);

		inline static REL::Relocation<decltype(ClearShader::thunk)> func;
		static constexpr std::size_t idx{ 57 }; //0x37
	};

	class EquipEventHandler : public RE::BSTEventSink<RE::TESEquipEvent>,
		public ISingleton<EquipEventHandler> {
	public:
		bool RegisterListener();

	private:
		RE::BSEventNotifyControl ProcessEvent(const RE::TESEquipEvent* a_event, RE::BSTEventSource<RE::TESEquipEvent>* a_eventSource) override;
	};

	void Install();
}
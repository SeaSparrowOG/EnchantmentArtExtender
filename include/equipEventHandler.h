#pragma once
#include "singletonHolder.h"

namespace EquipEventHandler {

	class EquipEvent : public RE::BSTEventSink<RE::TESEquipEvent> {

	public:

		static EquipEvent*       GetSingleton();
		static void              RegisterListener();
		RE::BSEventNotifyControl ProcessEvent(const RE::TESEquipEvent* a_event, RE::BSTEventSource<RE::TESEquipEvent>* a_eventSource) override;
	};
}
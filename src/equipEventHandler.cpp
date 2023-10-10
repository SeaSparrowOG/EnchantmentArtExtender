#include "equipEventHandler.h"

namespace EquipEventHandler {

	EquipEvent* EquipEvent::GetSingleton() {
		static EquipEvent event;
		return &event;
	}

	void EquipEvent::RegisterListener() {
		RE::ScriptEventSourceHolder* eventHolder = RE::ScriptEventSourceHolder::GetSingleton();
		eventHolder->AddEventSink(EquipEventHandler::EquipEvent::GetSingleton());
	}

	RE::BSEventNotifyControl EquipEvent::ProcessEvent(const RE::TESEquipEvent* a_event, RE::BSTEventSource<RE::TESEquipEvent>* a_eventSource) {
		//I am just lazy. This is bad practice. 
		auto response = RE::BSEventNotifyControl::kContinue;

		if (!a_event || !a_eventSource) {
			return response;
		}

		const auto reference = a_event->actor;
		const auto actor = a_event ? reference->As<RE::Actor>() : nullptr;

		if (!actor) {
			return response;
		}

		const auto baseObject = a_event->baseObject;
		const auto baseForm = a_event ? RE::TESObject::LookupByID(baseObject) : nullptr;
		const auto baseWeapon = baseForm ? baseForm->As<RE::TESObjectWEAP>() : nullptr;

		if (a_event->equipped) {

		}
		else {

		}

		return response;
	}


	//From here: These are just for readability.
	void UnequipHandler(RE::Actor* a_actor, RE::TESObjectWEAP* a_actorWeapon) {

	}
}
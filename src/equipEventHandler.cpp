#include "equipEventHandler.h"
#include "singletonHolder.h"
#include "helperFunctions.h"

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

		//Guard - Check that an actor equipped
		const auto reference = a_event->actor;
		const auto actor = a_event ? reference->As<RE::Actor>() : nullptr;
		if (!actor) return response;

		//Guard - Check that a weapon was equipped
		const auto baseObject = a_event->baseObject;
		const auto baseForm = a_event ? RE::TESObject::LookupByID(baseObject) : nullptr;
		const auto baseWeapon = baseForm ? baseForm->As<RE::TESObjectWEAP>() : nullptr;
		if (!baseWeapon) return response;

		helperFunction::EvaluateAbilities(actor);

		return response;
	}
}
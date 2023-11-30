#include "actorEventHandler.h"
#include "singletonHolder.h"
#include "helperFunctions.h"

namespace ActorEventHandler {
	RE::BSEventNotifyControl ActorLoadedEvent::ProcessEvent(const RE::TESObjectLoadedEvent* a_event, RE::BSTEventSource<RE::TESObjectLoadedEvent>* a_eventSource) {

		if (!(a_event && a_eventSource)) return RE::BSEventNotifyControl::kContinue;

		RE::FormID eventID = a_event->formID;
		auto eventForm = eventID ? RE::TESForm::LookupByID(eventID) : nullptr;
		auto eventActor = eventForm ? eventForm->As<RE::Actor>() : nullptr;

		if (!eventActor) return RE::BSEventNotifyControl::kContinue;

		if (a_event->loaded) {
			//Guard - Check that a weapon was equipped
			RE::TESForm* leftForm = eventActor->GetEquippedObject(true);
			RE::TESObjectWEAP* leftWeapon = leftForm ? leftForm->As<RE::TESObjectWEAP>() : nullptr;
			RE::TESForm* rightForm = eventActor->GetEquippedObject(false);
			RE::TESObjectWEAP* rightWeapon = rightForm ? rightForm->As<RE::TESObjectWEAP>() : nullptr;

			if (!(rightWeapon && leftWeapon)) return RE::BSEventNotifyControl::kContinue;

			helperFunction::EvaluateAbilities(eventActor);
		}

		return RE::BSEventNotifyControl::kContinue;
	}

	ActorLoadedEvent* ActorLoadedEvent::GetSingleton() {
		static ActorLoadedEvent singleton;
		return &singleton;
	}

	void ActorLoadedEvent::RegisterActorLoadedEvent() {
		RE::ScriptEventSourceHolder* eventHolder = RE::ScriptEventSourceHolder::GetSingleton();
		eventHolder->AddEventSink(ActorLoadedEvent::GetSingleton());
	}



	RE::BSEventNotifyControl ActorDeathEvent::ProcessEvent(const RE::TESDeathEvent* a_event, RE::BSTEventSource<RE::TESDeathEvent>* a_eventSource) {

		if (!(a_event && a_eventSource)) return RE::BSEventNotifyControl::kContinue;
		auto dyingRefptr = a_event->actorDying;
		auto dyingRef = dyingRefptr ? dyingRefptr.get() : nullptr;
		auto dyingActor = dyingRef ? dyingRef->As<RE::Actor>() : nullptr;

		if (!dyingActor) return RE::BSEventNotifyControl::kContinue;

		auto* allAbilities = SingletonHolder::ConditionHolder::GetSingleton()->GetAllAbilities();

		for (auto* ability : *allAbilities) {
			if (dyingActor->HasSpell(ability)) dyingActor->RemoveSpell(ability);
		}

		return RE::BSEventNotifyControl::kContinue;
	}

	ActorDeathEvent* ActorDeathEvent::GetSingleton() {
		static ActorDeathEvent singleton;
		return &singleton;
	}

	void ActorDeathEvent::RegisterActorDeathEvent() {
		RE::ScriptEventSourceHolder* eventHolder = RE::ScriptEventSourceHolder::GetSingleton();
		eventHolder->AddEventSink(ActorDeathEvent::GetSingleton());
	}
}
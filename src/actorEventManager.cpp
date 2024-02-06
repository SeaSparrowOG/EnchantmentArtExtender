#include "actorEventManager.h"
#include "cache.h"

namespace {
	bool SpellVectorContainsElement(RE::SpellItem* a_spell, std::vector<RE::SpellItem*>* a_vec) {
		for (auto* spell : *a_vec) {
			if (a_spell == spell) return true;
		}
		return false;
	}

	void EvaluateActor(RE::Actor* a_actor) {
		if (!a_actor) return;

		auto* rightData = a_actor->GetEquippedEntryData(false);
		bool rightEnchanted = rightData ? rightData->IsEnchanted() : false;
		auto* rightBound = rightEnchanted ? rightData->object : nullptr;
		auto* rightWeapon = rightBound ? rightBound->As<RE::TESObjectWEAP>() : nullptr;

		auto* leftData = a_actor->GetEquippedEntryData(true);
		bool leftEnchanted = leftData ? leftData->IsEnchanted() : false;
		auto* leftBound = leftEnchanted ? leftData->object : nullptr;
		auto* leftWeapon = leftBound ? leftBound->As<RE::TESObjectWEAP>() : nullptr;

		std::vector<RE::SpellItem*> abilities = std::vector<RE::SpellItem*>();
		if (rightWeapon) {
			auto* enchantment = rightWeapon->formEnchanting;
			if (!enchantment) enchantment = rightData->GetEnchantment();

			auto swaps = Cache::StoredData::GetSingleton()->GetMatchingSwaps(rightWeapon, enchantment);
			for (auto& swap : swaps) {
				auto* rightAbility = swap.rightAbility;
				if (!SpellVectorContainsElement(rightAbility, &abilities))
					abilities.push_back(rightAbility);
			}
		}

		if (leftWeapon) {
			auto* enchantment = leftWeapon->formEnchanting;
			if (!enchantment) enchantment = rightData->GetEnchantment();

			auto swaps = Cache::StoredData::GetSingleton()->GetMatchingSwaps(leftWeapon, enchantment);
			for (auto& swap : swaps) {
				auto* leftAbility = swap.leftAbility;
				if (!SpellVectorContainsElement(leftAbility, &abilities))
					abilities.push_back(leftAbility);
			}
		}

		for (auto* spell : abilities) {
			a_actor->AddSpell(spell);
		}

		std::vector<RE::SpellItem*> allAbilities = Cache::StoredData::GetSingleton()->GetAllAbilities();
		for (auto* spell : allAbilities) {
			if (!SpellVectorContainsElement(spell, &abilities))
				a_actor->RemoveSpell(spell);
		}
	}
}

namespace ActorEvents {

	/*
	* ====================================
	* Register handlers for the listeners.
	* ====================================
	*/
	bool LoadEvent::Register() {
		this->registered = false;
		auto* eventHolder = RE::ScriptEventSourceHolder::GetSingleton();
		if (!eventHolder) {
			_loggerInfo("Failed to get the Event Holder, aborting load...");
			return false;
		}

		eventHolder->AddEventSink(this);
		this->registered = true;
		return true;
	}

	bool EquipEvent::Register() {
		this->registered = false;
		auto* eventHolder = RE::ScriptEventSourceHolder::GetSingleton();
		if (!eventHolder) {
			_loggerInfo("    Failed to get the Event Holder, aborting load...");
			return false;
		}

		eventHolder->AddEventSink(this);
		this->registered = true;
		return true;
	}

	bool DeathEvent::Register() {
		this->registered = false;
		auto* eventHolder = RE::ScriptEventSourceHolder::GetSingleton();
		if (!eventHolder) {
			_loggerInfo("    Failed to get the Event Holder, aborting load...");
			return false;
		}

		eventHolder->AddEventSink(this);
		this->registered = true;
		return true;
	}

	/*
	* =======================================
	* De-Register handlers for the listeners.
	* =======================================
	*/
	void LoadEvent::UnRegister() {
		auto* eventHolder = RE::ScriptEventSourceHolder::GetSingleton();
		if (!eventHolder || !this->registered) return;

		eventHolder->RemoveEventSink(this);
		return;
	}

	void EquipEvent::UnRegister() {
		auto* eventHolder = RE::ScriptEventSourceHolder::GetSingleton();
		if (!eventHolder || !this->registered) return;

		eventHolder->RemoveEventSink(this);
		return;
	}

	void DeathEvent::UnRegister() {
		auto* eventHolder = RE::ScriptEventSourceHolder::GetSingleton();
		if (!eventHolder || !this->registered) return;

		eventHolder->RemoveEventSink(this);
		return;
	}
}

namespace ActorEvents {
	/*
	* ====================================
	* Event Processing Blocks
	* ====================================
	*/
	RE::BSEventNotifyControl LoadEvent::ProcessEvent(const RE::TESObjectLoadedEvent* a_event, RE::BSTEventSource<RE::TESObjectLoadedEvent>* a_eventSource) {
		if (!(a_event && a_eventSource)) return continueEvent;
		if (!a_event->loaded) return continueEvent;

		RE::FormID eventID = a_event->formID;
		auto eventForm = eventID ? RE::TESForm::LookupByID(eventID) : nullptr;
		auto eventActor = eventForm ? eventForm->As<RE::Actor>() : nullptr;
		if (!eventActor) return RE::BSEventNotifyControl::kContinue;
		
		EvaluateActor(eventActor);
		return continueEvent;
	}

	RE::BSEventNotifyControl EquipEvent::ProcessEvent(const RE::TESEquipEvent* a_event, RE::BSTEventSource<RE::TESEquipEvent>* a_eventSource) {
		if (!a_event || !a_eventSource) return continueEvent;

		auto reference = a_event->actor;
		auto* actor = a_event ? reference->As<RE::Actor>() : nullptr;
		if (!actor) return continueEvent;

		EvaluateActor(actor);
		return continueEvent;
	}

	RE::BSEventNotifyControl DeathEvent::ProcessEvent(const RE::TESDeathEvent* a_event, RE::BSTEventSource<RE::TESDeathEvent>* a_eventSource) {
		if (!(a_event && a_eventSource)) return continueEvent;
		auto eventActorPtr = a_event->actorDying;
		auto* eventActorRefr = eventActorPtr ? eventActorPtr.get() : nullptr;
		auto* eventActor = eventActorRefr ? eventActorRefr->As<RE::Actor>() : nullptr;

		EvaluateActor(eventActor);
		return continueEvent;
	}
}
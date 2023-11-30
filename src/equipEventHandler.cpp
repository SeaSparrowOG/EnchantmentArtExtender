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

	void EvaluateAbilities(RE::Actor* a_actor) {
		auto weaponCache = SingletonHolder::ConditionHolder::GetSingleton()->GetWeaponCache();
		std::vector<ArtSwap::ArtSwap> foundLeftSwaps;
		std::vector<ArtSwap::ArtSwap> foundRightSwaps;
		std::vector<RE::SpellItem*> appliedAbilities;

		auto leftEquipped = a_actor->GetEquippedObject(true) ? a_actor->GetEquippedObject(true)->As<RE::TESObjectWEAP>() : nullptr;
		auto leftEnchant = leftEquipped ? leftEquipped->formEnchanting : nullptr;
		float leftEnchantAmount = leftEnchant ? a_actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kLeftItemCharge) : 0.0f;
		bool bLeftEnchanted = leftEnchantAmount > 0.0 ? true : false;

		if (bLeftEnchanted) {
			if (weaponCache->find(leftEquipped) != weaponCache->end()) {
				std::vector<ArtSwap::ArtSwap> foundSwaps;
				foundSwaps = weaponCache->at(leftEquipped);
				
				for (auto swap : foundSwaps) {
					appliedAbilities.push_back(swap.GetAbility());
				}
			}
		}

		auto rightEquipped = a_actor->GetEquippedObject(false) ? a_actor->GetEquippedObject(false)->As<RE::TESObjectWEAP>() : nullptr;
		auto rightEnchant = rightEquipped ? rightEquipped->formEnchanting : nullptr;
		float rightEnchantAmount = rightEnchant ? a_actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kRightItemCharge) : 0.0f;
		bool bRightEnchanted = rightEnchantAmount > 0.0 ? true : false;

		if (bRightEnchanted) {
			if (weaponCache->find(rightEquipped) != weaponCache->end()) {
				std::vector<ArtSwap::ArtSwap> foundSwaps;
				foundSwaps = weaponCache->at(rightEquipped);
				for (auto swap : foundSwaps) {
					appliedAbilities.push_back(swap.GetAbility(false));
				}
			}
		}

		//Clean leftover abilities, add new ones.
		auto* allAbilities = SingletonHolder::ConditionHolder::GetSingleton()->GetAllAbilities();

		for (auto ability : appliedAbilities) {
			if (!a_actor->HasSpell(ability)) a_actor->AddSpell(ability);
		}

		//TODO:: Somewhat optimize this.
		for (auto ability : *allAbilities) {
			bool skip = false;

			for (auto appliedAbility : appliedAbilities) {
				if (ability == appliedAbility) skip = true;
			}
			if (a_actor->HasSpell(ability) && !skip) a_actor->RemoveSpell(ability);
		}
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

		EvaluateAbilities(actor);

		return response;
	}
}
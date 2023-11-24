#include "equipEventHandler.h"
#include "singletonHolder.h"

namespace EquipEventHandler {

	EquipEvent* EquipEvent::GetSingleton() {
		static EquipEvent event;
		return &event;
	}

	void EquipEvent::RegisterListener() {
		RE::ScriptEventSourceHolder* eventHolder = RE::ScriptEventSourceHolder::GetSingleton();
		eventHolder->AddEventSink(EquipEventHandler::EquipEvent::GetSingleton());
	}

	bool EnchantmentHasKeyword(RE::EnchantmentItem* a_enchantment, std::string a_keyword) {

		auto effects = a_enchantment->effects;

		for (auto effect : effects) {
			if (effect->baseEffect->HasKeywordString(a_keyword)) return true;
		}

		return false;
	}

	bool EvaluateEnchant(RE::TESObjectWEAP* a_weapon, RE::EnchantmentItem* a_enchantment, ArtSwap a_swap) {
		bool weaponMatches = true;

		for (auto requiredKeyword : a_swap.weaponKeywords) {
			if (!a_weapon->HasKeywordString(requiredKeyword)) {
				weaponMatches = false;
				break;
			}
		}

		if (!weaponMatches) return false;

		bool enchantmentMatches = true;

		for (auto requiredKeyword : a_swap.enchantmentKeywords) {
			if (!EnchantmentHasKeyword(a_enchantment, requiredKeyword)) {
				enchantmentMatches = false;
				break;
			}
		}

		if (!enchantmentMatches) return false;
		return true;
	}

	void EvaluateAbilities(RE::Actor* a_actor) {
		std::vector<ArtSwap>* allSwaps = SingletonHolder::ConditionHolder::GetSingleton()->GetSwaps();

		auto leftEquipped = a_actor->GetEquippedObject(true) ? a_actor->GetEquippedObject(true)->As<RE::TESObjectWEAP>() : nullptr;
		auto leftEnchant = leftEquipped ? leftEquipped->formEnchanting : nullptr;
		float leftEnchantAmount = leftEnchant ? a_actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kRightItemCharge) : 0.0f;
		bool bLeftEnchanted = leftEnchantAmount > 0.0 ? true : false;

		auto rightEquipped = a_actor->GetEquippedObject(false) ? a_actor->GetEquippedObject(false)->As<RE::TESObjectWEAP>() : nullptr;
		auto rightEnchant = rightEquipped ? rightEquipped->formEnchanting : nullptr;
		float rightEnchantAmount = rightEnchant ? a_actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kRightItemCharge) : 0.0f;
		bool bRightEnchanted = rightEnchantAmount > 0.0 ? true : false;

		for (auto swap : *allSwaps) {
			bool bRemoveLeft = true;
			bool bRemoveRight = true; 

			if (bLeftEnchanted) {
				if (EvaluateEnchant(leftEquipped, leftEnchant, swap)) { 
					a_actor->AddSpell(swap.leftHandArtObject); 
					bRemoveLeft = false;
				}
			}

			if (bRightEnchanted) {
				if (EvaluateEnchant(rightEquipped, rightEnchant, swap)) {
					a_actor->AddSpell(swap.rightHandArtObject);
					bRemoveRight = false;
				}
			}

			if (bRemoveLeft && a_actor->HasSpell(swap.leftHandArtObject)) a_actor->RemoveSpell(swap.leftHandArtObject);
			if (bRemoveRight && a_actor->HasSpell(swap.rightHandArtObject)) a_actor->RemoveSpell(swap.rightHandArtObject);
		}
	}

	RE::BSEventNotifyControl EquipEvent::ProcessEvent(const RE::TESEquipEvent* a_event, RE::BSTEventSource<RE::TESEquipEvent>* a_eventSource) {
		//I am just lazy. This is bad practice. 
		auto response = RE::BSEventNotifyControl::kContinue;

		if (!a_event || !a_eventSource) {
			return response;
		}

		//Only handle equipping
		if (!a_event->equipped) return response;

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
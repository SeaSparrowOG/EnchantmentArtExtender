#include "equipEventHandler.h"

namespace EquipEventHandler {

	void driver(RE::TESObjectWEAP* a_wpn, RE::Actor* a_actor) {
		//Only right hand, only fire.
		RE::BSTArray<RE::Effect*> empty;
		RE::EnchantmentItem* wpnEnchant = a_wpn->formEnchanting ? a_wpn->formEnchanting : nullptr;
		auto magicEffect = wpnEnchant ? wpnEnchant->effects : empty;

		for (auto effect : magicEffect) {
			if (effect->baseEffect->ContainsKeywordString("MagicDamageFire"sv)) {
				RE::FormID fireFXID;
				std::istringstream("0x800") >> std::hex >> fireFXID;
				RE::FormID frostFXID;
				std::istringstream("0x801") >> std::hex >> frostFXID;
				RE::SpellItem* fireFX = RE::TESDataHandler::GetSingleton()->LookupForm(fireFXID, "driver.esp")->As<RE::SpellItem>();
				RE::SpellItem* frostFX = RE::TESDataHandler::GetSingleton()->LookupForm(frostFXID, "driver.esp")->As<RE::SpellItem>();
				a_actor->AddSpell(fireFX);
				a_actor->AddSpell(frostFX);
				return;
			}
		}
	}

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
		if (!baseWeapon) return response;

		if (a_event->equipped) {
			driver(baseWeapon, actor);
		}
		else {

		}

		return response;
	}


	//From here: These are just for readability.
	void UnequipHandler(RE::Actor* a_actor, RE::TESObjectWEAP* a_actorWeapon) {

	}
}
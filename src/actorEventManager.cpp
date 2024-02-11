#include "actorEventManager.h"
#include "cache.h"

namespace {
	bool SpellVectorContainsElement(RE::SpellItem* a_spell, std::vector<RE::SpellItem*>* a_vec) {
		for (auto* spell : *a_vec) {
			if (a_spell == spell) return true;
		}
		return false;
	}

	void EvaluateActor(RE::Actor* a_actor, RE::TESObjectWEAP* a_leftWeapon, RE::TESObjectWEAP* a_rightWeapon, bool a_bDrawn) {
		if (!a_actor) return;

		std::vector<RE::SpellItem*> abilities = std::vector<RE::SpellItem*>();

		auto* magicLight = Cache::StoredData::GetSingleton()->lightObject;
		auto* magicSpellRight = Cache::StoredData::GetSingleton()->enchantmentLightRight;
		auto* magicSpellLeft = Cache::StoredData::GetSingleton()->enchantmentLightLeft;
		auto* magicSpellBoth = Cache::StoredData::GetSingleton()->enchantmentLightBoth;
		float red = 255.0f;
		float green = 255.0f;
		float blue = 255.0f;

		if (a_bDrawn && a_rightWeapon) {
			auto* enchantment = a_rightWeapon->formEnchanting;
			if (!enchantment) enchantment = a_actor->GetEquippedEntryData(false)->GetEnchantment();

			auto swaps = Cache::StoredData::GetSingleton()->GetMatchingSwaps(a_rightWeapon, enchantment);
			for (auto& swap : swaps) {
				auto* spell = swap.rightAbility;
				if (!SpellVectorContainsElement(spell, &abilities)) {
					if (Cache::StoredData::GetSingleton()->GetShouldAddLight()) {
						for (auto* effect : spell->effects) {
							auto* baseEffect = effect->baseEffect;
							bool valid = baseEffect ? baseEffect->GetArchetype() == RE::EffectSetting::Archetype::kLight : false;
							if (!valid) continue;

							auto* effectForm = baseEffect->data.associatedForm;
							auto* associatedLight = effectForm ? effectForm->As<RE::TESObjectLIGH>() : nullptr;
							if (!associatedLight) continue;

							auto color = associatedLight->data.color;
							uint8_t currentRed = color.red;
							uint8_t currentGreen = color.green;
							uint8_t currentBlue = color.blue;

							if (currentRed < red)
								red = currentRed;
							if (currentGreen < green)
								green = currentGreen;
							if (currentBlue < blue)
								blue = currentBlue;
						}
					}
					abilities.push_back(spell);
				}
			}
		}

		if (a_bDrawn && a_leftWeapon) {
			auto* enchantment = a_leftWeapon->formEnchanting;
			if (!enchantment) enchantment = a_actor->GetEquippedEntryData(true)->GetEnchantment();

			auto swaps = Cache::StoredData::GetSingleton()->GetMatchingSwaps(a_leftWeapon, enchantment);
			for (auto& swap : swaps) {
				auto* spell = swap.leftAbility;
				if (!SpellVectorContainsElement(spell, &abilities)) {
					if (Cache::StoredData::GetSingleton()->GetShouldAddLight()) {
						for (auto* effect : spell->effects) {
							auto* baseEffect = effect->baseEffect;
							bool valid = baseEffect ? baseEffect->GetArchetype() == RE::EffectSetting::Archetype::kLight : false;
							if (!valid) continue;

							auto* effectForm = baseEffect->data.associatedForm;
							auto* associatedLight = effectForm ? effectForm->As<RE::TESObjectLIGH>() : nullptr;
							if (!associatedLight) continue;

							auto color = associatedLight->data.color;
							uint8_t currentRed = color.red;
							uint8_t currentGreen = color.green;
							uint8_t currentBlue = color.blue;

							if (currentRed < red)
								red = currentRed;
							if (currentGreen < green)
								green = currentGreen;
							if (currentBlue < blue)
								blue = currentBlue;
						}
					}
					abilities.push_back(spell);
				}
			}
		}

		std::vector<RE::SpellItem*> allAbilities = Cache::StoredData::GetSingleton()->GetAllAbilities();
		for (auto* spell : allAbilities) {
			if (a_actor->HasSpell(spell)) {
				a_actor->RemoveSpell(spell);
			}
		}

		for (auto* spell : abilities) {
			if (!a_actor->HasSpell(spell)) {
				a_actor->AddSpell(spell);
			}
		}

		if (magicSpellRight && a_actor->HasSpell(magicSpellRight))
			a_actor->RemoveSpell(magicSpellRight);

		if (magicSpellLeft && a_actor->HasSpell(magicSpellLeft))
			a_actor->RemoveSpell(magicSpellLeft);

		if (magicSpellBoth && a_actor->HasSpell(magicSpellBoth))
			a_actor->RemoveSpell(magicSpellBoth);

		if (Cache::StoredData::GetSingleton()->GetShouldAddLight() && (red < 255.0f || green < 255.0f || blue < 255.0f)) {
			magicLight->data.color.red = red;
			magicLight->data.color.green = green;
			magicLight->data.color.blue = blue;
			RE::SpellItem* magicSpell = nullptr;

			if (a_leftWeapon && a_rightWeapon && magicSpellBoth) {
				magicSpell = magicSpellBoth;
			}
			else if (a_leftWeapon && magicSpellLeft) {
				magicSpell = magicSpellLeft;
			}
			else if (a_rightWeapon && magicSpellRight) {
				magicSpell = magicSpellRight;
			}
			a_actor->AddSpell(magicSpell);
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

	void AnimationEventListener::RegisterActor(RE::Actor* a_actor) {
		if (!a_actor || a_actor->IsDead()) return;
		if (!this->managedActors.contains(a_actor))
			a_actor->AddAnimationGraphEventSink(this);
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

	void AnimationEventListener::UnRegisterActor(RE::Actor* a_actor) {
		if (!a_actor) return;
		if (!this->managedActors.contains(a_actor)) return;
		a_actor->RemoveAnimationGraphEventSink(this);
		this->managedActors.erase(a_actor);
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

		RE::FormID eventID = a_event->formID;
		auto eventForm = eventID ? RE::TESForm::LookupByID(eventID) : nullptr;
		auto eventActor = eventForm ? eventForm->As<RE::Actor>() : nullptr;
		if (!eventActor) return continueEvent;
		
		ActorRegistry::GetSingleton()->ProcessLoadEvent(eventActor, a_event->loaded);
		return continueEvent;
	}

	RE::BSEventNotifyControl EquipEvent::ProcessEvent(const RE::TESEquipEvent* a_event, RE::BSTEventSource<RE::TESEquipEvent>* a_eventSource) {
		if (!a_event || !a_eventSource) return continueEvent;

		auto reference = a_event->actor;
		auto* actor = a_event ? reference->As<RE::Actor>() : nullptr;
		if (!actor) return continueEvent;

		ActorRegistry::GetSingleton()->ProcessEquipEvent(actor, a_event->equipped);
		return continueEvent;
	}

	RE::BSEventNotifyControl DeathEvent::ProcessEvent(const RE::TESDeathEvent* a_event, RE::BSTEventSource<RE::TESDeathEvent>* a_eventSource) {
		if (!(a_event && a_eventSource)) return continueEvent;
		auto eventActorPtr = a_event->actorDying;
		auto* eventActorRefr = eventActorPtr ? eventActorPtr.get() : nullptr;
		auto* eventActor = eventActorRefr ? eventActorRefr->As<RE::Actor>() : nullptr;
		if (!eventActor) return continueEvent;

		ActorRegistry::GetSingleton()->ProcessDeathEvent(eventActor);
		return continueEvent;
	}

	RE::BSEventNotifyControl AnimationEventListener::ProcessEvent(const RE::BSAnimationGraphEvent* a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource) {
		if (!(a_event && a_eventSource)) return continueEvent;
		auto tag = a_event->tag;
		auto eventFormID = a_event->holder->formID;
		auto* eventForm = RE::TESForm::LookupByID(eventFormID);
		auto* eventActor = eventForm ? eventForm->As<RE::Actor>() : nullptr;
		if (!eventActor) return continueEvent;

		if (eventActor && tag == "weaponDraw") {
			ActorRegistry::GetSingleton()->ProcessAnimationEvent(eventActor, true);
		}
		else if (eventActor && tag == "weaponSheathe") {
			ActorRegistry::GetSingleton()->ProcessAnimationEvent(eventActor, false);
		}

		return continueEvent;
	}
}

namespace ActorEvents {

	void ActorEvents::ActorRegistry::ProcessDeathEvent(RE::Actor* a_actor) {
		if (!this->managedActors.contains(a_actor)) return;
		AnimationEventListener::GetSingleton()->UnRegisterActor(a_actor);
		this->managedActors.erase(a_actor);
	}

	void ActorRegistry::ProcessEquipEvent(RE::Actor* a_actor, bool a_equipped) {
		if (!a_actor || a_actor->IsDead()) return;
		auto* actorData = this->managedActors.contains(a_actor) ? &this->managedActors[a_actor] : nullptr;

		if (actorData) {
			//Managed actor. Make sure that data has changed.
			auto* rightData = a_actor->GetEquippedEntryData(false);
			bool rightEnchanted = rightData ? rightData->IsEnchanted() : false;
			auto* rightBound = rightEnchanted ? rightData->object : nullptr;
			auto* rightWeapon = rightBound ? rightBound->As<RE::TESObjectWEAP>() : nullptr;

			auto* leftData = a_actor->GetEquippedEntryData(true);
			bool leftEnchanted = leftData ? leftData->IsEnchanted() : false;
			auto* leftBound = leftEnchanted ? leftData->object : nullptr;
			auto* leftWeapon = leftBound ? leftBound->As<RE::TESObjectWEAP>() : nullptr;

			//No longer has enchantments.
			if (!(leftWeapon || rightWeapon)) {
				AnimationEventListener::GetSingleton()->UnRegisterActor(a_actor);
				this->managedActors.erase(a_actor);
				return;
			}

			//Unchanged data.
			if (actorData->first.first == leftWeapon && actorData->first.second == rightWeapon) {
				return;
			}

			//Avoiding a crash:
			if (actorData->second && rightWeapon && actorData->first.second != rightWeapon) {
				this->managedActors[a_actor] = std::pair<std::pair<RE::TESObjectWEAP*, RE::TESObjectWEAP*>, bool>
					(std::pair<RE::TESObjectWEAP*, RE::TESObjectWEAP*>(leftWeapon, rightWeapon),
						false);
				return;
			}

			actorData->first.first = leftWeapon;
			actorData->first.second = rightWeapon;
			EvaluateActor(a_actor, leftWeapon, rightWeapon, a_actor->IsWeaponDrawn());
		}
		else {
			//Unmanaged actor unequipped something. Don't care.
			if (!a_equipped) return;

			auto* rightData = a_actor->GetEquippedEntryData(false);
			bool rightEnchanted = rightData ? rightData->IsEnchanted() : false;
			auto* rightBound = rightEnchanted ? rightData->object : nullptr;
			auto* rightWeapon = rightBound ? rightBound->As<RE::TESObjectWEAP>() : nullptr;

			auto* leftData = a_actor->GetEquippedEntryData(true);
			bool leftEnchanted = leftData ? leftData->IsEnchanted() : false;
			auto* leftBound = leftEnchanted ? leftData->object : nullptr;
			auto* leftWeapon = leftBound ? leftBound->As<RE::TESObjectWEAP>() : nullptr;

			//Unmanaged actor equipped non-enchanted weapon. Don't care.
			if (!(leftWeapon || rightWeapon)) return;

			bool weaponOut = a_actor->IsWeaponDrawn();
			this->managedActors[a_actor] = std::pair<std::pair<RE::TESObjectWEAP*, RE::TESObjectWEAP*>, bool>
				(std::pair<RE::TESObjectWEAP*, RE::TESObjectWEAP*>(leftWeapon, rightWeapon),
					weaponOut);
			AnimationEventListener::GetSingleton()->RegisterActor(a_actor);
			EvaluateActor(a_actor, leftWeapon, rightWeapon, weaponOut);
		}
	}

	void ActorEvents::ActorRegistry::ProcessAnimationEvent(RE::Actor* a_actor, bool a_drawing) {
		if (!a_actor || !this->managedActors.contains(a_actor)) return;

		auto* rightData = a_actor->GetEquippedEntryData(false);
		bool rightEnchanted = rightData ? rightData->IsEnchanted() : false;
		auto* rightBound = rightEnchanted ? rightData->object : nullptr;
		auto* rightWeapon = rightBound ? rightBound->As<RE::TESObjectWEAP>() : nullptr;

		auto* leftData = a_actor->GetEquippedEntryData(true);
		bool leftEnchanted = leftData ? leftData->IsEnchanted() : false;
		auto* leftBound = leftEnchanted ? leftData->object : nullptr;
		auto* leftWeapon = leftBound ? leftBound->As<RE::TESObjectWEAP>() : nullptr;

		if (!(leftWeapon || rightWeapon)) {
			AnimationEventListener::GetSingleton()->UnRegisterActor(a_actor);
			this->managedActors.erase(a_actor);
			return;
		}

		AnimationEventListener::GetSingleton()->RegisterActor(a_actor);
		this->managedActors[a_actor] = std::pair<std::pair<RE::TESObjectWEAP*, RE::TESObjectWEAP*>, bool>
			(std::pair<RE::TESObjectWEAP*, RE::TESObjectWEAP*>(leftWeapon, rightWeapon),
				a_drawing);
		EvaluateActor(a_actor, leftWeapon, rightWeapon, a_drawing);
	}

	void ActorEvents::ActorRegistry::ProcessLoadEvent(RE::Actor* a_actor, bool a_loaded) {
		if (!a_loaded && this->managedActors.contains(a_actor)) {
			AnimationEventListener::GetSingleton()->UnRegisterActor(a_actor);
			this->managedActors.erase(a_actor);
			return;
		}

		auto* rightData = a_actor->GetEquippedEntryData(false);
		bool rightEnchanted = rightData ? rightData->IsEnchanted() : false;
		auto* rightBound = rightEnchanted ? rightData->object : nullptr;
		auto* rightWeapon = rightBound ? rightBound->As<RE::TESObjectWEAP>() : nullptr;

		auto* leftData = a_actor->GetEquippedEntryData(true);
		bool leftEnchanted = leftData ? leftData->IsEnchanted() : false;
		auto* leftBound = leftEnchanted ? leftData->object : nullptr;
		auto* leftWeapon = leftBound ? leftBound->As<RE::TESObjectWEAP>() : nullptr;
		
		if (!(leftWeapon || rightWeapon)) {
			//Super unecessary. This should never happen.
			if (this->managedActors.contains(a_actor)) {
				AnimationEventListener::GetSingleton()->UnRegisterActor(a_actor);
				this->managedActors.erase(a_actor);
			}
			return;
		}

		if (!this->managedActors.contains(a_actor)) {
			this->managedActors[a_actor] = this->managedActors[a_actor] = std::pair<std::pair<RE::TESObjectWEAP*, RE::TESObjectWEAP*>, bool>
				(std::pair<RE::TESObjectWEAP*, RE::TESObjectWEAP*>(leftWeapon, rightWeapon),
					a_actor->IsWeaponDrawn());
			AnimationEventListener::GetSingleton()->RegisterActor(a_actor);
		}
		EvaluateActor(a_actor, leftWeapon, rightWeapon, a_actor->IsWeaponDrawn());
	}
}
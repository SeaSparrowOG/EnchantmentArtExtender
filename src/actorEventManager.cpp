#include "actorEventManager.h"
#include "cache.h"

namespace {
	bool SpellVectorContainsElement(RE::SpellItem* a_spell, std::vector<RE::SpellItem*>* a_vec) {
		for (const auto* spell : *a_vec) {
			if (a_spell == spell) return true;
		}
		return false;
	}

	void EvaluateActor(RE::Actor* a_actor, bool drawn) {
		if (!a_actor) return;
		//New guard for paused menus
		if (RE::UI::GetSingleton()->GameIsPaused()) return;

		std::vector<RE::SpellItem*> abilities = std::vector<RE::SpellItem*>();

		auto* magicLight = Cache::StoredData::GetSingleton()->lightObject;
		auto* magicSpellRight = Cache::StoredData::GetSingleton()->enchantmentLightRight;
		auto* magicSpellLeft = Cache::StoredData::GetSingleton()->enchantmentLightLeft;
		auto* magicSpellBoth = Cache::StoredData::GetSingleton()->enchantmentLightBoth;
		float red = 255.0f;
		float green = 255.0f;
		float blue = 255.0f;

		auto* right = a_actor->GetEquippedEntryData(false);
		auto* left = a_actor->GetEquippedEntryData(true);
#undef GetObject
		auto* a_rightWeapon = right ? right->GetObject()->As<RE::TESObjectWEAP>() : nullptr;
		auto* a_leftWeapon = left ? left->GetObject()->As<RE::TESObjectWEAP>() : nullptr;
		if (drawn && a_rightWeapon && !a_rightWeapon->IsBound()) {
			auto* enchantment = a_rightWeapon->formEnchanting;
			if (!enchantment) enchantment = a_actor->GetEquippedEntryData(false)->GetEnchantment();

			const auto swaps = Cache::StoredData::GetSingleton()->GetMatchingSwaps(a_rightWeapon, enchantment);
			for (const auto& swap : swaps) {
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

		if (drawn && a_leftWeapon && !a_leftWeapon->IsBound()) {
			auto* enchantment = a_leftWeapon->formEnchanting;
			if (!enchantment) enchantment = a_actor->GetEquippedEntryData(true)->GetEnchantment();

			const auto swaps = Cache::StoredData::GetSingleton()->GetMatchingSwaps(a_leftWeapon, enchantment);
			for (const auto& swap : swaps) {
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
	void Install() {
		DrawWeaponMagicHands::Install();
	}

	void DrawWeaponMagicHands::thunk(RE::Actor* a_this, bool a_draw)
	{
		if (a_this) {
			EvaluateActor(a_this, a_draw);
		}
		func(a_this, a_draw);
	}
}
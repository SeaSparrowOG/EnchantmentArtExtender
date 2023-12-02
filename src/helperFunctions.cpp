#include "helperFunctions.h"
#include "artSwap.h"
#include "singletonHolder.h"

namespace helperFunction {
	//Returns a FormID from a given string. Credits:
	//Colinswrath
	//Github: https://github.com/colinswrath
	//Nexus: https://www.nexusmods.com/skyrimspecialedition/users/6850662
	RE::FormID GetFormIDFromString(std::string a_string) {
		RE::FormID result;
		std::istringstream ss{ a_string };
		ss >> std::hex >> result;
		return result;
	}

	//Returns true if a mod is installed.
	bool IsModPresent(std::string a_fileName) {
		std::string_view modName = std::string_view(a_fileName);

		RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();
		const RE::TESFile* mod;
		mod = dataHandler->LookupLoadedModByName(modName);

		if (mod) {
			return true;
		}

		mod = dataHandler->LookupLoadedLightModByName(modName);

		if (mod) {
			return true;
		}

		return false;

	}

	RE::TESObjectWEAP* GetWeaponFromID(RE::FormID a_FormID, std::string a_ModName) {
		RE::TESObjectWEAP* response;
		RE::TESForm* form = RE::TESDataHandler::GetSingleton()->LookupForm(a_FormID, a_ModName);
		response = form ? form->As<RE::TESObjectWEAP>() : NULL;
		return response;
	}

	RE::SpellItem* GetSpellFromFormID(RE::FormID a_FormID, std::string a_ModName) {
		RE::SpellItem* response;
		RE::TESForm* form = RE::TESDataHandler::GetSingleton()->LookupForm(a_FormID, a_ModName);
		response = form ? form->As<RE::SpellItem>() : NULL;
		return response;
	}


	bool VectorContainsString(std::vector<std::string>* a_vector, std::string a_element) {
		for (std::string vectorElement : *a_vector) {
			if (vectorElement == a_element) {
				return true;
			}
		}
		return false;
	}

	bool VectorContainsWeapon(std::vector<RE::TESObjectWEAP*>* a_vector, RE::TESObjectWEAP* a_element) {
		for (RE::TESObjectWEAP* vectorElement : *a_vector) {
			if (vectorElement == a_element) {
				return true;
			}
		}
		return false;
	}

	bool VectorContainsSpell(std::vector<RE::SpellItem*>* a_vector, RE::SpellItem* a_element) {
		for (RE::SpellItem* vectorElement : *a_vector) {
			if (vectorElement == a_element) {
				return true;
			}
		}
		return false;
	}

	void AppendMatchingSwapsToVector(std::vector<RE::SpellItem*>* a_vec, RE::TESObjectWEAP* a_weap, bool a_left, RE::ExtraEnchantment* a_extraEnchant = nullptr) {
		auto conditionSingleton = SingletonHolder::ConditionHolder::GetSingleton();

		if (!a_extraEnchant) {
			auto weaponCache = conditionSingleton->GetWeaponCache();
			if (weaponCache->find(a_weap) != weaponCache->end()) {
				std::vector<ArtSwap::ArtSwap> foundSwaps;
				foundSwaps = weaponCache->at(a_weap);

				for (auto& swap : foundSwaps) {
					a_vec->push_back(swap.GetAbility(a_left));
				}
			}
		}
		else {
			RE::EnchantmentItem* weaponEnchant = a_extraEnchant->enchantment;
			if (!weaponEnchant) return;

			auto foundSwap = conditionSingleton->GetBestMatchingSwap(*conditionSingleton->GetSwaps(ArtSwapMode::Exclusive), a_weap, weaponEnchant);

			if (!foundSwap.GetName().empty()) {
				a_vec->push_back(foundSwap.GetAbility(a_left));
			}

			if (!a_vec->empty()) return;

			for (auto& additiveSwap : *conditionSingleton->GetSwaps(ArtSwapMode::Additive)) {
				if (additiveSwap.IsMatch(a_weap, weaponEnchant)) {
					a_vec->push_back(foundSwap.GetAbility(a_left));
				}
			}

			if (!a_vec->empty()) return;

			foundSwap = conditionSingleton->GetBestMatchingSwap(*conditionSingleton->GetSwaps(ArtSwapMode::LowPriority), a_weap);

			if (!foundSwap.GetName().empty()) {
				a_vec->push_back(foundSwap.GetAbility(a_left));
			}
		}
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

		if (bLeftEnchanted) AppendMatchingSwapsToVector(&appliedAbilities, leftEquipped, true);

		auto rightEquipped = a_actor->GetEquippedObject(false) ? a_actor->GetEquippedObject(false)->As<RE::TESObjectWEAP>() : nullptr;
		auto rightEnchant = rightEquipped ? rightEquipped->formEnchanting : nullptr;
		float rightEnchantAmount = rightEnchant ? a_actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kRightItemCharge) : 0.0f;
		bool bRightEnchanted = rightEnchantAmount > 0.0 ? true : false;

		if (bRightEnchanted) AppendMatchingSwapsToVector(&appliedAbilities, rightEquipped, false);

		//Check for player-made enchantments
		const auto process = a_actor->GetActorRuntimeData().currentProcess;
		const auto middleHigh = process ? process->middleHigh : nullptr;
		const auto rightHand = middleHigh ? middleHigh->rightHand : nullptr;
		const auto leftHand = middleHigh ? middleHigh->leftHand : nullptr;

		if (leftEquipped && leftHand && leftHand->object) {
			if (const auto& extraLists = leftHand->extraLists) {
				for (const auto& extraList : *extraLists) {
					const auto exEnch = extraList->GetByType<RE::ExtraEnchantment>();
					if (exEnch && exEnch->enchantment) AppendMatchingSwapsToVector(&appliedAbilities, leftEquipped, true, exEnch);
				}
			}
		}

		if (rightEquipped && rightHand && rightHand->object) {
			if (const auto& extraLists = rightHand->extraLists) {
				for (const auto& extraList : *extraLists) {
					const auto exEnch = extraList->GetByType<RE::ExtraEnchantment>();
					if (exEnch && exEnch->enchantment) AppendMatchingSwapsToVector(&appliedAbilities, rightEquipped, false, exEnch);
				}
			}
		}

		//Clean leftover abilities, add new ones.
		auto* allAbilities = SingletonHolder::ConditionHolder::GetSingleton()->GetAllAbilities();

		for (auto ability : appliedAbilities) {
			if (!a_actor->HasSpell(ability)) a_actor->AddSpell(ability);
			else {
				a_actor->AddSpell(ability);
			}
		}

		//TODO:: Somewhat optimize this.
		for (auto ability : *allAbilities) {
			bool skip = false;

			for (auto appliedAbility : appliedAbilities) {
				if (ability == appliedAbility) {
					skip = true;
				}
			}
			if (a_actor->HasSpell(ability) && !skip) {
				a_actor->RemoveSpell(ability);
			}
		}
	}
}
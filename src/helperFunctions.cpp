#include "helperFunctions.h"

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
}
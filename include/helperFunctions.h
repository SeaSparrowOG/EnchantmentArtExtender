#pragma once

namespace helperFunction {
	//Returns a FormID from a given string. Credits:
	//Colinswrath
	//Github: https://github.com/colinswrath
	//Nexus: https://www.nexusmods.com/skyrimspecialedition/users/6850662
	RE::FormID GetFormIDFromString(std::string a_string);

	bool IsModPresent(std::string a_fileName);
	RE::TESObjectWEAP* GetWeaponFromID(RE::FormID a_FormID, std::string a_ModName);
	RE::SpellItem* GetSpellFromFormID(RE::FormID a_FormID, std::string a_ModName);

	bool VectorContainsString(std::vector<std::string>* a_vector, std::string a_element);
	bool VectorContainsWeapon(std::vector<RE::TESObjectWEAP*>* a_vector, RE::TESObjectWEAP* a_element);
}
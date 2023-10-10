#pragma once

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

#include <ClibUtil/utils.hpp>
#include <fstream>
#include <stdexcept>
#include <json/json.h>

using namespace std::literals;

#define  _logger SKSE::log

//Some code readability. Each Art Swap is a struct, and they are kept in a vector (in singletonHolder).
struct ArtSwap {
	std::vector<RE::BGSKeyword*> weaponKeywords;
	std::vector<RE::BGSKeyword*> enchantmentKeywords;
	RE::BGSArtObject* leftHandArtObject;
	RE::BGSArtObject* rightHandArtObject;

	//The constructor is EXPLICIT. If any part is missing, it should not work, anyways.
	ArtSwap(std::vector<RE::BGSKeyword*> a_weaponKeywords, std::vector<RE::BGSKeyword*> a_enchantmentKeywords,
		RE::BGSArtObject* a_leftHandArtObject, RE::BGSArtObject* a_rightHandArtObject) {

		this->weaponKeywords = a_weaponKeywords;
		this->enchantmentKeywords = a_enchantmentKeywords;
		this->leftHandArtObject = a_leftHandArtObject;
		this->rightHandArtObject = a_rightHandArtObject;
	}

	void to_String() {
		_logger::info("===================================");
		_logger::info("           Art Swap");
		_logger::info("===================================");
		_logger::info("");
		
		_logger::info("");
		_logger::info("===================================");
		_logger::info("        End Of Art Swap");
		_logger::info("===================================");
	}
};

//Used to bind Papyrus Functions.
#define BIND(a_method, ...) a_vm->RegisterFunction(#a_method##sv, script, a_method __VA_OPT__(, ) __VA_ARGS__)
using VM = RE::BSScript::Internal::VirtualMachine;
using StackID = RE::VMStackID;
inline constexpr auto script = "SEA_EnchantmentExtender"sv;

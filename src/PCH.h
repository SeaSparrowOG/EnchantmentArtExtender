#pragma once

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

#include <ClibUtil/utils.hpp>
#include <fstream>
#include <stdexcept>
#include <json/json.h>

using namespace std::literals;

#define  _logger SKSE::log

//Slight readability tweak.
enum class ArtSwapMode {
	Additive = 0,    //Layered on top of other additive swaps.
	Exclusive = 1,   //Is applied and does not allow other swaps to be placed over it.
	LowPriority = 2  //Is applied only if no other swaps would be applied.
};

enum class ActorCacheAction {
	AddToCache = 0,
	RemoveAbilityLeft = 1,
	RemoveAbilityRight = 2,
	AddAbilityLeft = 3,
	AddAbilityRight = 4,
	ClearFromCache = 5
};

//Used to bind Papyrus Functions.
#define BIND(a_method, ...) a_vm->RegisterFunction(#a_method##sv, script, a_method __VA_OPT__(, ) __VA_ARGS__)
using VM = RE::BSScript::Internal::VirtualMachine;
using StackID = RE::VMStackID;
inline constexpr auto script = "SEA_EnchantmentExtender"sv;

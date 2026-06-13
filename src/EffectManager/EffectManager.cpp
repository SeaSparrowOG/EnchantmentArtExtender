#include "EffectManager.h"

#include "Common/JSONUtils.hpp"
#include "Settings/INI/INISettings.h"
#include "Settings/JSON/JSONSettings.h"

namespace EffectManager
{
	void EffectDistributor::TryApplyEffect(RE::WeaponEnchantmentController* controller, RE::EnchantmentItem* enchant) const
	{
		assert(controller);
		static bool onlyPlayer = Settings::INI::GetSetting<bool>
			(Settings::INI::GENERAL_ONLY_PLAYER.data())
			.value_or(false);

		const auto* magicCaster = controller->caster;
		const auto* actor = magicCaster ? magicCaster->GetCasterAsActor() : nullptr;
		if (!actor) {
			LOG_DEBUG("  - No actor."sv);
			return;
		}
		if (onlyPlayer && !actor->IsPlayerRef()) {
			LOG_DEBUG(" - Violated only player principle"sv);
			return;
		}

		const bool left = magicCaster->castingSource == RE::MagicSystem::CastingSource::kLeftHand;
		const auto* entry = actor->GetEquippedEntryData(left);
		const auto* weap = entry && entry->object ? entry->object->As<RE::TESObjectWEAP>() : nullptr;
		if (!weap) {
			LOG_DEBUG("  - No Weap."sv);
			return;
		}
		
		int lastStrictness = 0;
		PriorityLevel lastPrioLevel = PriorityLevel::None;
		RE::BGSArtObject* found = nullptr;
		for (const auto& swap : _swaps) {
			const int currStrictness = swap.GetStrictnessLevel();
			const PriorityLevel currPrioLevel = swap.GetPriorityLevel();
			if (currPrioLevel < lastPrioLevel) {
				break;
			}
			if (currPrioLevel != PriorityLevel::SpecificEnchantToSpecificWeap &&
				currStrictness < lastStrictness)
			{
				break;
			}

			if (!swap.CanApply(weap, enchant)) {
				continue;
			}
			lastStrictness = currStrictness;
			lastPrioLevel = currPrioLevel;
			found = swap.GetArtObject();
		}

		if (found) {
			controller->art = found;
			LOG_DEBUG("Worked nicely!"sv);
		}
	}

	bool EffectDistributor::AppendArtSwap(ArtSwapProxy& proxy) {
		ArtSwap swap;
		swap.SetArtObject(proxy._artObject);
		for (auto* kwd : proxy._forbiddenKeywords) {
			swap.AddForbiddenKeyword(kwd);
		}
		for (auto* kwd : proxy._requiredWeaponKeywords) {
			swap.AddWeapKeywordCondition(kwd);
		}
		for (auto* kwd : proxy._requiredEnchantmentKeywords) {
			swap.AddEnchKeywordCondition(kwd);
		}
		for (auto* weap : proxy._allowedWeapons) {
			swap.AddWeaponCondition(weap);
		}
		for (auto* ench : proxy._allowedEnchantments) {
			swap.AddEnchantmentCondition(ench);
		}
		if (!swap.Finalize()) {
			return false;
		}

		auto fn = [prio = swap.GetPriorityLevel(), strictness = swap.GetStrictnessLevel()](const ArtSwap& rhs) { 
			return rhs.GetPriorityLevel() < prio && rhs.GetStrictnessLevel() < strictness; 
			};
		auto where = std::ranges::find_if(_swaps, fn);
		if (where == _swaps.end()) {
			_swaps.emplace_back(std::move(swap));
			return true;
		}
		_swaps.emplace(where, std::move(swap));
		return true;
	}

	bool EffectDistributor::ArtSwap::Finalize() {
		if (!_artObject) {
			return false;
		}

		if (!_requiredWeapons.empty() && !_requiredEnchantments.empty()) {
			_level = PriorityLevel::SpecificEnchantToSpecificWeap; // no need for strictness.
		}
		else if (!_requiredEnchantments.empty() && !_requiredWeaponKeywords.empty()) {
			_level = PriorityLevel::SpecificEnchantToWeapOfType;
			size_t size = _requiredWeaponKeywords.size();
			_strictness = size > std::numeric_limits<int>::max() ? std::numeric_limits<int>::max() : static_cast<int>(size);
		}
		else if (!_requiredWeapons.empty() && !_requiredEnchantKeywords.empty()) {
			_level = PriorityLevel::EnchantOfTypeToSpecificWeap;
			size_t size = _requiredEnchantKeywords.size();
			_strictness = size > std::numeric_limits<int>::max() ? std::numeric_limits<int>::max() : static_cast<int>(size);
		}
		else if (!_requiredWeaponKeywords.empty() && !_requiredEnchantKeywords.empty()) {
			_level = PriorityLevel::EnchantOfTypeToWeapOfType;
			size_t size = _requiredWeaponKeywords.size() + _requiredEnchantKeywords.size();
			_strictness = size > std::numeric_limits<int>::max() ? std::numeric_limits<int>::max() : static_cast<int>(size);
		}

		return _level != PriorityLevel::None;
	}

	bool EffectDistributor::ArtSwap::CanApply(const RE::TESObjectWEAP* weap, 
		const RE::EnchantmentItem* ench) const
	{
		auto weapHasKeyword = [weap](const RE::FormID id) { return weap->HasKeywordID(id); };
		auto enchHasKeyword = [ench](const RE::FormID id) { 
			if (ench->HasKeywordID(id)) {
				return true;
			}
			auto& effects = ench->effects;
			if (effects.empty()) {
				return false;
			}
			return std::ranges::any_of(effects, [id](const RE::Effect* effect) {
				const auto* base = effect ? effect->baseEffect : nullptr;
				return base && base->HasKeywordID(id);
				});
			};

		const bool containsForbiddenKeyword =
			std::ranges::any_of(_forbiddenKeywords, weapHasKeyword) ||
			std::ranges::any_of(_forbiddenKeywords, enchHasKeyword);
		if (containsForbiddenKeyword) {
			//return false;
		}

		switch (_level) {
		case PriorityLevel::EnchantOfTypeToWeapOfType:
			return std::ranges::all_of(_requiredWeaponKeywords, weapHasKeyword) &&
				std::ranges::all_of(_requiredEnchantKeywords, enchHasKeyword);

		case PriorityLevel::EnchantOfTypeToSpecificWeap:
			return _requiredWeapons.contains(weap->GetFormID()) &&
				std::ranges::all_of(_requiredEnchantKeywords, enchHasKeyword);

		case PriorityLevel::SpecificEnchantToWeapOfType:
			return _requiredEnchantments.contains(ench->GetFormID()) &&
				std::ranges::all_of(_requiredWeaponKeywords, weapHasKeyword);

		case PriorityLevel::SpecificEnchantToSpecificWeap:
			return _requiredWeapons.contains(weap->GetFormID()) && 
				_requiredEnchantments.contains(ench->GetFormID());

		default:
			return false;
		}
	}

	void EffectDistributor::ArtSwap::SetArtObject(RE::BGSArtObject* obj) { _artObject = obj; }
	void EffectDistributor::ArtSwap::AddWeaponCondition(RE::TESObjectWEAP* weap) { _requiredWeapons.insert(weap->GetFormID()); }
	void EffectDistributor::ArtSwap::AddEnchantmentCondition(RE::EnchantmentItem* enchant) { _requiredEnchantments.insert(enchant->GetFormID()); }
	EffectDistributor::PriorityLevel EffectDistributor::ArtSwap::GetPriorityLevel() const { return _level; }
	RE::BGSArtObject* EffectDistributor::ArtSwap::GetArtObject() const { return _artObject; }
	int EffectDistributor::ArtSwap::GetStrictnessLevel() const { return _strictness; }

	void EffectDistributor::ArtSwap::AddEnchKeywordCondition(RE::BGSKeyword* keyword) {
		auto keywordID = keyword->GetFormID();
		if (std::ranges::contains(_requiredEnchantKeywords, keywordID)) {
			return;
		}
		_requiredEnchantKeywords.emplace_back(keywordID);
	}

	void EffectDistributor::ArtSwap::AddWeapKeywordCondition(RE::BGSKeyword* keyword) {
		auto keywordID = keyword->GetFormID();
		if (std::ranges::contains(_requiredWeaponKeywords, keywordID)) {
			return;
		}
		_requiredWeaponKeywords.emplace_back(keywordID);
	}

	void EffectDistributor::ArtSwap::AddForbiddenKeyword(RE::BGSKeyword* keyword) {
		auto keywordID = keyword->GetFormID();
		if (std::ranges::contains(_forbiddenKeywords, keywordID)) {
			return;
		}
		_forbiddenKeywords.emplace_back(keywordID);
	}

	bool ReadConfigs() {
		logger::info("Parsing stored configs..."sv);
		const auto* jsonHolder = Settings::JSON::Holder::GetSingleton();
		if (!jsonHolder) {
			logger::critical("  - Failed to get internal JSON Config Holder."sv);
			return false;
		}

		auto* distributor = EffectDistributor::GetSingleton();
		if (!distributor) {
			logger::critical("  - Failed to get internal Effect Distributor."sv);
			return false;
		}

		const auto& configs = jsonHolder->GetConfigs();
		static std::vector<std::string> knownFields = {
			"weaponart",
			"enchantmentkeywords",
			"weapons",
			"weaponkeywords",
			"enchantments"
		};

		for (const auto& [name, config] : configs) {
			if (!config.isObject()) {
				LOG_DEBUG("{} top level is not an object."sv, name);
				continue;
			}
			const auto& rules = config["rules"];
			if (!rules || !rules.isArray()) {
				LOG_DEBUG("{} top level is not rules."sv, name);
				continue;
			}
			for (const auto& rule : rules) {
				if (!rule.isObject()) {
					LOG_DEBUG("{} - Non-Object."sv, name);
					continue;
				}

				const auto& displayObjRaw = rule["weaponart"];
				if (!displayObjRaw) {
					LOG_DEBUG("{} - missing Weapon Art."sv, name);
					continue;
				}
				else if (!displayObjRaw.isString()) {
					LOG_DEBUG("{} - Weapon Art is not a string."sv, name);
					continue;
				}

				ArtSwapProxy proxy;
				auto queryResponse = JSONUtils::GetFormFromString<RE::BGSArtObject>(displayObjRaw.asString());
				if (queryResponse.status == JSONUtils::QueryResult::Success) {
					proxy._artObject = queryResponse.value.value();
				}
				else {
					LOG_DEBUG("{} - Failed to resolve {}"sv, name, displayObjRaw.asString());
					continue;
				}

				auto members = rule.getMemberNames();
				for (auto& member : members) {
					bool negate = member.starts_with("!");
					if (negate) {
						member = member.substr(1, member.size());
					}

					if (std::ranges::none_of(knownFields, [member = member](const std::string& str) { return str == member; })) {
						LOG_DEBUG("{} - Unknown field {}"sv, name, member);
						continue;
					}

					const auto& field = rule[member];
					if (negate) {
						auto val = JSONUtils::GetFormsFromValue<RE::BGSKeyword>(field);
						if (!val._data.has_value()) {
							LOG_DEBUG("{}"sv, JSONUtils::ValueResultToString(val._status));
							continue;
						}
						auto result = val._data.value();
						for (auto* kwd : result) {
							if (kwd) {
								proxy._forbiddenKeywords.emplace_back(kwd);
							}
						}
					}
					else if (member == "enchantmentkeywords") {
						auto val = JSONUtils::GetFormsFromValue<RE::BGSKeyword>(field);
						if (!val._data.has_value()) {
							LOG_DEBUG("{}"sv, JSONUtils::ValueResultToString(val._status));
							continue;
						}
						auto result = val._data.value();
						for (auto* kwd : result) {
							if (kwd) {
								proxy._requiredEnchantmentKeywords.emplace_back(kwd);
							}
						}
					}
					else if (member == "weaponkeywords") {
						auto val = JSONUtils::GetFormsFromValue<RE::BGSKeyword>(field);
						if (!val._data.has_value()) {
							LOG_DEBUG("{}"sv, JSONUtils::ValueResultToString(val._status));
							continue;
						}
						auto result = val._data.value();
						for (auto* kwd : result) {
							if (kwd) {
								proxy._requiredWeaponKeywords.emplace_back(kwd);
							}
						}
					}
					else if (member == "weapons") {
						auto val = JSONUtils::GetFormsFromValue<RE::TESObjectWEAP>(field);
						if (!val._data.has_value()) {
							LOG_DEBUG("{}"sv, JSONUtils::ValueResultToString(val._status));
							continue;
						}
						auto result = val._data.value();
						for (auto* weap : result) {
							proxy._allowedWeapons.emplace_back(weap);
						}
					}
					else if (member == "enchantments") {
						auto val = JSONUtils::GetFormsFromValue<RE::EnchantmentItem>(field);
						if (!val._data.has_value()) {
							LOG_DEBUG("{}"sv, JSONUtils::ValueResultToString(val._status));
							continue;
						}
						auto result = val._data.value();
						for (auto* ench : result) {
							proxy._allowedEnchantments.emplace_back(ench);
						}
					}
				}

				if (!distributor->AppendArtSwap(proxy)) {
					LOG_DEBUG("Failure"sv);
				}
			}
		}
		return true;
	}
}
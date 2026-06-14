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

		if (controller->art) {
			return;
		}
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
		for (auto* weap : proxy._forbiddenWeapons) {
			swap.AddForbiddenWeapon(weap);
		}
		for (auto* ench : proxy._forbiddenEnchantments) {
			swap.AddForbiddenEnchantment(ench);
		}
		for (auto* kwd : proxy._forbiddenEnchantmentKeywords) {
			swap.AddForbiddenEnchantmentKeyword(kwd);
		}
		for (auto* kwd : proxy._forbiddenWeaponKeywords) {
			swap.AddForbiddenWeaponKeyword(kwd);
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

		bool isWeapForbidden = _forbiddenWeapons.contains(weap->GetFormID());
		const auto* templateWeap = weap->templateWeapon;
		while (!isWeapForbidden && templateWeap) {
			isWeapForbidden = _forbiddenWeapons.contains(templateWeap->GetFormID());
			templateWeap = templateWeap->templateWeapon;
		}
		isWeapForbidden = isWeapForbidden || std::ranges::any_of(_forbiddenWeaponKeywords, weapHasKeyword);
		if (isWeapForbidden) {
			return false;
		}

		const bool isEnchantmentForbidden = _forbiddenEnchantments.contains(ench->GetFormID()) ||
			std::ranges::any_of(_forbiddenEnchantmentKeywords, enchHasKeyword);
		if (isEnchantmentForbidden) {
			return false;
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
	void EffectDistributor::ArtSwap::AddForbiddenWeapon(RE::TESObjectWEAP* weap) { _forbiddenWeapons.insert(weap->GetFormID()); }
	void EffectDistributor::ArtSwap::AddForbiddenEnchantment(RE::EnchantmentItem* ench) { _forbiddenEnchantments.insert(ench->GetFormID()); }

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

	void EffectDistributor::ArtSwap::AddForbiddenEnchantmentKeyword(RE::BGSKeyword* kwd) { 
		const auto id = kwd->GetFormID();
		if (std::ranges::contains(_forbiddenEnchantmentKeywords, id)) {
			return;
		}
		_forbiddenEnchantmentKeywords.emplace_back(id);
	}

	void EffectDistributor::ArtSwap::AddForbiddenWeaponKeyword(RE::BGSKeyword* kwd) {
		const auto id = kwd->GetFormID();
		if (std::ranges::contains(_forbiddenWeaponKeywords, id)) {
			return;
		}
		_forbiddenWeaponKeywords.emplace_back(id);
	}

	static constexpr std::string_view ART = "weaponart"sv;
	static constexpr std::string_view ENCH_KEYWORDS = "enchantmentkeywords"sv;
	static constexpr std::string_view WEAP_KEYWORDS = "weaponkeywords"sv;
	static constexpr std::string_view WEAP = "weapons"sv;
	static constexpr std::string_view ENCH = "enchantments"sv;
	static constexpr std::string_view RULES = "rules"sv;

	static constexpr size_t SIZE = 5u;
	static constexpr std::array<std::string_view, SIZE> KNOWN_FIELDS = {
		ART,
		ENCH_KEYWORDS,
		WEAP_KEYWORDS,
		WEAP,
		ENCH
	};

	static bool IsValueResultFatal(JSONUtils::ValueStatus val) {
		switch (val) {
		case JSONUtils::ValueStatus::Empty:
		case JSONUtils::ValueStatus::FormatError:
		case JSONUtils::ValueStatus::InvalidForm:
		case JSONUtils::ValueStatus::GenericFailure:
			return true;
		default:
			return false;
		}
	}

	[[nodiscard]] static bool ParseRule(const Json::Value& rule) {
		assert(rule.isObject());
		static auto* distributor = EffectDistributor::GetSingleton();
		if (!distributor) {
			logger::critical("    > Failed to get internal distributor."sv);
			return false;
		}

		bool result = true;
		bool skipRegistration = false;
		auto members = rule.getMemberNames();
		ArtSwapProxy proxy;
		for (auto& member : members) {
			const auto& val = rule[member];
			bool negate = member.starts_with("!");
			if (negate) {
				member = member.substr(1, member.size());
			}

			const bool known = std::ranges::contains(KNOWN_FIELDS, member);
			if (!known) {
				logger::warn("    > Unknown field [{}{}]"sv, negate ? "!" : "", member);
				result = false;
				continue;
			}

			if (member == ART) {
				if (negate) {
					logger::warn("    > Field {} was prefixed with \"!\", which is not allowed."sv, member);
					result = false;
					continue;
				}
				if (!val.isString()) {
					logger::warn("    > Field {} is not a string."sv, member);
					result = false;
					continue;
				}
				auto queryResult = JSONUtils::GetFormFromString<RE::BGSArtObject>(val.asString());
				if (queryResult.value.has_value()) {
					proxy._artObject = queryResult.value.value();
				}
				else {
					logger::warn("    > Failed to resolve {} with error: {}"sv, member, 
						JSONUtils::QueryResultToString(queryResult.status));
					switch (queryResult.status) {
					case JSONUtils::QueryResult::FormatError:
					case JSONUtils::QueryResult::GenericFailure:
					case JSONUtils::QueryResult::MissingPo3Tweaks:
					case JSONUtils::QueryResult::WrongFormtype:
						result = false;
						break;
					}
					skipRegistration = true;
					continue;
				}
			}
			else if (member == ENCH_KEYWORDS) {
				auto queryResult = JSONUtils::GetFormsFromValue<RE::BGSKeyword>(val);
				if (!queryResult._data.has_value()) {
					logger::warn("    > {} Failed with: {}"sv, member, JSONUtils::ValueResultToString(queryResult._status));
					result &= IsValueResultFatal(queryResult._status);
					skipRegistration = true;
				}
				const auto& data = queryResult._data.value();
				for (auto* kwd : data) {
					if (negate) {
						proxy._forbiddenEnchantmentKeywords.emplace_back(kwd);
					}
					else {
						proxy._requiredEnchantmentKeywords.emplace_back(kwd);
					}
				}
			}
			else if (member == WEAP_KEYWORDS) {
				auto queryResult = JSONUtils::GetFormsFromValue<RE::BGSKeyword>(val);
				if (!queryResult._data.has_value()) {
					logger::warn("    > {} Failed with: {}"sv, member, JSONUtils::ValueResultToString(queryResult._status));
					result &= IsValueResultFatal(queryResult._status);
					skipRegistration = true;
				}
				const auto& data = queryResult._data.value();
				for (auto* kwd : data) {
					if (negate) {
						proxy._forbiddenWeaponKeywords.emplace_back(kwd);
					}
					else {
						proxy._requiredWeaponKeywords.emplace_back(kwd);
					}
				}
			}
			else if (member == WEAP) {
				auto queryResult = JSONUtils::GetFormsFromValue<RE::TESObjectWEAP>(val);
				if (!queryResult._data.has_value()) {
					logger::warn("    > {} Failed with: {}"sv, member, JSONUtils::ValueResultToString(queryResult._status));
					result &= IsValueResultFatal(queryResult._status);
					skipRegistration = true;
				}
				const auto& data = queryResult._data.value();
				for (auto* weap : data) {
					if (negate) {
						proxy._forbiddenWeapons.emplace_back(weap);
					}
					else {
						proxy._allowedWeapons.emplace_back(weap);
					}
				}
			}
			else if (member == ENCH) {
				auto queryResult = JSONUtils::GetFormsFromValue<RE::EnchantmentItem>(val);
				if (!queryResult._data.has_value()) {
					logger::warn("    > {} Failed with: {}"sv, member, JSONUtils::ValueResultToString(queryResult._status));
					result &= IsValueResultFatal(queryResult._status);
					skipRegistration = true;
				}
				const auto& data = queryResult._data.value();
				for (auto* weap : data) {
					if (negate) {
						proxy._forbiddenEnchantments.emplace_back(weap);
					}
					else {
						proxy._allowedEnchantments.emplace_back(weap);
					}
				}
			}
		}
		if (!result) {
			return false;
		}
		if (skipRegistration) {
			return true;
		}
		return distributor->AppendArtSwap(proxy);
	}

	[[nodiscard]] static bool ParseSwaps(const Json::Value& swaps) {
		assert(swaps.isObject());
		const auto& rulesValue = swaps[RULES.data()];
		if (rulesValue) {
			if (!rulesValue.isArray()) {
				logger::critical("    > Detect {} field which is not an array."sv, RULES);
				return false;
			}
			bool arrayResolved = true;
			for (const auto& arrayVal : rulesValue) {
				arrayResolved &= ParseRule(arrayVal);
			}
			return arrayResolved;
		}
		return ParseRule(swaps);
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

		bool success = true;
		const auto& configs = jsonHolder->GetConfigs();

		for (const auto& [name, config] : configs) {
			if (config.isObject()) {
				logger::info("  - Reading Config: {}"sv, name);
				success &= ParseSwaps(config);
			}
			else if (config.isArray()) {
				for (Json::ArrayIndex i = 0; i < config.size(); ++i) {
					logger::info("  - Reading Config: {} (Index {})"sv, name, i);
					const auto& arrayElement = config[i];
					if (!arrayElement.isObject()) {
						logger::warn("    >Not an object - skipped."sv);
						continue;
					}
					success &= ParseSwaps(arrayElement);
				}
			}
		}
		return success;
	}
}
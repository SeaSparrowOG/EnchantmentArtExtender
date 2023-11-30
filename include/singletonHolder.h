#pragma once
#include "artSwap.h"
#include "helperFunctions.h"

namespace SingletonHolder {
	class ConditionHolder {
	public:
		std::vector<ArtSwap::ArtSwap>* GetSwaps(ArtSwapMode a_mode) {
			switch (a_mode) {
			case ArtSwapMode::LowPriority:
				return &LowPrioArtSwaps;
				break;
			case ArtSwapMode::Exclusive:
				return &ExclusiveArtSwaps;
				break;
			default:
				return &AdditiveArtSwaps;
			}
		}

		static ConditionHolder* GetSingleton() {
			static ConditionHolder singleton;
			return &singleton;
		}

		int GetVersion() {
			return iVerCode;
		}

		int GetNumberOfPatches() {
			int response = 0;
			response += AdditiveArtSwaps.size();
			response += ExclusiveArtSwaps.size();
			response += LowPrioArtSwaps.size();
			return response;
		}

		std::vector<RE::SpellItem*>* GetAllAbilities() {
			return &allAbilities;
		}

		std::unordered_map<RE::TESObjectWEAP*, std::vector<ArtSwap::ArtSwap>>* GetWeaponCache() {
			return &weaponCache;
		}

		//This should throw. But I am lazy.
		void Register(ArtSwap::ArtSwap a_artSwap) {
			ArtSwapMode swapMode = a_artSwap.GetMode();

			switch (swapMode) {
			case ArtSwapMode::Additive:
				AdditiveArtSwaps.push_back(a_artSwap);
				break;
			case ArtSwapMode::Exclusive:
				ExclusiveArtSwaps.push_back(a_artSwap);
				break;
			case ArtSwapMode::LowPriority:
				LowPrioArtSwaps.push_back(a_artSwap);
				break;
			}

			allAbilities.push_back(a_artSwap.GetAbility());
			allAbilities.push_back(a_artSwap.GetAbility(false));
		}

		void CreateWeaponCache() {
			auto start = std::chrono::steady_clock::now();

			_logger::info("Creating weapon cache.");

			auto weaponArray = RE::TESDataHandler::GetSingleton()->GetFormArray<RE::TESObjectWEAP>();

			for (auto weapon : weaponArray) {
				std::vector<ArtSwap::ArtSwap> matchingSwaps;
				std::vector<ArtSwap::ArtSwap> tempSwaps;
				RE::EnchantmentItem* weaponEnchant = weapon->formEnchanting;

				if (!weaponEnchant) continue;

				int lastMatchRating = 0;
				bool matchedWeapon = false;
				bool matched = false;

				ArtSwap::ArtSwap lastSwap = ArtSwap::ArtSwap();

				for (auto exclusiveSwap : ExclusiveArtSwaps) {
					if (exclusiveSwap.IsMatch(weapon, weaponEnchant)) {
						int matchRating = 0;
						matchRating += exclusiveSwap.GetRequiredEnchantmentKeywords().size();
						
						if (!exclusiveSwap.GetRequiredWeapons().empty()) {
							if (!matchedWeapon) matchRating = 0;
							matchedWeapon = true;

							if (matchRating <= lastMatchRating) continue;
							lastMatchRating = matchRating;
							lastSwap = exclusiveSwap;
							matched = true;
						}

						if (matchedWeapon) continue;

						matchRating += exclusiveSwap.GetIgnoredWeapons().size();
						matchRating += exclusiveSwap.GetRequiredWeaponKeywords().size();

						if (matchRating <= lastMatchRating) continue;
						lastMatchRating = matchRating;
						lastSwap = exclusiveSwap;
						matched = true;
					}
				}

				if (matched) {
					matchingSwaps.push_back(lastSwap);
					weaponCache[weapon] = (matchingSwaps);
					continue;
				}

				for (auto additiveSwap : AdditiveArtSwaps) {
					if (additiveSwap.IsMatch(weapon, weaponEnchant)) {
						matchingSwaps.push_back(additiveSwap);
					}
				}

				if (!matchingSwaps.empty()) {
					weaponCache[weapon] = (matchingSwaps);
					continue;
				}

				lastMatchRating = 0;
				matchedWeapon = false;
				matched = false;

				for (auto lowPrioSwap : LowPrioArtSwaps) {
					if (lowPrioSwap.IsMatch(weapon, weaponEnchant)) {
						int matchRating = 0;
						matchRating += lowPrioSwap.GetRequiredEnchantmentKeywords().size();

						if (!lowPrioSwap.GetRequiredWeapons().empty()) {
							if (!matchedWeapon) matchRating = 0;
							matchedWeapon = true;

							if (matchRating <= lastMatchRating) continue;
							lastMatchRating = matchRating;
							lastSwap = lowPrioSwap;
							matched = true;
						}

						if (matchedWeapon) continue;

						matchRating += lowPrioSwap.GetIgnoredWeapons().size();
						matchRating += lowPrioSwap.GetRequiredWeaponKeywords().size();

						if (matchRating <= lastMatchRating) continue;
						lastMatchRating = matchRating;
						lastSwap = lowPrioSwap;
						matched = true;
					}
				}

				if (!matched) continue;
				matchingSwaps.push_back(lastSwap);
				weaponCache[weapon] = (matchingSwaps);
			}

			auto end = std::chrono::steady_clock::now();
			auto execTime = end - start;
			_logger::info("Finished generating the weapon cache in {}ms. Created {} entries.", std::chrono::duration<double, std::milli>(execTime).count(), weaponCache.size());
		}

		void ModifyActorCache(RE::Actor* a_actor, ActorCacheAction a_action, RE::SpellItem* a_ability = nullptr) {
			switch (a_action) {
			case(ActorCacheAction::AddToCache):
				break;
			case(ActorCacheAction::ClearFromCache):
				break;
			case(ActorCacheAction::AddAbilityLeft):
				break;
			case(ActorCacheAction::AddAbilityRight):
				break;
			case(ActorCacheAction::RemoveAbilityLeft):
				break;
			case(ActorCacheAction::RemoveAbilityRight):
				break;
			default:
				break;

			}
		}

		ConditionHolder(const ConditionHolder& obj) = delete;
	private:
		int                           iVerCode;
		std::vector<ArtSwap::ArtSwap> AdditiveArtSwaps;
		std::vector<ArtSwap::ArtSwap> ExclusiveArtSwaps;
		std::vector<ArtSwap::ArtSwap> LowPrioArtSwaps;
		std::unordered_map<RE::TESObjectWEAP*, std::vector<ArtSwap::ArtSwap>> weaponCache;
		std::unordered_map<RE::Actor*, std::vector<RE::SpellItem*>> actorCache;
		std::vector<RE::SpellItem*>   allAbilities;

		ConditionHolder() {
			this->iVerCode = 1;
			weaponCache = std::unordered_map<RE::TESObjectWEAP*, std::vector<ArtSwap::ArtSwap>>();
		}
	};
}
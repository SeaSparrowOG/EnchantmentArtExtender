#include "singletonHolder.h"

namespace SingletonHolder {

	/**
	* Returns a pointer to the best matching art swap in a given array.
	* @param a_swaps The array to iterate over.
	* @param a_weapon The weapon to match against. If it has extra data, that is taken into account.
	*/
	ArtSwap::ArtSwap ConditionHolder::GetBestMatchingSwap(std::vector<ArtSwap::ArtSwap> a_swaps, RE::TESObjectWEAP* a_weapon, RE::EnchantmentItem* a_enchant) {
		RE::EnchantmentItem* weaponEnchant;
		if (!a_enchant) weaponEnchant = a_weapon->formEnchanting;
		else weaponEnchant = a_enchant;

		ArtSwap::ArtSwap response = ArtSwap::ArtSwap();
		if (!weaponEnchant) return response;

		int lastMatchRating = 0;
		bool matchedWeapon = false;

		for (auto& swap : a_swaps) {
			if (swap.IsMatch(a_weapon, weaponEnchant)) {
				int matchRating = 0;
				matchRating += swap.GetRequiredEnchantmentKeywords().size();

				if (!swap.GetRequiredWeapons().empty()) {
					if (!matchedWeapon) matchRating = 0;
					matchedWeapon = true;

					if (matchRating <= lastMatchRating) continue;
					lastMatchRating = matchRating;
					response = swap;
				}

				if (matchedWeapon) continue;

				matchRating += swap.GetIgnoredWeapons().size();
				matchRating += swap.GetRequiredWeaponKeywords().size();

				if (matchRating <= lastMatchRating) continue;
				lastMatchRating = matchRating;
				response = swap;
			}
		}
		return response;
	}

	//To Follow - Class Method Definitions.
	ConditionHolder* ConditionHolder::GetSingleton() {
		static ConditionHolder singleton;
		return &singleton;
	}

	int ConditionHolder::GetVersion() {
		return iVerCode;
	}

	int ConditionHolder::GetNumberOfPatches() {
		int response = 0;
		response += AdditiveArtSwaps.size();
		response += ExclusiveArtSwaps.size();
		response += LowPrioArtSwaps.size();
		return response;
	}

	_weaponCache* ConditionHolder::GetWeaponCache() {
		return &weaponCache;
	}

	std::vector<RE::SpellItem*>* ConditionHolder::GetAllAbilities() {
		return &allAbilities;
	}

	std::vector<ArtSwap::ArtSwap>* ConditionHolder::GetSwaps(ArtSwapMode a_mode) {
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

	void ConditionHolder::Register(ArtSwap::ArtSwap a_artSwap) {
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

	void ConditionHolder::CreateWeaponCache() {
		auto start = std::chrono::steady_clock::now();

		_logger::info("Creating weapon cache.");

		auto weaponArray = RE::TESDataHandler::GetSingleton()->GetFormArray<RE::TESObjectWEAP>();

		for (auto weapon : weaponArray) {
			std::vector<ArtSwap::ArtSwap> matchingSwaps;
			RE::EnchantmentItem* weaponEnchant = weapon->formEnchanting;

			if (!weaponEnchant) continue;

			auto foundSwap = GetBestMatchingSwap(ExclusiveArtSwaps, weapon);

			if (!foundSwap.GetName().empty()) {
				matchingSwaps.push_back(foundSwap);
				weaponCache[weapon] = (matchingSwaps);
				continue;
			}

			for (auto& additiveSwap : AdditiveArtSwaps) {
				if (additiveSwap.IsMatch(weapon, weaponEnchant)) {
					matchingSwaps.push_back(additiveSwap);
				}
			}

			if (!matchingSwaps.empty()) {
				weaponCache[weapon] = (matchingSwaps);
				continue;
			}

			foundSwap = GetBestMatchingSwap(LowPrioArtSwaps, weapon);

			if (!foundSwap.GetName().empty()) {
				matchingSwaps.push_back(foundSwap);
				weaponCache[weapon] = (matchingSwaps);
				continue;
			}
		}

		auto end = std::chrono::steady_clock::now();
		auto execTime = end - start;
		_logger::info("Finished generating the weapon cache in {}ms. Created {} entries.", std::chrono::duration<double, std::milli>(execTime).count(), weaponCache.size());
	}
}
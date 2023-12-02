#pragma once
#include "artSwap.h"
#include "helperFunctions.h"

namespace SingletonHolder {
	class ConditionHolder {
	public:
		static ConditionHolder*        GetSingleton();
		int                            GetVersion();
		int                            GetNumberOfPatches();
		void                           Register(ArtSwap::ArtSwap a_artSwap);
		void                           CreateWeaponCache();
		_weaponCache*                  GetWeaponCache();
		std::vector<RE::SpellItem*>*   GetAllAbilities();
		std::vector<ArtSwap::ArtSwap>* GetSwaps(ArtSwapMode a_mode);

		ConditionHolder(const ConditionHolder& obj) = delete;
	private:
		int                           iVerCode;
		_weaponCache                  weaponCache;
		std::vector<ArtSwap::ArtSwap> AdditiveArtSwaps;
		std::vector<ArtSwap::ArtSwap> ExclusiveArtSwaps;
		std::vector<ArtSwap::ArtSwap> LowPrioArtSwaps;
		std::vector<RE::SpellItem*>   allAbilities;

		ConditionHolder() {
			this->iVerCode = 1;
			weaponCache = _weaponCache();
		}
	};
}
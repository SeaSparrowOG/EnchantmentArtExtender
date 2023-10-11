#pragma once

namespace SingletonHolder {
	class ConditionHolder {
	public:
		std::vector<ArtSwap>*  GetSwaps() {
			return &ArtSwaps;
		}

		static ConditionHolder* GetSingleton() {
			return &Instance;
		}

		int                     GetVersion() {
			return iVerCode;
		}

		void                    CreateSwap(ArtSwap a_artSwap) {
			this->ArtSwaps.push_back(a_artSwap);
		}

		ConditionHolder(const ConditionHolder& obj) = delete;
	private:
		int                     iVerCode;
		std::vector<ArtSwap>    ArtSwaps;
		static ConditionHolder  Instance;

		ConditionHolder() {
			this->iVerCode = 1;
		}
	};
	ConditionHolder ConditionHolder::Instance;
}
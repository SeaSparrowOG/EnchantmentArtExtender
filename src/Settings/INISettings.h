#pragma once

#include "utilities/utilities.h"

namespace Settings
{
	namespace INI
	{
		class SettingsHolder : public Utilities::Singleton::ISingleton<SettingsHolder> {
		public:
			void Read();
			RE::TESEffectShader* GetEmptyShader();
			RE::TESObjectLIGH* GetLight(); //unused for now.

			bool useEmptyShader{ false };

		private:
			RE::TESEffectShader* emptyShader;
			RE::TESObjectLIGH* light;
		};
	}
}
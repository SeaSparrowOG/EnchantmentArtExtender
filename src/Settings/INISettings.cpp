#include "Settings/INISettings.h"

#include <SimpleIni.h>
#include "utilities/utilities.h"

void Settings::INI::SettingsHolder::Read()
{
	CSimpleIniA ini{};
	ini.SetUnicode();
	ini.LoadFile(fmt::format(R"(.\Data\SKSE\Plugins\{}.ini)", Plugin::NAME).c_str());

	std::string sEmptyShader = ini.GetValue("General", "sEmptyShader", "EnchantmentArtExtender.esl|0x800");
	if (!sEmptyShader.empty()) {
		const auto candidateShader = Utilities::Forms::GetFormFromString<RE::TESEffectShader>(sEmptyShader);
		this->emptyShader = candidateShader;
	}

	std::string sLight = ini.GetValue("General", "sLight", "EnchantmentArtExtender.esl|0x801");
	if (!sLight.empty()) {
		const auto candidateLight = Utilities::Forms::GetFormFromString<RE::TESObjectLIGH>(sLight);
		this->light = candidateLight;
	}
}

RE::TESEffectShader* Settings::INI::SettingsHolder::GetEmptyShader()
{
	return this->emptyShader;
}

RE::TESObjectLIGH* Settings::INI::SettingsHolder::GetLight()
{
	return this->light;
}

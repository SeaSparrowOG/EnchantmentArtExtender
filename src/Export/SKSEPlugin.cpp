#include "EffectManager/EffectManager.h"
#include "Data/ModObjectManager.h"
#include "Hooks/Hooks.h"
#include "Settings/INI/INISettings.h"
#include "Settings/JSON/JSONSettings.h"

static void MessageEventCallback(SKSE::MessagingInterface::Message* a_msg)
{
	static auto* jsonHolder = Settings::JSON::Holder::GetSingleton();
	if (!jsonHolder) {
		REX::FAIL("Failed to get internal JSON logger."sv);
	}

	switch (a_msg->type) {
	case SKSE::MessagingInterface::kDataLoaded:
		if (!Data::PreloadModObjects()) {
			REX::FAIL(
				fmt::format("Failed to preload mod objects. Check the log (Documents/My Games/Skyrim Special Edition/{}.log for more information."sv, Plugin::NAME));
		}
		SECTION_SEPARATOR;
		if (!EffectManager::ReadConfigs()) {
			REX::FAIL(
				fmt::format("Failed to parse configs. Check the log (Documents/My Games/Skyrim Special Edition/{}.log for more information."sv, Plugin::NAME));
		}
		SECTION_SEPARATOR;
		jsonHolder->Release();
		REX::INFO("Finished startup tasks, enjoy your game!"sv);
		break;
	default:
		break;
	}
}

extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []()
	{
		SKSE::PluginVersionData v{};

		v.PluginVersion(Plugin::VERSION);
		v.PluginName(Plugin::NAME);
		v.AuthorName("SeaSparrow"sv);
		v.UsesAddressLibrary();
		v.UsesUpdatedStructs();

		return v;
	}();

SKSE_PLUGIN_QUERY(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
	a_info->infoVersion = SKSE::PluginInfo::kVersion;
	a_info->name = Plugin::NAME.data();
	a_info->version = Plugin::VERSION[0];

	if (a_skse->IsEditor()) {
		REX::CRITICAL("Loaded in editor, marking as incompatible"sv);
		return false;
	}
	return true;
}

SKSE_PLUGIN_LOAD(const SKSE::LoadInterface * a_skse)
{
	SKSE::InitInfo info;
	info.log = true;
	info.hook = true;
	info.trampoline = true;
	info.trampolineSize = 14u * 1u + 14u;

	SKSE::Init(a_skse, info);
	REX::INFO("Author: SeaSparrow"sv);
	SECTION_SEPARATOR;

	const auto ver = a_skse->RuntimeVersion();

	static constexpr std::array<REL::Version, 2> supported = 
	{
		SKSE::RUNTIME_SSE_1_7_104,
		SKSE::RUNTIME_SSE_1_7_99
	};

	if (!std::ranges::contains(supported, ver)) {
		REX::CRITICAL("Game Version: {}"sv, ver.string());
		REX::CRITICAL("Supported Versions:"sv);
		for (const auto& allowed : supported) {
			REX::CRITICAL("  - {}"sv, allowed.string());
		}
		REX::FAIL(
			fmt::format("You are using a version not supported by this plugin. Check the log at (Documents/My Games/Skyrim Special Edition/{}.log for more information."sv, Plugin::NAME)
		);
	}

	REX::INFO("Performing startup tasks..."sv);

	if (!Settings::INI::Read()) {
		REX::FAIL(
			fmt::format("Failed to load the INI settings. Check the log (Documents/My Games/Skyrim Special Edition/{}.log for more information."sv, Plugin::NAME));
	}
	SECTION_SEPARATOR;
	if (!Hooks::Install()) {
		REX::FAIL(
			fmt::format("Failed to install the necessary hooks. Check the log (Documents/My Games/Skyrim Special Edition/{}.log for more information."sv, Plugin::NAME));
	}
	SECTION_SEPARATOR;

	const auto messaging = SKSE::GetMessagingInterface();
	messaging->RegisterListener(&MessageEventCallback);

	SECTION_SEPARATOR;
	if (!Settings::JSON::Preload()) {
#ifdef NDEBUG
		REX::FAIL(
			fmt::format("Failed to parse configs. Check the log (Documents/My Games/Skyrim Special Edition/{}.log for more information."sv, Plugin::NAME));
#endif
	}
	
	return true;
}
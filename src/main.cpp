#include "actorEventManager.h"
#include "cache.h"

void SetupLog() {
    auto logsFolder = SKSE::log::log_directory();
    if (!logsFolder) SKSE::stl::report_and_fail("SKSE log_directory not provided, logs disabled.");

    auto pluginName = Version::PROJECT;
    auto logFilePath = *logsFolder / std::format("{}.log", pluginName);
    auto fileLoggerPtr = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true);
    auto loggerPtr = std::make_shared<spdlog::logger>("log", std::move(fileLoggerPtr));

    spdlog::set_default_logger(std::move(loggerPtr));
    spdlog::set_level(spdlog::level::info);
    spdlog::flush_on(spdlog::level::info);

    //Pattern
    spdlog::set_pattern("%v");
}

void MessageHandler(SKSE::MessagingInterface::Message* a_message) {
    bool goodToGo = true;
    switch (a_message->type) {
    case SKSE::MessagingInterface::kDataLoaded:
        if (ActorEvents::LoadEvent::GetSingleton()->Register()) {
            _loggerInfo("Registered for Load/Unload events.");
        }
        else {
            goodToGo = false;
        }
        
        if (goodToGo && ActorEvents::EquipEvent::GetSingleton()->Register()) {
            _loggerInfo("Registered for Equip events.");
        }
        else {
            goodToGo = false;
        }

        if (goodToGo && ActorEvents::DeathEvent::GetSingleton()->Register()) {
            _loggerInfo("Registered for Death events.");
        }
        else {
            goodToGo = false;
        }

        if (goodToGo && Cache::StoredData::GetSingleton()->InitiateCache()) {
            _loggerInfo("Built the Cache system.");
        }
        else {
            goodToGo = false;
        }

        if (goodToGo) {
            _loggerInfo("All pre-game tasks finished successfully, enjoy your game!");
        }
        else {
            _loggerInfo("A task failed. The framework will not load.");
            ActorEvents::LoadEvent::GetSingleton()->UnRegister();
            ActorEvents::EquipEvent::GetSingleton()->UnRegister();
            ActorEvents::DeathEvent::GetSingleton()->UnRegister();
        }
        break;
    default:
        break;
    }
}

#ifdef SKYRIM_AE
extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() {
    SKSE::PluginVersionData v;
    v.PluginVersion({ Version::MAJOR, Version::MINOR, Version::PATCH });
    v.PluginName(Version::PROJECT);
    v.AuthorName("SeaSparrow");
    v.UsesAddressLibrary();
    v.UsesNoStructs();
    v.CompatibleVersions({
        SKSE::RUNTIME_1_6_1130,
        _1_6_1170 });
    return v;
    }();
#else 
extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface * a_skse, SKSE::PluginInfo * a_info)
{
    a_info->infoVersion = SKSE::PluginInfo::kVersion;
    a_info->name = "EnchantmentArtExtender";
    a_info->version = 1;

    const auto ver = a_skse->RuntimeVersion();
    if (ver
#	ifndef SKYRIMVR
        < SKSE::RUNTIME_1_5_39
#	else
        > SKSE::RUNTIME_VR_1_4_15_1
#	endif
        ) {
        SKSE::log::critical(FMT_STRING("Unsupported runtime version {}"), ver.string());
        return false;
    }

    return true;
}
#endif

    extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface * a_skse) {
        SetupLog();
        _loggerInfo("Starting up Enchantment Effects Extender.");
#ifdef BUILD_AE
        _loggerInfo("Plugin Version: {}.{}.{}", Version::MAJOR, Version::MINOR, Version::PATCH);
#else 
        _loggerInfo("Plugin Version: {}.{}.{}, 1.5 build.", Version::MAJOR, Version::MINOR, Version::PATCH);
        _loggerInfo("Do not report ANY issues with this version.");
#endif
        _loggerInfo("==================================================");

        SKSE::Init(a_skse);
        auto messaging = SKSE::GetMessagingInterface();
        messaging->RegisterListener(MessageHandler);

        return true;
    }
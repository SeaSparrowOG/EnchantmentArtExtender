#include <spdlog/sinks/basic_file_sink.h>
#include "configParser.h"
#include "equipEventHandler.h"
#include "actorEventHandler.h"

namespace logger = SKSE::log;

void SetupLog() {
    auto logsFolder = SKSE::log::log_directory();
    if (!logsFolder) SKSE::stl::report_and_fail("SKSE log_directory not provided, logs disabled.");

    auto pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();
    auto logFilePath = *logsFolder / std::format("{}.log", pluginName);
    auto fileLoggerPtr = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true);
    auto loggerPtr = std::make_shared<spdlog::logger>("log", std::move(fileLoggerPtr));

    spdlog::set_default_logger(std::move(loggerPtr));
    spdlog::set_level(spdlog::level::info);
    spdlog::flush_on(spdlog::level::info);

    //Pattern
    spdlog::set_pattern("%v");
}

void MessageHandler(SKSE::MessagingInterface::Message* a_message)
{
    switch (a_message->type) {
    case SKSE::MessagingInterface::kDataLoaded: {
        ConfigParser::ParseConfigs();
        EquipEventHandler::EquipEvent::GetSingleton()->RegisterListener();
        ActorEventHandler::ActorLoadedEvent::GetSingleton()->RegisterActorLoadedEvent();
        ActorEventHandler::ActorDeathEvent::GetSingleton()->RegisterActorDeathEvent();
        break;
    }
    default:
        break;
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* skse) {
    SetupLog();
    SKSE::Init(skse);
    auto messaging = SKSE::GetMessagingInterface();
    messaging->RegisterListener(MessageHandler);
    return true;
}
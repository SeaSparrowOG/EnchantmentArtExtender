#pragma once

namespace ActorEventHandler {
	class ActorLoadedEvent : RE::BSTEventSink<RE::TESObjectLoadedEvent> {

	public:
		static void   RegisterActorLoadedEvent();
		RE::BSEventNotifyControl ProcessEvent(const RE::TESObjectLoadedEvent* a_event, RE::BSTEventSource<RE::TESObjectLoadedEvent>* a_eventSource) override;
		static ActorLoadedEvent* GetSingleton();
	};

	class ActorDeathEvent : RE::BSTEventSink<RE::TESDeathEvent> {

	public:
		static void   RegisterActorDeathEvent();
		RE::BSEventNotifyControl ProcessEvent(const RE::TESDeathEvent* a_event, RE::BSTEventSource<RE::TESDeathEvent>* a_eventSource) override;
		static ActorDeathEvent* GetSingleton();
	};
}
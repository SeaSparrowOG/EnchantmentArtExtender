#pragma once

namespace ActorEvents {
#define continueEvent RE::BSEventNotifyControl::kContinue

	/*
	* Listener for when anything loads or unloads.
	*/
	class LoadEvent : public clib_util::singleton::ISingleton<LoadEvent>,
		public RE::BSTEventSink<RE::TESObjectLoadedEvent> {
	public:
		bool Register();
		void UnRegister();

	private:
		RE::BSEventNotifyControl ProcessEvent(const RE::TESObjectLoadedEvent* a_event, RE::BSTEventSource<RE::TESObjectLoadedEvent>* a_eventSource) override;

		bool registered;
	};

	/*
	* Listener for when anyone equips something.
	*/
	class EquipEvent : public clib_util::singleton::ISingleton<EquipEvent>,
		public RE::BSTEventSink<RE::TESEquipEvent> {
	public:
		bool Register();
		void UnRegister();

	private:
		RE::BSEventNotifyControl ProcessEvent(const RE::TESEquipEvent* a_event, RE::BSTEventSource<RE::TESEquipEvent>* a_eventSource) override;

		bool registered;
	};

	/*
	* Listener for when an actor dies.
	*/
	class DeathEvent : public clib_util::singleton::ISingleton<DeathEvent>,
		public RE::BSTEventSink<RE::TESDeathEvent> {
	public:
		bool Register();
		void UnRegister();

	private:
		RE::BSEventNotifyControl ProcessEvent(const RE::TESDeathEvent* a_event, RE::BSTEventSource<RE::TESDeathEvent>* a_eventSource) override;

		bool registered;
	};

	/*
	* Listener for Seathe/Draw events.
	*/
	class AnimationEventListener : public clib_util::singleton::ISingleton<AnimationEventListener>,
		public RE::BSTEventSink<RE::BSAnimationGraphEvent> {
	public:
		void RegisterActor(RE::Actor* a_actor);
		void UnRegisterActor(RE::Actor* a_actor);
	private:
		RE::BSEventNotifyControl ProcessEvent(const RE::BSAnimationGraphEvent* a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource) override;

		std::unordered_map<RE::Actor*, bool> managedActors;
	};

	/*
	* Actor registry. Helps avoid dulicate calls.
	*/
	class ActorRegistry : public clib_util::singleton::ISingleton<ActorRegistry> {
	public:
		void ProcessEquipEvent(RE::Actor* a_actor, bool a_equipped);
		void ProcessDeathEvent(RE::Actor* a_actor);
		void ProcessAnimationEvent(RE::Actor* a_actor, bool a_sheathing);
		void ProcessLoadEvent(RE::Actor* a_actor, bool a_loaded);
	private:
		/*
		Cache for the actor's state. If the actor is managed, it will be given 3 pieces of information:
		1. The current left enchanted equipped weapon.
		2. The current right enchanted equipped weapon.
		3. If they have their hands raised.
		*/
		std::unordered_map<RE::Actor*, std::pair<std::pair<RE::TESObjectWEAP*, RE::TESObjectWEAP*>, bool>> managedActors;
	};
}
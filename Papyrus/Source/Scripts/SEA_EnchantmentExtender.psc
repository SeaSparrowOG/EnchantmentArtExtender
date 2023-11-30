Scriptname SEA_EnchantmentExtender Hidden

;Returns the version of the extender as an int.
Int Function GetVersion() Global Native
;Useful only for debugging. Reloads all configs. Do not use this for gameplay. Seriously.
Bool Function ReloadConfigs() Global Native
;Returns true if given conflict has been loaded.
Bool Function IsConfigValid(String a_sConfigName)
;Returns all actors that currently have an active ability.
Actor[] Function GetManagedActors() Global Native
;Returns all active swaps on a given actor.
Spell[] Function GetActorAbilities(Actor a_kActor)
;Applies the given swap to an actor of your choice, ignoring conditions.
Function ApplyEffectOnActor(Actor a_kTarget, String a_sConfigName, String a_sSwapName)

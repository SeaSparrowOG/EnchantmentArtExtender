Scriptname SEA_EnchantmentExtender Hidden

;Useful only for debugging. Reloads all configs. Do not use this for gameplay. Seriously.
Bool Function ReloadConfigs() Global Native
;Returns all active arts on a given actor.
Spell[] Function GetAllActorActiveArts(Actor a_kTarget) Global Native
;Returns an array with all loaded swaps. If you want to know if your config is loaded, just iterate through it.
String[] Function GetAllValidSwaps() Global Native

;Logs all applied swaps to the log file.
Function LogAllSwaps() Global Native
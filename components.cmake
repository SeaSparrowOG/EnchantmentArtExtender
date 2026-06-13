install(
	FILES
		"${ARTIFACT_FOLDER}/SKSE/Plugins/EnchantmentArtExtender.ini"
	DESTINATION "SKSE/Plugins"
	COMPONENT components
	EXCLUDE_FROM_ALL
)

install(
	FILES
		"${ARTIFACT_FOLDER}/EnchantmentArtExtender.esl"
	DESTINATION "."
	COMPONENT components
	EXCLUDE_FROM_ALL
)

install(
	FILES
		"${ARTIFACT_FOLDER}/EnchantmentArtExtender.bsa"
	DESTINATION "."
	COMPONENT components
	EXCLUDE_FROM_ALL
)
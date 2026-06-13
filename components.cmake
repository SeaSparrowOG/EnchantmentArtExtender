install(
	FILES
		"${ARTIFACT_DIR}/SKSE/Plugins/EnchantmentArtExtender.ini"
	DESTINATION "SKSE/Plugins"
	COMPONENT components
	EXCLUDE_FROM_ALL
)

install(
	FILES
		"${ARTIFACT_DIR}/EnchantmentArtExtender.esl"
	DESTINATION "."
	COMPONENT components
	EXCLUDE_FROM_ALL
)

install(
	FILES
		"${ARTIFACT_DIR}/EnchantmentArtExtender.bsa"
	DESTINATION "."
	COMPONENT components
	EXCLUDE_FROM_ALL
)
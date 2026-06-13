cmake_minimum_required(VERSION 3.24)

option(FULL_RELEASE "Set by your preset, DO NOT edit. Applies Release rules." OFF)
if(FULL_RELEASE AND CMAKE_CONFIGURATION_TYPES)
    message(STATUS "FULL_RELEASE enabled; ensure Release configuration is used")
endif()

# -------- Project ----------
project(
	EnchantmentArtExtender
	VERSION 3.0.0
	LANGUAGES CXX
)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

get_filename_component(
    Skyrim64Path
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\Bethesda Softworks\\Skyrim Special Edition;installed path]"
    ABSOLUTE
)

set(Skyrim64Path "${Skyrim64Path}" CACHE PATH "Skyrim SE install directory")

if(NOT IS_DIRECTORY "${Skyrim64Path}")
    message(WARNING "Skyrim SE install path not found; Papyrus scripts will not be compiled or archived.")
endif()

list(APPEND CMAKE_MODULE_PATH
	"${PROJECT_SOURCE_DIR}/tools/SKSE-CMakeModules/Modules"
)
list(APPEND CMAKE_MODULE_PATH
	"${PROJECT_SOURCE_DIR}/tools/Bethesda-CMakeModules/Modules"
)

include(SKSEPlugin)
set(SKSE_USE_XBYAK ON)
add_compile_definitions(SKSE_SUPPORT_XBYAK)
add_compile_definitions(SKYRIM_AE)
add_compile_definitions(SKYRIM_SUPPORT_AE)

SKSEPlugin_Add(
	${PROJECT_NAME}
	SOURCE_DIR src
	SOURCES
		README.md
		vcpkg.json
	PRECOMPILE_HEADERS
		src/common/PCH.h
)

find_path(CLIB_UTIL_INCLUDE_DIRS "ClibUtil/utils.hpp")
find_package(jsoncpp CONFIG REQUIRED)

target_link_libraries(
		"${PROJECT_NAME}"
		PRIVATE
			JsonCpp::JsonCpp
)

target_include_directories(
	${PROJECT_NAME}
	PRIVATE
		${CLIB_UTIL_INCLUDE_DIRS}
)

set_target_properties(CommonLibSSE PROPERTIES
	FOLDER External
)

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

if (MSVC)
	target_compile_options(
		${PROJECT_NAME}
		PRIVATE
			/sdl                    # Enable Additional Security Checks
			/utf-8                  # Set Source and Executable character sets to UTF-8
			/Zi                     # Debug Information Format

			/permissive-            # Standards conformance
			/Zc:preprocessor        # Enable preprocessor conformance mode
			/Zc:__cplusplus
			
			/wd4200                 # nonstandard extension used : zero-sized array in struct/union
			/W4                     # More warnings 
			/external:anglebrackets # Ignore warnings for external resources.
			/external:W0            # Same as above
			
			"$<$<CONFIG:DEBUG>:>"
			"$<$<CONFIG:RELEASE>:/WX;/Zc:inline;/JMC-;/Ob3>"
	)

	target_link_options(
		${PROJECT_NAME}
		PRIVATE
			"$<$<CONFIG:DEBUG>:/INCREMENTAL;/OPT:NOREF;/OPT:NOICF>"
			"$<$<CONFIG:RELEASE>:/INCREMENTAL:NO;/OPT:REF;/OPT:ICF;/DEBUG:FULL>"
	)
endif ()

set(DATA_DIR "${PROJECT_SOURCE_DIR}/Data")
set(SKSE_DIR "${DATA_DIR}/SKSE")
set(SKSE_PLUGINS_DIR "${SKSE_DIR}/Plugins")

set(INI_PATH "${SKSE_PLUGINS_DIR}/${PROJECT_NAME}.ini")

include(components.cmake)

# ---- Post build ----
if(DEFINED ENV{SKYRIM_MODS_FOLDER} AND IS_DIRECTORY "$ENV{SKYRIM_MODS_FOLDER}")
	if (FULL_RELEASE)
		set(OUTPUT_FOLDER "$ENV{SKYRIM_MODS_FOLDER}/${PROJECT_NAME} - Full Release")
	else()
		set(OUTPUT_FOLDER "$ENV{SKYRIM_MODS_FOLDER}/${PROJECT_NAME} - Test")
	endif()
endif()

# ---- Automatically copy artifacts to MO2 ----
if(DEFINED OUTPUT_FOLDER)
    set(DLL_FOLDER "${OUTPUT_FOLDER}/SKSE/Plugins")

	set(POST_BUILD_COMMANDS
		COMMAND "${CMAKE_COMMAND}" -E copy_if_different
			"$<TARGET_FILE:${PROJECT_NAME}>"
			"${DLL_FOLDER}/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
		COMMAND "${CMAKE_COMMAND}" -E copy_if_different
			"$<$<BOOL:$<TARGET_PDB_FILE:${PROJECT_NAME}>>:$<TARGET_PDB_FILE:${PROJECT_NAME}>>"
			"${DLL_FOLDER}/$<TARGET_PDB_FILE_NAME:${PROJECT_NAME}>"
		COMMAND "${CMAKE_COMMAND}"
			-DSRC_DIR=${DATA_DIR}
			-DDST_DIR=${OUTPUT_FOLDER}
			-DPROJECT_NAME=${PROJECT_NAME}
			-P "${PROJECT_SOURCE_DIR}/CMake/Scripts/CMake-Copy-Artifacts.cmake"
	)
	
	
	message(STATUS "==== Build Configuration ====")
	message(STATUS "FULL_RELEASE      = ${FULL_RELEASE}")
	message(STATUS "OUTPUT_FOLDER     = ${OUTPUT_FOLDER}")
	message(STATUS "=============================")

	add_custom_command(
		TARGET "${PROJECT_NAME}"
		POST_BUILD
		${POST_BUILD_COMMANDS}
		VERBATIM
	)
else()
	message(STATUS "Output folder environment variable not set, build files will not be automatically deployed to MO2.")
endif()
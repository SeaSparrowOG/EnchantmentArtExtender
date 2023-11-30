cmake_minimum_required(VERSION 3.21)

project(
	EnchantmentEffectsExtender 
	VERSION 1.0.0 
	DESCRIPTION "Allows for the application of unique effects to enchanted weapons based on configs."
	LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON) 

#===========================================================================
#MO2 Deployment
#===========================================================================
if(DEFINED ENV{SKYRIM_MODS_FOLDER} AND IS_DIRECTORY "$ENV{SKYRIM_MODS_FOLDER}")
    set(OUTPUT_FOLDER "$ENV{SKYRIM_MODS_FOLDER}/${PROJECT_NAME}")
endif()

#===========================================================================
#Plugin depoendencies
#===========================================================================
find_package(CommonLibSSE CONFIG REQUIRED)
find_package(jsoncpp CONFIG REQUIRED)
find_path(CLIB_UTIL_INCLUDE_DIRS "ClibUtil/utils.hpp")

set(headers
        include/actorEventHandler.h
        include/helperFunctions.h
        include/artSwap.h
        include/configParser.h
        include/equipEventHandler.h
        include/singletonHolder.h) 

set(sources
        src/actorEventHandler.cpp
        src/helperFunctions.cpp
        src/artSwap.cpp
		src/configParser.cpp
        src/equipEventHandler.cpp
        src/main.cpp)

add_commonlibsse_plugin(${PROJECT_NAME} SOURCES ${headers} ${sources})
add_library("${PROJECT_NAME}::${PROJECT_NAME}" ALIAS "${PROJECT_NAME}")

target_include_directories(${PROJECT_NAME}
        PRIVATE
        $<INSTALL_INTERFACE:${CLIB_UTIL_INCLUDE_DIRS}>
        $<INSTALL_INTERFACE:${JSONCPP_INCLUDE_DIRS}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/src>
        $<INSTALL_INTERFACE:src>)

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        JsonCpp::JsonCpp
        CommonLibSSE::CommonLibSSE)

target_include_directories(${PROJECT_NAME}
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

target_precompile_headers(${PROJECT_NAME}
            PRIVATE 
            src/PCH.h)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

install(TARGETS ${PROJECT_NAME}
        DESTINATION "${CMAKE_INSTALL_LIBDIR}")

#===========================================================================
#Build and deployment to MO2's directory
#===========================================================================
if(DEFINED OUTPUT_FOLDER)
    set(DLL_FOLDER "${OUTPUT_FOLDER}/SKSE/Plugins")
    set(CONFIG_FOLDER "${OUTPUT_FOLDER}/Enchantment Effects Extender")
    set(PAPYRUS_SOURCE_FOLDER "${OUTPUT_FOLDER}/Source/Scripts")
    set(PAPYRUS_SCRIPTS_FOLDER "${OUTPUT_FOLDER}/Scripts")

    add_custom_command(
        TARGET "${PROJECT_NAME}"
        POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:${PROJECT_NAME}>" "${DLL_FOLDER}/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
        COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_SOURCE_DIR}/exampleJSON/" "${CONFIG_FOLDER}"
        COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_SOURCE_DIR}/Papyrus/Source/Scripts/" "${PAPYRUS_SOURCE_FOLDER}"
        VERBATIM
    )

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_custom_command(
            TARGET "${PROJECT_NAME}"
            POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_PDB_FILE:${PROJECT_NAME}>" "${DLL_FOLDER}/$<TARGET_PDB_FILE_NAME:${PROJECT_NAME}>"
            VERBATIM
        )
    endif()
endif()
 
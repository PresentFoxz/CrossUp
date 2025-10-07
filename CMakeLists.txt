cmake_minimum_required(VERSION 3.14)
set(CMAKE_C_STANDARD 11)

set(ENVSDK $ENV{PLAYDATE_SDK_PATH})

if (NOT ${ENVSDK} STREQUAL "")
	# Convert path from Windows
	file(TO_CMAKE_PATH ${ENVSDK} SDK)
else()
	execute_process(
			COMMAND bash -c "egrep '^\\s*SDKRoot' $HOME/.Playdate/config"
			COMMAND head -n 1
			COMMAND cut -c9-
			OUTPUT_VARIABLE SDK
			OUTPUT_STRIP_TRAILING_WHITESPACE
	)
endif()

if (NOT EXISTS ${SDK})
	message(FATAL_ERROR "SDK Path not found; set ENV value PLAYDATE_SDK_PATH")
	return()
endif()

set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
set(CMAKE_XCODE_GENERATE_SCHEME TRUE)

# Game Name Customization
set(PLAYDATE_GAME_NAME "CrossUp")
set(PLAYDATE_GAME_DEVICE "CrossUp")

message(STATUS "Game name: ${PLAYDATE_GAME_NAME}")
project(${PLAYDATE_GAME_NAME} C ASM)

# Enable high optimization for release builds
set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG -mcpu=cortex-m7 -mfpu=fpv5-sp-d16 -mfloat-abi=hard -ffast-math" CACHE STRING "Release build flags" FORCE)
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O3 -g -DNDEBUG" CACHE STRING "Release with debug info" FORCE)

add_compile_options(-O3)

set(SOURCES
    ../C/main.c
	../C/library.c
	../C/entities.c
	../C/movement.c
	../C/draw.c
	../C/collisions.c
	#../C/syscall.c
)

if (TOOLCHAIN STREQUAL "armgcc")
	add_executable(${PLAYDATE_GAME_DEVICE} ${SOURCES})
else()
    add_library(${PLAYDATE_GAME_NAME} SHARED ${SOURCES} ${IMAGES} ${FILES})
endif()

include(${SDK}/C_API/buildsupport/playdate_game.cmake)

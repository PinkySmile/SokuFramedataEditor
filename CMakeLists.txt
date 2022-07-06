cmake_minimum_required(VERSION 3.13)
project(SoFGV CXX)

set(CMAKE_CXX_STANDARD 17)

add_compile_definitions(_USE_MATH_DEFINES=1)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/pkgs)
find_package(SFML REQUIRED)
find_package(TGUI REQUIRED)
include_directories(include ${SFML_INCLUDE_DIRS} ${TGUI_INCLUDE_DIRS} ${clip_INCLUDE_DIRS} src/Core)

add_subdirectory(shady-packer)

if (CMAKE_BUILD_TYPE STREQUAL Debug)
	if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		add_compile_options(-W -Wall -Wextra)
	endif ()
	add_compile_definitions(_DEBUG=1)
else ()
	if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		if (WIN32)
			add_link_options(-mwindows)
		endif()
	endif ()
	add_compile_definitions(NDEBUG=1)
endif ()

if (WIN32)
	set(GICON src/Game/game.rc)
	set(FDICON src/FrameDataEditor/fdeditor.rc)
else()
	set(GICON)
	set(FDICON)
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	if ("${CMAKE_BUILD_TYPE}" STREQUAL Debug)
		set(TGUI_LIB "${TGUI_DIR}/lib/tgui-d.lib")
		set(SFML_GRAPHICS_LIB "${SFML_DIR}/lib/sfml-graphics-d.lib")
		set(SFML_AUDIO_LIB "${SFML_DIR}/lib/sfml-audio-d.lib")
		set(SFML_SYSTEM_LIB "${SFML_DIR}/lib/sfml-system-d.lib")
		set(SFML_WINDOW_LIB "${SFML_DIR}/lib/sfml-window-d.lib")
		set(SFML_NETWORK_LIB "${SFML_DIR}/lib/sfml-network-d.lib")
	else ()
		set(TGUI_LIB "${TGUI_LIBRARIES}")
		set(SFML_GRAPHICS_LIB "${SFML_GRAPHICS_LIBRARY}")
		set(SFML_AUDIO_LIB "${SFML_AUDIO_LIBRARY}")
		set(SFML_SYSTEM_LIB "${SFML_SYSTEM_LIBRARY}")
		set(SFML_WINDOW_LIB "${SFML_WINDOW_LIBRARY}")
		set(SFML_NETWORK_LIB "${SFML_NETWORK_LIBRARY}")
	endif ()
else ()
	set(TGUI_LIB "${TGUI_LIBRARIES}")
	set(SFML_GRAPHICS_LIB "${SFML_GRAPHICS_LIBRARY}")
	set(SFML_AUDIO_LIB "${SFML_AUDIO_LIBRARY}")
	set(SFML_SYSTEM_LIB "${SFML_SYSTEM_LIBRARY}")
	set(SFML_WINDOW_LIB "${SFML_WINDOW_LIBRARY}")
	set(SFML_NETWORK_LIB "${SFML_NETWORK_LIBRARY}")
endif ()

add_library(
	SoFGVCore SHARED
	src/Core/Data/Color.hpp
	src/Core/Data/Vector.hpp
	src/Core/Logger.cpp
	src/Core/Logger.hpp
	src/Core/Objects/IObject.hpp
	src/Core/Resources/FrameData.cpp
	src/Core/Resources/FrameData.hpp
	src/Core/Resources/Game.cpp
	src/Core/Resources/Game.hpp
	src/Core/Resources/Screen.cpp
	src/Core/Resources/Screen.hpp
	src/Core/Resources/SoundManager.cpp
	src/Core/Resources/SoundManager.hpp
	src/Core/Resources/Sprite.hpp
	src/Core/Resources/TextureManager.cpp
	src/Core/Resources/TextureManager.hpp
	src/Core/Utils.cpp
	src/Core/Utils.hpp
	src/Core/Resources/version.h
)
target_compile_definitions(SoFGVCore PRIVATE MYDLL_EXPORTS=1)

if (WIN32)
	target_link_libraries(
		SoFGVCore
		dbghelp
		${SFML_GRAPHICS_LIB}
		${SFML_AUDIO_LIB}
		${SFML_SYSTEM_LIB}
		${SFML_WINDOW_LIB}
		${SFML_NETWORK_LIB}
		${TGUI_LIB}
		ws2_32
	)
else ()
	target_link_libraries(
		SoFGVCore
		${SFML_GRAPHICS_LIB}
		${SFML_AUDIO_LIB}
		${SFML_SYSTEM_LIB}
		${SFML_WINDOW_LIB}
		${SFML_NETWORK_LIB}
		${TGUI_LIB}
		pthread
		png
		xcb
	)
endif ()

add_executable(
	FrameDataEditor
	${FDICON}
	src/FrameDataEditor/dataEditor.cpp
	src/FrameDataEditor/EditableObject.cpp
	src/FrameDataEditor/EditableObject.hpp
	src/Core/SoFGV.hpp)
target_link_libraries(FrameDataEditor SoFGVCore shady-core)
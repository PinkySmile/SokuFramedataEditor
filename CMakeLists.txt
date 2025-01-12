cmake_minimum_required(VERSION 3.13)
project(SokuFDEditor CXX)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_compile_definitions(_USE_MATH_DEFINES=1)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/pkgs)
find_package(SFML REQUIRED)
find_package(TGUI REQUIRED)
include_directories(include ${SFML_INCLUDE_DIRS} ${TGUI_INCLUDE_DIRS} ${clip_INCLUDE_DIRS} src/Core shady-packer/src/Core SokuLib/src)

set(SHADY_ENABLE_EXES OFF)
set(SHADY_ENABLE_MODULE OFF)
set(SHADY_ENABLE_TEST OFF)
add_subdirectory(shady-packer)

if (STATIC_BUILD)
	add_compile_definitions(SFML_STATIC)
endif ()
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

message("TGUI is ${TGUI_LIB}")
message("SFML-graphics is ${SFML_GRAPHICS_LIB}")
message("SFML-audio is ${SFML_AUDIO_LIB}")
message("SFML-system is ${SFML_SYSTEM_LIB}")
message("SFML-window is ${SFML_WINDOW_LIB}")
message("SFML-network is ${SFML_NETWORK_LIB}")

if (STATIC_BUILD)
	add_library(SoFGVCore STATIC)
else ()
	add_library(SoFGVCore SHARED)
endif ()

target_sources(
	SoFGVCore PUBLIC
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
	src/Core/Resources/Sprite.hpp
	src/Core/Resources/TextureManager.cpp
	src/Core/Resources/TextureManager.hpp
	src/Core/Utils.cpp
	src/Core/Utils.hpp
	src/Core/Resources/version.h
)

target_compile_definitions(SoFGVCore PRIVATE MYDLL_EXPORTS=1)

if (WIN32)
	set(
		FINAL_LIBS
		dbghelp
		${TGUI_LIB}
		${SFML_GRAPHICS_LIB}
		${SFML_WINDOW_LIB}
		${SFML_SYSTEM_LIB}
		shady-core
	)
else ()
	set(
		FINAL_LIBS
		${TGUI_LIB}
		${SFML_GRAPHICS_LIB}
		${SFML_WINDOW_LIB}
		${SFML_SYSTEM_LIB}
		shady-core
		pthread
		png
		xcb
	)
endif ()
if (NOT STATIC_BUILD)
	target_link_libraries(SoFGVCore ${FINAL_LIBS})
endif()

add_executable(
	FrameDataEditor
	${FDICON}
	src/FrameDataEditor/dataEditor.cpp
	src/FrameDataEditor/OverlayObject.cpp
	src/FrameDataEditor/OverlayObject.hpp
	src/FrameDataEditor/EditableObject.cpp
	src/FrameDataEditor/EditableObject.hpp
	src/Core/SoFGV.hpp
	src/FrameDataEditor/ActionName.cpp
	src/FrameDataEditor/ActionName.hpp
)
if (STATIC_BUILD)
	target_link_libraries(FrameDataEditor SoFGVCore ${FINAL_LIBS} opengl32 freetype brotlidec brotlicommon bz2 gdi32 winmm -static)
else ()
	target_link_libraries(FrameDataEditor SoFGVCore)
endif ()

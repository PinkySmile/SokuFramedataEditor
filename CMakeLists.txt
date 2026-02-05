cmake_minimum_required(VERSION 3.13)
project(SokuFDEditor CXX)

set(CMAKE_CXX_STANDARD 20)

add_compile_definitions(_USE_MATH_DEFINES)

find_package(SFML 3 REQUIRED COMPONENTS Graphics Audio Network)
if (STATIC_BUILD)
	message("Building statically")
	add_compile_definitions(SFML_STATIC)
	set(TGUI_SHARED_LIBS FALSE)
else ()
	message("Building shared")
	set(TGUI_SHARED_LIBS TRUE)
endif ()
find_package(TGUI 1 REQUIRED)

set(SHADY_ENABLE_EXES OFF)
set(SHADY_ENABLE_MODULE OFF)
set(SHADY_ENABLE_TEST OFF)
add_subdirectory(shady-packer)

add_compile_definitions(USE_TGUI=1)
add_compile_definitions(USE_SFML=1)

include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/src/LibCore shady-packer/src/Core src/LibCore/SFML)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
if (ASAN)
	add_compile_options(-fsanitize=address)
	add_link_options(-fsanitize=address)
endif ()
if (TSAN)
	add_compile_options(-fsanitize=thread)
	add_link_options(-fsanitize=thread)
endif ()
if (MSAN)
	add_compile_options(-fsanitize=memory)
	add_link_options(-fsanitize=memory)
endif ()
if (UBSAN)
	add_compile_options(-fsanitize=undefined)
	add_link_options(-fsanitize=undefined)
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
		add_compile_options("-fmacro-prefix-map=${CMAKE_SOURCE_DIR}/src=src" "-ffile-prefix-map=${CMAKE_SOURCE_DIR}/src=src")
	endif ()
	add_compile_definitions(NDEBUG=1)
endif ()

if (WIN32)
	set(FDICON src/FrameDataEditor/fdeditor.rc src/FrameDataEditor/version.rc)
	enable_language(RC)
else()
	set(FDICON)
endif ()

message("${FDICON}")
add_executable(
	FrameDataEditor
	${FDICON}

	src/LibCore/Data/Vector.hpp
	src/LibCore/Logger.cpp
	src/LibCore/Logger.hpp
	src/LibCore/Resources/Assets/FrameData.cpp
	src/LibCore/Resources/Assets/FrameData.hpp
	src/LibCore/Resources/Game.cpp
	src/LibCore/Resources/Game.hpp
	src/LibCore/Resources/IScene.hpp
	src/LibCore/Resources/MSVCMacros.hpp
	src/LibCore/Resources/version.h
	src/LibCore/Utils.cpp
	src/LibCore/Utils.hpp
	src/LibCore/Resources/ReplayData.hpp
	src/LibCore/Resources/Assert.hpp
	src/LibCore/Resources/Assert.cpp
	src/LibCore/Resources/Settings.cpp
	src/LibCore/Resources/Settings.hpp
	src/LibCore/LibCore.hpp

	src/FrameDataEditor/dataEditor.cpp
	src/FrameDataEditor/EditableObject.cpp
	src/FrameDataEditor/EditableObject.hpp
	src/FrameDataEditor/FrameDataEditor.cpp
	src/FrameDataEditor/FrameDataEditor.hpp
	src/FrameDataEditor/PngLoader.cpp
	src/FrameDataEditor/PngLoader.hpp

	src/FrameDataEditor/UI/ColorPlaneWidget.cpp
	src/FrameDataEditor/UI/ColorPlaneWidget.hpp
	src/FrameDataEditor/UI/ColorSliderWidget.cpp
	src/FrameDataEditor/UI/ColorSliderWidget.hpp
	src/FrameDataEditor/UI/ColorWidgets.hpp
	src/FrameDataEditor/UI/LocalizedContainer.hpp
	src/FrameDataEditor/UI/MainWindow.cpp
	src/FrameDataEditor/UI/MainWindow.hpp
	src/FrameDataEditor/UI/PreviewWidget.cpp
	src/FrameDataEditor/UI/PreviewWidget.hpp
	src/FrameDataEditor/UI/SettingsWindow.cpp
	src/FrameDataEditor/UI/SettingsWindow.hpp
	src/FrameDataEditor/UI/ShortcutsWindow.cpp
	src/FrameDataEditor/UI/ShortcutsWindow.hpp

	src/FrameDataEditor/Operations/Operation.hpp
	src/FrameDataEditor/Operations/DummyOperation.cpp
	src/FrameDataEditor/Operations/DummyOperation.hpp
	src/FrameDataEditor/Operations/FlagOperation.hpp
	src/FrameDataEditor/Operations/BasicDataOperation.hpp
	src/FrameDataEditor/Operations/BasicData2StepOperation.hpp
	src/FrameDataEditor/Operations/BasicDataDoubleOperation.hpp
	src/FrameDataEditor/Operations/BasicDataDouble2Operation.hpp
	src/FrameDataEditor/Operations/BasicDataQuadOperation.hpp
	src/FrameDataEditor/Operations/EditBoxOperation.cpp
	src/FrameDataEditor/Operations/EditBoxOperation.hpp
	src/FrameDataEditor/Operations/ClearAttackOperation.cpp
	src/FrameDataEditor/Operations/ClearAttackOperation.hpp
	src/FrameDataEditor/Operations/EditColorOperation.cpp
	src/FrameDataEditor/Operations/EditColorOperation.hpp
	src/FrameDataEditor/Operations/CreateBlockOperation.cpp
	src/FrameDataEditor/Operations/CreateBlockOperation.hpp
	src/FrameDataEditor/Operations/CreateBoxOperation.cpp
	src/FrameDataEditor/Operations/CreateBoxOperation.hpp
	src/FrameDataEditor/Operations/CreateFrameOperation.cpp
	src/FrameDataEditor/Operations/CreateFrameOperation.hpp
	src/FrameDataEditor/Operations/CreateMoveOperation.cpp
	src/FrameDataEditor/Operations/CreateMoveOperation.hpp
	src/FrameDataEditor/Operations/CreatePaletteOperation.cpp
	src/FrameDataEditor/Operations/CreatePaletteOperation.hpp
	src/FrameDataEditor/Operations/PasteAnimDataOperation.cpp
	src/FrameDataEditor/Operations/PasteAnimDataOperation.hpp
	src/FrameDataEditor/Operations/PasteBoxDataOperation.cpp
	src/FrameDataEditor/Operations/PasteBoxDataOperation.hpp
	src/FrameDataEditor/Operations/PasteDataOperation.cpp
	src/FrameDataEditor/Operations/PasteDataOperation.hpp
	src/FrameDataEditor/Operations/RemoveBoxOperation.cpp
	src/FrameDataEditor/Operations/RemoveBoxOperation.hpp
	src/FrameDataEditor/Operations/RemoveFrameOperation.cpp
	src/FrameDataEditor/Operations/RemoveFrameOperation.hpp
	src/FrameDataEditor/Operations/RemovePaletteOperation.cpp
	src/FrameDataEditor/Operations/RemovePaletteOperation.hpp
	src/FrameDataEditor/Operations/EditSpriteOperation.cpp
	src/FrameDataEditor/Operations/EditSpriteOperation.hpp
	src/FrameDataEditor/Operations/RemoveBlockOperation.cpp
	src/FrameDataEditor/Operations/RemoveBlockOperation.hpp
	src/FrameDataEditor/Operations/RemoveMoveOperation.cpp
	src/FrameDataEditor/Operations/RemoveMoveOperation.hpp
	src/FrameDataEditor/Operations/EditColorsOperation.cpp
	src/FrameDataEditor/Operations/EditColorsOperation.hpp
	src/FrameDataEditor/Operations/FlattenCollisionBoxesOperation.cpp
	src/FrameDataEditor/Operations/FlattenCollisionBoxesOperation.hpp
	src/FrameDataEditor/Operations/FlattenPropretiesOperation.cpp
	src/FrameDataEditor/Operations/FlattenPropretiesOperation.hpp
	src/FrameDataEditor/Operations/CleanPropertiesOperation.cpp
	src/FrameDataEditor/Operations/CleanPropertiesOperation.hpp


	src/LibCore/SFML/Color.hpp
	src/LibCore/SFML/Color.cpp
	src/LibCore/SFML/Sprite.hpp
	src/LibCore/SFML/Screen.cpp
	src/LibCore/SFML/Screen.hpp
	src/LibCore/SFML/Sprite.cpp
	src/LibCore/SFML/TextureManager.cpp
	src/LibCore/SFML/TextureManager.hpp
	src/LibCore/SFML/SoundManager.cpp
	src/LibCore/SFML/SoundManager.hpp
	src/FrameDataEditor/Rectangle.cpp
	src/FrameDataEditor/Rectangle.hpp
)

set(
	COMMON_LIBS
	SFML::Graphics
	SFML::Audio
	SFML::Network
	TGUI::TGUI
	shady-core
)
if (WIN32)
	set(
		OS_LIBS
		dbghelp
		ws2_32
	)
else ()
	set(
		OS_LIBS
		pthread
		png
		xcb
		X11
	)
endif ()

if (STATIC_BUILD)
	set(
		FINAL_LIBS
		${COMMON_LIBS}
		${OS_LIBS}
		opengl32
		freetype
		brotlidec
		brotlicommon
		bz2
		gdi32
		winmm
		-static
	)
else ()
	set(
		FINAL_LIBS
		${COMMON_LIBS}
		${OS_LIBS}
	)
endif ()

target_link_libraries(FrameDataEditor ${FINAL_LIBS})
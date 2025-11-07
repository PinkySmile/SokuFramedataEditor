cmake_minimum_required(VERSION 3.13)
project(SoFGV CXX)

set(CMAKE_CXX_STANDARD 20)

add_compile_definitions(_USE_MATH_DEFINES ZLIB_CONST)

if (CMAKE_SYSTEM_NAME STREQUAL Emscripten)
	set(USE_SDL2 ON)
endif ()
if (USE_SDL2)
	find_package(SDL2 REQUIRED)

	message("SDL2 include dir is ${SDL2_INCLUDE_DIRS}")
	message("SDL2 library is ${SDL2_LIBRARY}")
	message("SDL2 image library is ${SDL2_IMAGE_LIBRARY}")

	set(EXTRA_LIBRARY_INCLUDES ${SDL2_INCLUDE_DIRS} src/LibCore/SDL2)
	set(SPECIFIC_INCLUDES_PATH src/LibCore/SDL2)
	set(
		INPUT_SRCS
		src/LibCore/SDL2/ControllerInput.cpp
		src/LibCore/SDL2/ControllerInput.hpp
		src/LibCore/SDL2/KeyboardInput.cpp
		src/LibCore/SDL2/KeyboardInput.hpp
	)
	set(
		ASSETS_SRCS
		src/LibCore/SDL2/SoundManager.cpp
		src/LibCore/SDL2/SoundManager.hpp
		src/LibCore/SDL2/TextureManager.cpp
		src/LibCore/SDL2/TextureManager.hpp
	)
	set(
		SCREEN_SRCS
		src/LibCore/SDL2/Color.hpp
		src/LibCore/SDL2/Sprite.hpp
		src/LibCore/SDL2/Screen.cpp
		src/LibCore/SDL2/Screen.hpp
	)

	find_package(SFML 3 REQUIRED COMPONENTS Graphics Audio Network)
	find_package(TGUI 1 REQUIRED)

	set(
		EXTRA_LINK_LIBRARIES
		${SDL2_LIBRARY}
		${SDL2_IMAGE_LIBRARY}
		${SFML_GRAPHICS_LIB}
		${SFML_AUDIO_LIB}
		${SFML_SYSTEM_LIB}
		${SFML_WINDOW_LIB}
		${SFML_NETWORK_LIB}
		TGUI::TGUI
	)

	set(NETWORK_CLASSES)
	set(NETWORK_EXTRA_SRC)
	set(VIRTUAL_CONTROLLER OFF)
	add_compile_definitions(USE_SDL=1)
else ()
	set(NETWORK_CLASSES IP_CLASS=sf::IpAddress SOCKET_CLASS=sf::UdpSocket)
	set(NETWORK_EXTRA_SRC)
	set(
		INPUT_SRCS
		src/LibCore/SFML/ControllerInput.cpp
		src/LibCore/SFML/ControllerInput.hpp
		src/LibCore/SFML/KeyboardInput.cpp
		src/LibCore/SFML/KeyboardInput.hpp
		src/LibCore/SFML/VirtualController.cpp
		src/LibCore/SFML/VirtualController.hpp
	)
	set(
		ASSETS_SRCS
		src/LibCore/SFML/SoundManager.cpp
		src/LibCore/SFML/SoundManager.hpp
		src/LibCore/SFML/TextureManager.cpp
		src/LibCore/SFML/TextureManager.hpp
	)
	set(
		SCREEN_SRCS
		src/LibCore/SFML/Color.hpp
		src/LibCore/SFML/Color.cpp
		src/LibCore/SFML/Sprite.hpp
		src/LibCore/SFML/Screen.cpp
		src/LibCore/SFML/Screen.hpp
		src/LibCore/SFML/Sprite.cpp
	)

	find_package(SFML 3 REQUIRED COMPONENTS Graphics Audio Network)
	set(TGUI_SHARED_LIBS TRUE)
	find_package(TGUI 1 REQUIRED)

	if (NO_TGUI)
		set(
			<EXTRA_LINK_LIBRARIES
			SFML::Graphics
			SFML::Audio
			SFML::Network
		)
	else ()
		set(
			EXTRA_LINK_LIBRARIES
			SFML::Graphics
			SFML::Audio
			SFML::Network
			TGUI::TGUI
		)
		add_compile_definitions(USE_TGUI=1)
	endif()
	add_compile_definitions(USE_SFML=1)
	set(EXTRA_LIBRARY_INCLUDES src/LibCore/SFML)
endif ()

if (VIRTUAL_CONTROLLER)
	add_compile_definitions(VIRTUAL_CONTROLLER=1)
endif ()
if (NETWORK_CLASSES)
	find_library(ZLIB_LIBRARY z NAMES zlib z REQUIRED)
	message("zlib library is ${ZLIB_LIBRARY}")
	set(
		EXTRA_LINK_LIBRARIES
		${EXTRA_LINK_LIBRARIES}
		${ZLIB_LIBRARY}
	)
	set(
		NETWORK_SRCS
		${NETWORK_EXTRA_SRC}
		src/LibCore/Inputs/NetworkInput.cpp
		src/LibCore/Inputs/NetworkInput.hpp
		src/LibCore/Resources/Network/ClientConnection.cpp
		src/LibCore/Resources/Network/ClientConnection.hpp
		src/LibCore/Resources/Network/SpectatorConnection.cpp
		src/LibCore/Resources/Network/SpectatorConnection.hpp
		src/LibCore/Resources/Network/Connection.cpp
		src/LibCore/Resources/Network/Connection.hpp
		src/LibCore/Resources/Network/Exceptions.hpp
		src/LibCore/Resources/Network/IConnection.hpp
		src/LibCore/Resources/Network/NetManager.cpp
		src/LibCore/Resources/Network/NetManager.hpp
		src/LibCore/Resources/Network/Packet.cpp
		src/LibCore/Resources/Network/Packet.hpp
		src/LibCore/Resources/Network/RollbackMachine.cpp
		src/LibCore/Resources/Network/RollbackMachine.hpp
		src/LibCore/Resources/Network/ServerConnection.cpp
		src/LibCore/Resources/Network/ServerConnection.hpp
		src/LibCore/Resources/Network/SpectatorInputManager.cpp
		src/LibCore/Resources/Network/SpectatorInputManager.hpp
		src/LibCore/Inputs/SpectatorInput.cpp
		src/LibCore/Inputs/SpectatorInput.hpp
		src/LibCore/Resources/ObjectFactory.cpp
		src/LibCore/Resources/ObjectFactory.hpp
		src/LibCore/Resources/Assert.hpp
		src/LibCore/Resources/Assert.cpp
		src/LibCore/Resources/Settings.cpp
		src/LibCore/Resources/Settings.hpp
	)
	set(
		NETWORK_EXTRA_SCENES
		src/Game/Scenes/Network/SyncTestInGame.cpp
		src/Game/Scenes/Network/SyncTestInGame.hpp
		src/Game/Scenes/Network/ClientCharacterSelect.cpp
		src/Game/Scenes/Network/ClientCharacterSelect.hpp
		src/Game/Scenes/Network/ServerCharacterSelect.cpp
		src/Game/Scenes/Network/ServerCharacterSelect.hpp
		src/Game/Scenes/Network/NetworkCharacterSelect.cpp
		src/Game/Scenes/Network/NetworkCharacterSelect.hpp
		src/Game/Scenes/Network/SpectatorCharacterSelect.cpp
		src/Game/Scenes/Network/SpectatorCharacterSelect.hpp
		src/Game/Scenes/Network/ClientInGame.cpp
		src/Game/Scenes/Network/ClientInGame.hpp
		src/Game/Scenes/Network/ServerInGame.cpp
		src/Game/Scenes/Network/ServerInGame.hpp
		src/Game/Scenes/Network/NetworkInGame.cpp
		src/Game/Scenes/Network/NetworkInGame.hpp
		src/Game/Scenes/Network/SpectatorInGame.cpp
		src/Game/Scenes/Network/SpectatorInGame.hpp
	)
	add_compile_definitions(HAS_NETWORK=1)
	add_compile_definitions(NETWORK_CLASSES)
endif ()

include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/src/LibCore ${EXTRA_LIBRARY_INCLUDES})
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

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	if (NOT CMAKE_BUILD_TYPE STREQUAL Debug)
		add_compile_options("-fmacro-prefix-map=${CMAKE_SOURCE_DIR}/src=src" "-ffile-prefix-map=${CMAKE_SOURCE_DIR}/src=src")
	endif()
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
	set(CRES src/LibCore/version.rc)
	set(GICON src/Game/game.rc src/Game/version.rc)
	set(FDICON src/FrameDataEditor/fdeditor.rc src/FrameDataEditor/version.rc)
else()
	set(CRES)
	set(GICON)
	set(FDICON)
endif()

add_library(
	SoFGVCore SHARED
	${CRES}
	src/LibCore/Data/Vector.hpp
	src/LibCore/Inputs/DelayInput.cpp
	src/LibCore/Inputs/DelayInput.hpp
	src/LibCore/Inputs/IInput.hpp
	src/LibCore/Inputs/InputEnum.hpp
	src/LibCore/Inputs/RemoteInput.cpp
	src/LibCore/Inputs/RemoteInput.hpp
	src/LibCore/Inputs/ReplayInput.cpp
	src/LibCore/Inputs/ReplayInput.hpp
	src/LibCore/Inputs/RollbackInput.cpp
	src/LibCore/Inputs/RollbackInput.hpp
	src/LibCore/Logger.cpp
	src/LibCore/Logger.hpp
	src/LibCore/Objects/Characters/Character.cpp
	src/LibCore/Objects/Characters/Character.hpp
	src/LibCore/Objects/Characters/Projectile.cpp
	src/LibCore/Objects/Characters/Projectile.hpp
	src/LibCore/Objects/Characters/Stickman/Stickman.cpp
	src/LibCore/Objects/Characters/Stickman/Stickman.hpp
	src/LibCore/Objects/Characters/SubObject.cpp
	src/LibCore/Objects/Characters/SubObject.hpp
	src/LibCore/Objects/Characters/VictoriaStar/Butterfly.cpp
	src/LibCore/Objects/Characters/VictoriaStar/Butterfly.hpp
	src/LibCore/Objects/Characters/VictoriaStar/Flower.cpp
	src/LibCore/Objects/Characters/VictoriaStar/Flower.hpp
	src/LibCore/Objects/Characters/VictoriaStar/Shadow.cpp
	src/LibCore/Objects/Characters/VictoriaStar/Shadow.hpp
	src/LibCore/Objects/Characters/VictoriaStar/Shadows/MatterShadow.cpp
	src/LibCore/Objects/Characters/VictoriaStar/Shadows/MatterShadow.hpp
	src/LibCore/Objects/Characters/VictoriaStar/Shadows/NeutralShadow.cpp
	src/LibCore/Objects/Characters/VictoriaStar/Shadows/NeutralShadow.hpp
	src/LibCore/Objects/Characters/VictoriaStar/Shadows/SpiritShadow.cpp
	src/LibCore/Objects/Characters/VictoriaStar/Shadows/SpiritShadow.hpp
	src/LibCore/Objects/Characters/VictoriaStar/Shadows/VoidShadow.cpp
	src/LibCore/Objects/Characters/VictoriaStar/Shadows/VoidShadow.hpp
	src/LibCore/Objects/Characters/VictoriaStar/VictoriaProjectile.cpp
	src/LibCore/Objects/Characters/VictoriaStar/VictoriaProjectile.hpp
	src/LibCore/Objects/Characters/VictoriaStar/VictoriaStar.cpp
	src/LibCore/Objects/Characters/VictoriaStar/VictoriaStar.hpp
	src/LibCore/Objects/FakeObject.cpp
	src/LibCore/Objects/FakeObject.hpp
	src/LibCore/Objects/IObject.hpp
	src/LibCore/Objects/Object.cpp
	src/LibCore/Objects/Object.hpp
	src/LibCore/Objects/Platform.cpp
	src/LibCore/Objects/Platform.hpp
	src/LibCore/Objects/StageObjects/Cloud.cpp
	src/LibCore/Objects/StageObjects/Cloud.hpp
	src/LibCore/Objects/StageObjects/StageObject.cpp
	src/LibCore/Objects/StageObjects/StageObject.hpp
	src/LibCore/Resources/Assets/FileManager.cpp
	src/LibCore/Resources/Assets/FileManager.hpp
	src/LibCore/Resources/Assets/FrameData.cpp
	src/LibCore/Resources/Assets/FrameData.hpp
	src/LibCore/Resources/Assets/MoveListData.cpp
	src/LibCore/Resources/Assets/MoveListData.hpp
	src/LibCore/Resources/Battle/BattleManager.cpp
	src/LibCore/Resources/Battle/BattleManager.hpp
	src/LibCore/Resources/Battle/PracticeBattleManager.cpp
	src/LibCore/Resources/Battle/PracticeBattleManager.hpp
	src/LibCore/Resources/Battle/RandomWrapper.cpp
	src/LibCore/Resources/Battle/RandomWrapper.hpp
	src/LibCore/Resources/Game.cpp
	src/LibCore/Resources/Game.hpp
	src/LibCore/Resources/IScene.hpp
	src/LibCore/Resources/MSVCMacros.hpp
	src/LibCore/Resources/SceneArgument.hpp
	src/LibCore/Resources/SceneManager.cpp
	src/LibCore/Resources/SceneManager.hpp
	src/LibCore/Resources/version.h
	src/LibCore/Utils.cpp
	src/LibCore/Utils.hpp
	src/LibCore/Objects/Particle.cpp
	src/LibCore/Objects/Particle.hpp
	src/LibCore/Objects/ParticleGenerator.cpp
	src/LibCore/Objects/ParticleGenerator.hpp
	src/LibCore/Resources/ReplayData.hpp
	src/LibCore/Objects/Characters/CharacterParams.hpp
	${INPUT_SRCS}
	${ASSETS_SRCS}
	${SCREEN_SRCS}
	${NETWORK_SRCS}
)
target_compile_definitions(SoFGVCore PRIVATE MYDLL_EXPORTS=1)
if (WIN32)
	enable_language(RC)
	set_target_properties(SoFGVCore PROPERTIES SUFFIX ".dll")
endif ()

if (WIN32)
	target_link_libraries(
		SoFGVCore
		${EXTRA_LINK_LIBRARIES}
		dbghelp
		ws2_32
	)
else ()
	target_link_libraries(
		SoFGVCore
		${EXTRA_LINK_LIBRARIES}
		pthread
		png
		xcb
	)
endif ()

message("${GICON}")
add_executable(
	SoFGV
	${GICON}
	src/Game/main.cpp
	src/Game/Scenes/MainMenu/Menu.cpp
	src/Game/Scenes/MainMenu/Menu.hpp
	src/Game/Scenes/MainMenu/MenuItem.cpp
	src/Game/Scenes/MainMenu/MenuItem.hpp
	src/Game/Scenes/MainMenu/TitleScreen.cpp
	src/Game/Scenes/MainMenu/TitleScreen.hpp
	src/Game/Scenes/LoadingScene.cpp
	src/Game/Scenes/LoadingScene.hpp
	src/Game/Scenes/CharacterSelect.cpp
	src/Game/Scenes/CharacterSelect.hpp
	src/Game/Scenes/InGame.cpp
	src/Game/Scenes/InGame.hpp
	src/Game/Scenes/PracticeInGame.cpp
	src/Game/Scenes/PracticeInGame.hpp
	src/Game/Scenes/ReplayInGame.cpp
	src/Game/Scenes/ReplayInGame.hpp
	${NETWORK_EXTRA_SCENES}
	src/LibCore/LibCore.hpp
	src/Game/Scenes/Scenes.hpp
	src/Game/Scenes/SkeletonData.hpp
)
target_link_libraries(SoFGV SoFGVCore ${EXTRA_LINK_LIBRARIES})
target_include_directories(SoFGV PRIVATE src/LibCore src/Game)

if (NOT USE_SDL2)
	if (NOT NO_TGUI)
		message("${FDICON}")
		add_executable(
			FrameDataEditor
			${FDICON}
			src/LibCore/LibCore.hpp
			src/FrameDataEditor/dataEditor.cpp
			src/FrameDataEditor/EditableObject.cpp
			src/FrameDataEditor/EditableObject.hpp
			src/FrameDataEditor/FrameDataEditor.cpp
			src/FrameDataEditor/FrameDataEditor.hpp
			src/FrameDataEditor/UI/LocalizedContainer.hpp
			src/FrameDataEditor/UI/MainWindow.cpp
			src/FrameDataEditor/UI/MainWindow.hpp
			src/FrameDataEditor/UI/PreviewWidget.cpp
			src/FrameDataEditor/UI/PreviewWidget.hpp
			src/FrameDataEditor/UI/SettingsWindow.cpp
			src/FrameDataEditor/UI/SettingsWindow.hpp
			src/FrameDataEditor/Operations/BasicDataOperation.hpp
			src/FrameDataEditor/Operations/FlagOperation.hpp
			src/FrameDataEditor/Operations/Operation.hpp
			src/FrameDataEditor/Operations/SpriteChangeOperation.cpp
			src/FrameDataEditor/Operations/SpriteChangeOperation.hpp
			src/FrameDataEditor/Operations/SoundChangeOperation.cpp
			src/FrameDataEditor/Operations/SoundChangeOperation.hpp
			src/FrameDataEditor/Operations/ClearHitOperation.cpp
			src/FrameDataEditor/Operations/ClearHitOperation.hpp
			src/FrameDataEditor/Operations/ClearBlockOperation.cpp
			src/FrameDataEditor/Operations/ClearBlockOperation.hpp
			src/FrameDataEditor/Operations/DummyOperation.cpp
			src/FrameDataEditor/Operations/DummyOperation.hpp
			src/FrameDataEditor/Operations/CreateMoveOperation.cpp
			src/FrameDataEditor/Operations/CreateMoveOperation.hpp
			src/FrameDataEditor/Operations/CreateFrameOperation.cpp
			src/FrameDataEditor/Operations/CreateFrameOperation.hpp
			src/FrameDataEditor/Operations/BoxModificationOperation.cpp
			src/FrameDataEditor/Operations/BoxModificationOperation.hpp
			src/FrameDataEditor/Operations/CreateBlockOperation.cpp
			src/FrameDataEditor/Operations/CreateBlockOperation.hpp
			src/FrameDataEditor/Operations/CreateBoxOperation.cpp
			src/FrameDataEditor/Operations/CreateBoxOperation.hpp
			src/FrameDataEditor/Operations/RemoveFrameOperation.cpp
			src/FrameDataEditor/Operations/RemoveFrameOperation.hpp
			src/FrameDataEditor/Operations/RemoveBoxOperation.cpp
			src/FrameDataEditor/Operations/RemoveBoxOperation.hpp
			src/FrameDataEditor/UI/ColorPlaneWidget.cpp
			src/FrameDataEditor/UI/ColorPlaneWidget.hpp
			src/FrameDataEditor/UI/ColorWidgets.hpp
			src/FrameDataEditor/UI/ColorSliderWidget.cpp
			src/FrameDataEditor/UI/ColorSliderWidget.hpp
			src/FrameDataEditor/PngLoader.cpp
			src/FrameDataEditor/PngLoader.hpp
			src/FrameDataEditor/Operations/ColorEditionOperation.cpp
			src/FrameDataEditor/Operations/ColorEditionOperation.hpp
			src/FrameDataEditor/Operations/CreatePaletteOperation.cpp
			src/FrameDataEditor/Operations/CreatePaletteOperation.hpp
			src/FrameDataEditor/Operations/RemovePaletteOperation.cpp
			src/FrameDataEditor/Operations/RemovePaletteOperation.hpp
		)
		target_link_libraries(FrameDataEditor SoFGVCore png)
	endif ()

	add_executable(FrameComparer src/FrameComparer/main.cpp)
	target_link_libraries(
		FrameComparer
		SoFGVCore
	)

	add_executable(PaletteExtractor src/PaletteExtractor/main.cpp)
	target_link_libraries(
		PaletteExtractor
		SFML::Graphics
	)

	add_executable(LagSwitch src/LagSwitch/main.cpp)
	target_link_libraries(
		LagSwitch
		SFML::Network
	)
	target_include_directories(LagSwitch PRIVATE src/LibCore)
endif ()
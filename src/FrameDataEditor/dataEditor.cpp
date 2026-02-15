#include <iostream>
#ifdef __linux__
#include <pwd.h>
#endif
#include "LibCore.hpp"
#include "FrameDataEditor.hpp"

using namespace SpiralOfFate;

FrameDataEditor *editor;

void run()
{
	while (game->screen->isOpen()) {
		try {
			editor->update();
			game->screen->clear(sf::Color{0x40, 0x40, 0x40, 0xFF});
			editor->render();
			game->gui.draw();
			game->screen->display();
			while (auto event = game->screen->pollEvent()) {
				if (event->is<EVENT_WINDOW_CLOSED>())
					editor->closeAll();
				if (auto key = event->getIf<sf::Event::KeyPressed>()) {
					if (editor->canHandleKeyPress(*key)) {
						editor->keyPressed(*key);
						continue;
					}
				}
				if (auto key = event->getIf<sf::Event::KeyReleased>()) {
					editor->keyReleased(*key);
				}
				if (auto mouse = event->getIf<sf::Event::MouseMoved>())
					editor->mouseMovedAbsolute(tgui::Vector2f(mouse->position.x, mouse->position.y));
				game->gui.handleEvent(*event);
			}
		} catch (std::exception &e) {
			Utils::dispMsg(game->gui, "Error", std::string("Uncaught exception in main loop: ") + e.what(), MB_ICONERROR);
		}
	}
}

void initEditor()
{
	sf::Image icon;

#ifdef __linux__
	std::filesystem::path home = getenv("HOME") ?: getpwuid(getuid())->pw_dir;
	const char *tmp = getenv("XDG_CONFIG_HOME");
	std::filesystem::path config = tmp ? std::filesystem::path(tmp) : home / ".config" / "th123fde";
	tmp = getenv("XDG_DATA_HOME");
	std::filesystem::path data = tmp ? std::filesystem::path(tmp) : home / ".local" / "share" / "th123fde";
	tmp = getenv("XDG_STATE_HOME");
	std::filesystem::path state = tmp ? std::filesystem::path(tmp) : home / ".local" / "state" / "th123fde";

	std::filesystem::create_directories(config);
	std::filesystem::create_directories(data);
	std::filesystem::create_directories(state);
	new Game(config, data, state, config / "settings.json", state / "editor.log");
#else
	new Game(".", ".", ".", "settings.json", "./editor.log");
#endif
	game->logger.info("Starting editor.");
	game->screen = std::make_unique<Screen>("Touhou 12.3: Hisoutensoku | FrameData Editor");
	if (icon.loadFromFile("assets/editorIcon.png"))
		game->screen->setIcon(icon.getSize(), icon.getPixelsPtr());
	game->screen->setFramerateLimit(240);
	game->gui.setWindow(*game->screen);
	editor = new FrameDataEditor();
}

int main()
{
#ifdef _DEBUG
	if (Utils::isBeingDebugged()) {
		initEditor();
		run();
		game->logger.info("Goodbye !");
		delete editor;
		delete game;
		return EXIT_SUCCESS;
	}
#endif
	try {
		initEditor();
		run();
	} catch (std::exception &e) {
		if (game) {
			game->logger.fatal(e.what());
			delete game;
		} else
			std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	game->logger.info("Goodbye !");
	delete editor;
	delete game;
	return EXIT_SUCCESS;
}
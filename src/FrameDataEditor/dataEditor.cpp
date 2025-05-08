#include "LibCore.hpp"
#include "FrameDataEditor.hpp"

using namespace SpiralOfFate;

FrameDataEditor *editor;

void run()
{
	while (game->screen->isOpen()) {
		editor->update();
		game->screen->clear(sf::Color{0x40, 0x40, 0x40, 0xFF});
		editor->render();
		game->gui.draw();
		game->screen->display();
		while (auto event = game->screen->pollEvent()) {
			if (event->is<EVENT_WINDOW_CLOSED>())
				game->screen->close();
			game->gui.handleEvent(*event);
		}
	}
}

void initEditor()
{
	sf::Image icon;

	new Game("assets/fonts/Retro Gaming.ttf", "settings.json", "./editor.log");
	game->logger.info("Starting editor.");
	game->screen = std::make_unique<Screen>("Spiral of Fate: Grand Vision | FrameData Editor");
	if (icon.loadFromFile("assets/editorIcon.png"))
		game->screen->setIcon(icon.getSize(), icon.getPixelsPtr());
	game->screen->setFramerateLimit(240);
	game->gui.setWindow(*game->screen);
	editor = new FrameDataEditor();
}

int main()
{
	try {
		initEditor();
		run();
	} catch (std::exception &e) {
		if (game)
			game->logger.fatal(e.what());
		delete game;
		return EXIT_FAILURE;
	}
	game->logger.info("Goodbye !");
	delete editor;
	delete game;
	return EXIT_SUCCESS;
}
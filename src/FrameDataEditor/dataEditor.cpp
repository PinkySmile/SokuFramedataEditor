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
			if (event->is<EVENT_WINDOW_CLOSED>()) {
				if (editor->closeAll())
					game->screen->close();
				else
					Utils::dispMsg(game->gui, editor->localize("message_box.title.not_saved"), editor->localize("message_box.not_saved"), MB_ICONINFORMATION);
			}
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

#ifndef _WIN32
#include <sys/types.h>
#include <sys/ptrace.h>

// https://forum.juce.com/t/detecting-if-a-process-is-being-run-under-a-debugger/2098
bool runningUnderDebugger()
{
	static bool isCheckedAlready = false;
	static bool underDebugger = false;

	if (!isCheckedAlready) {
		if (ptrace(PTRACE_TRACEME, 0, 1, 0) < 0)
			underDebugger = true;
		else
			ptrace(PTRACE_DETACH, 0, 1, 0);
		isCheckedAlready = true;
	}
	return underDebugger;
}
#else
bool runningUnderDebugger()
{
     return IsDebuggerPresent() == TRUE;
}
#endif

int main()
{
#ifdef _DEBUG
	if (runningUnderDebugger()) {
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
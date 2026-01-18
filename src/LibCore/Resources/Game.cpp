//
// Created by PinkySmile on 18/09/2021.
//

#include <memory>
#include "Game.hpp"
#include "Assert.hpp"
#include "Logger.hpp"

namespace SpiralOfFate
{
	MYDLL_API Game *game = nullptr;

	Game::Game(const std::filesystem::path &fontPath, const std::filesystem::path &settingsPath, const std::filesystem::path &loggerPath) :
		settings(settingsPath),
		logger(loggerPath),
		font(fontPath)
	{
		assert_exp(!game);
		game = this;
		try {
			tgui::Theme::setDefault(tgui::Theme::create(this->settings.theme));
		} catch (std::exception &) {
			game = nullptr;
			throw;
		}
	}
}


namespace ShadyCore {
	void* Allocate(size_t s) { return new char[s]; }
	void Deallocate(void* p) { delete[] static_cast<char *>(p); }
}
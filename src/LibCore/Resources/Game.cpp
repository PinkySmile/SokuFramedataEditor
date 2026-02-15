//
// Created by PinkySmile on 18/09/2021.
//

#include <memory>
#include "Game.hpp"

#include <ranges>

#include "Assert.hpp"
#include "Logger.hpp"

namespace SpiralOfFate
{
	MYDLL_API Game *game = nullptr;

	Game::Game(
		const std::filesystem::path &config,
		const std::filesystem::path &data,
		const std::filesystem::path &state,
		const std::filesystem::path &settingsPath,
		const std::filesystem::path &loggerPath
	) :
		config(config),
		data(data),
		state(state),
		settings(data, settingsPath),
		logger(loggerPath)
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

	Game::~Game()
	{
		for (auto id : this->soundEffects | std::ranges::views::values)
			this->soundMgr.remove(id);
	}

	void Game::reloadSounds()
	{
		char buffer[] = "data/se/000.wav";

		for (auto id : this->soundEffects | std::ranges::views::values)
			this->soundMgr.remove(id);
		this->soundEffects.clear();
		for (unsigned i = 0; i < 256; i++) {
			sprintf(buffer, "data/se/%03u.wav", i);

			auto id = this->soundMgr.load(buffer);
			if (id == 0)
				continue;
			this->soundEffects[i] = id;
		}
	}
}


namespace ShadyCore {
	void* Allocate(size_t s) { return new char[s]; }
	void Deallocate(void* p) { delete[] static_cast<char *>(p); }
}
//
// Created by PinkySmile on 18/09/2021
//

#include "Game.hpp"
#include <sstream>
#include <memory>
#include <iomanip>
#include "../Logger.hpp"

namespace SpiralOfFate
{
	MYDLL_API Game *game = nullptr;

	Game::Game(const std::string &loggerPath) :
		random(std::random_device()()),
		logger(loggerPath)
	{
		my_assert(!game);
		game = this;
	}

	Game::~Game()
	{
		this->logger.debug("~Game()>");
		this->logger.debug("~Game()<");
	}
}

namespace ShadyCore {
	void* Allocate(size_t s) { return new char[s]; }
	void Deallocate(void* p) { delete[] (char *)p; }
}

//
// Created by PinkySmile on 18/09/2021.
//

#ifndef SOFGV_GAME_HPP
#define SOFGV_GAME_HPP


#include <mutex>
#include <memory>
#include <TGUI/TGUI.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>
#include <package.hpp>
#include "Settings.hpp"
#include "Screen.hpp"
#include "TextureManager.hpp"
#include "Logger.hpp"
#include "MSVCMacros.hpp"

#define random_distrib(r, mi, ma) ((((uint64_t)(r)() - (r).min()) * ((ma) - (mi))) / ((r).max() - (r).min()) + (mi))

namespace SpiralOfFate
{
	struct Game {
		Settings settings;
		Logger logger;
		sf::Font font;
		std::unique_ptr<Screen> screen;
		ShadyCore::PackageEx package;
		TextureManager textureMgr;
		tgui::Gui gui;
		unsigned lastSwap = 0;

		Game(const std::filesystem::path &fontPath, const std::filesystem::path &settingsPath, const std::filesystem::path &loggerPath = "./latest.log");
	};
	extern MYDLL_API Game *game;
}


#endif //SOFGV_GAME_HPP

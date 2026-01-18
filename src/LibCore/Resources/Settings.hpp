//
// Created by PinkySmile on 08/05/25.
//

#ifndef SOFGV_SETTINGS_HPP
#define SOFGV_SETTINGS_HPP


#include <filesystem>
#include <string>

namespace SpiralOfFate
{
	class Settings {
	private:
		std::filesystem::path _path;

	public:
		std::filesystem::path palettes;
		std::string theme;
		std::filesystem::path swr;
		std::filesystem::path soku;
		std::filesystem::path soku2;
		std::vector<std::filesystem::path> extra;

		Settings(const std::filesystem::path &path);
		~Settings();
		void save();
	};
}


#endif //SOFGV_SETTINGS_HPP

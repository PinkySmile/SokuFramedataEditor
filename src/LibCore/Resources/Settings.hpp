//
// Created by PinkySmile on 08/05/25.
//

#ifndef SOFGV_SETTINGS_HPP
#define SOFGV_SETTINGS_HPP


#include <string>

namespace SpiralOfFate
{
	class Settings {
	private:
		std::string _path;

	public:
		std::string inputPresetP1;
		std::string inputPresetP2;
		std::string theme;

		Settings(const std::string &path);
		~Settings();
		void save();
	};
}


#endif //SOFGV_SETTINGS_HPP

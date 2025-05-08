//
// Created by PinkySmile on 08/05/25.
//

#include <fstream>
#include <nlohmann/json.hpp>
#include "Settings.hpp"
#include "Utils.hpp"

SpiralOfFate::Settings::Settings(const std::string &path) :
	_path(path)
{
	std::ifstream stream{path};
	nlohmann::json json;

	if (!stream.fail()) {
		stream >> json;
		this->inputPresetP1 = json["inputPresetP1"];
		this->inputPresetP2 = json["inputPresetP2"];
		this->theme = json["theme"];
	} else if (errno != ENOENT)
		throw std::runtime_error("Cannot open settings file: " + this->_path + ": " + std::string(strerror(errno)));
	else {
		this->inputPresetP1 = "inputs/default1.in";
		this->inputPresetP2 = "inputs/default2.in";
		this->theme = "assets/gui/themes/Black.style";
	}
}

SpiralOfFate::Settings::~Settings()
{
	this->save();
}

void SpiralOfFate::Settings::save()
{
	std::ofstream stream{this->_path};
	nlohmann::json json = {
		{ "inputPresetP1", this->inputPresetP1 },
		{ "inputPresetP2", this->inputPresetP2 },
		{ "theme", this->theme },
	};

	if (!stream)
		throw std::runtime_error("Cannot open settings file: " + this->_path + ": " + std::string(strerror(errno)));
	stream << json.dump(4);
}

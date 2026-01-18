//
// Created by PinkySmile on 08/05/25.
//

#include <fstream>
#include <nlohmann/json.hpp>
#include "Settings.hpp"
#include "Utils.hpp"

SpiralOfFate::Settings::Settings(const std::filesystem::path &path) :
	_path(path)
{
	std::ifstream stream{path};
	nlohmann::json json;

	if (!stream.fail()) {
		stream >> json;
		try {
			this->palettes = json["palettes"].get<std::filesystem::path>();
			this->theme = json["theme"];
			this->swr = json["swr"].get<std::filesystem::path>();
			this->soku = json["soku"].get<std::filesystem::path>();
			this->soku2 = json["soku2"].get<std::filesystem::path>();
			this->extra = json["extra"].get<std::vector<std::filesystem::path>>();
			return;
		} catch (...) {}
	} else if (errno != ENOENT)
		throw std::runtime_error("Cannot open settings file: " + this->_path.string() + ": " + std::string(strerror(errno)));
	this->theme = "assets/gui/themes/Black.style";
	this->palettes = "palettes";
}

SpiralOfFate::Settings::~Settings()
{
	this->save();
}

void SpiralOfFate::Settings::save()
{
	std::ofstream stream{this->_path};
	nlohmann::json json = {
		{ "palettes", this->palettes },
		{ "theme", this->theme },
		{ "swr", this->swr },
		{ "soku", this->soku },
		{ "soku2", this->soku2 },
		{ "extra", this->extra },
	};

	if (!stream)
		throw std::runtime_error("Cannot open settings file: " + this->_path.string() + ": " + std::string(strerror(errno)));
	stream << json.dump(4);
}

//
// Created by PinkySmile on 08/05/25.
//

#include <fstream>
#include <nlohmann/json.hpp>
#include "Settings.hpp"
#include "Utils.hpp"

SpiralOfFate::Settings::Settings(const std::filesystem::path &data, const std::filesystem::path &path) :
	_path(path)
{
	std::ifstream stream{path};
	nlohmann::json json;

	if (!stream.fail()) {
		stream >> json;
		try {
			this->palettes = json["palettes"].get<std::filesystem::path::string_type>();
			this->theme = json["theme"];
			this->swr = json["swr"].get<std::filesystem::path::string_type>();
			this->soku = json["soku"].get<std::filesystem::path::string_type>();
			this->soku2 = json["soku2"].get<std::filesystem::path::string_type>();
			this->extra.clear();
			for (const auto &e : json["extra"].get<std::vector<std::filesystem::path::string_type>>())
				this->extra.emplace_back(e);
			return;
		} catch (...) {}
	} else if (errno != ENOENT)
		throw std::runtime_error("Cannot open settings file: " + this->_path.string() + ": " + std::string(strerror(errno)));
	this->theme = "assets/gui/themes/Black.style";
	this->palettes = data / "palettes";
}

SpiralOfFate::Settings::~Settings()
{
	this->save();
}

void SpiralOfFate::Settings::save()
{
	nlohmann::json json = {
		{ "palettes", this->palettes.native() },
		{ "theme", this->theme },
		{ "swr", this->swr.native() },
		{ "soku", this->soku.native() },
		{ "soku2", this->soku2.native() },
		{ "extra", nlohmann::json::array() },
	};

	for (auto &e : this->extra)
		json["extra"].emplace_back(e.native());

	std::ofstream stream{this->_path};

	if (!stream)
		throw std::runtime_error("Cannot open settings file: " + this->_path.string() + ": " + std::string(strerror(errno)));
	stream << json.dump(4);
}

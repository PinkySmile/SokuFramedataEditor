//
// Created by PinkySmile on 27/10/2025.
//

#include <fstream>
#include <cstring>
#include "PngLoader.hpp"
#include "Resources/Assert.hpp"
#include "Resources/Game.hpp"

const LoadedImage &PNGLoader::loadImage(const std::string &path)
{
	auto it = this->_cache.find(path);

	if (it != this->_cache.end())
		return *it->second;
	this->_cache.emplace(path, std::make_unique<LoadedImage>());

	auto &img = *this->_cache.at(path);
	auto entry = SpiralOfFate::game->package.find(path, ShadyCore::FileType::TYPE_IMAGE);

	if (entry == SpiralOfFate::game->package.end()) {
		SpiralOfFate::game->logger.error("Failed to load " + path);
		return img;
	}

	auto &stream = entry.open();

	ShadyCore::getResourceReader(entry.fileType())(&img, stream);
	entry.close(stream);
	return img;
}

PNGLoader pngLoader;
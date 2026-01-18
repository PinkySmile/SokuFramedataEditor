//
// Created by PinkySmile on 27/10/2025.
//

#ifndef SOFGV_PNGLOADER_HPP
#define SOFGV_PNGLOADER_HPP


#include <memory>
#include <unordered_map>
#include <string>
#include <array>
#include <filesystem>
#include <png.h>
#include "Color.hpp"
#include "resource.hpp"

using LoadedImage = ShadyCore::Image;

class PNGLoader {
public:
	const LoadedImage &loadImage(const std::string &path);

private:
	std::unordered_map<std::string, std::unique_ptr<LoadedImage>> _cache;
};

extern PNGLoader pngLoader;


#endif //SOFGV_PNGLOADER_HPP

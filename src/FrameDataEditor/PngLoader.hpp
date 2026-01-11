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

struct LoadedImage {
	unsigned char bitsPerPixel = 0;
	unsigned width;
	unsigned height;
	unsigned paddedWidth;
	unsigned altSize = 0;
	unsigned bufferSize = 0;
	unsigned char *raw = nullptr;
	std::array<SpiralOfFate::Color, 256> palette;

	LoadedImage() = default;
	LoadedImage(const LoadedImage &) = delete;
	~LoadedImage();

	void alloc();
};

class PNGLoader {
public:
	const LoadedImage &loadImage(const std::filesystem::path &path);

private:
	std::unordered_map<std::filesystem::path, std::unique_ptr<LoadedImage>> _cache;
};

extern PNGLoader pngLoader;


#endif //SOFGV_PNGLOADER_HPP

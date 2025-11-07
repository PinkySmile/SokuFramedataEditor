//
// Created by PinkySmile on 27/10/2025.
//

#include <fstream>
#include <cstring>
#include "PngLoader.hpp"
#include "Resources/Assert.hpp"

LoadedImage::~LoadedImage()
{
	delete[] this->raw;
}

void LoadedImage::alloc()
{
	this->bufferSize = this->altSize ? this->altSize : this->paddedWidth * this->height * (this->bitsPerPixel == 24 ? 4 : this->bitsPerPixel / 8);
	this->raw = new unsigned char[this->bufferSize];
}

static void pngError(png_structp, png_const_charp message)
{
	fprintf(stderr, "%s\n", message);
	throw std::runtime_error(message);
}

static void pngWarning(png_structp, png_const_charp message)
{
	printf("%s\n", message);
}

static void pngRead(png_structp pngData, png_bytep buffer, png_size_t length)
{
	auto *input = (std::istream *)png_get_io_ptr(pngData);

	input->read((char*)buffer, length);
}

const LoadedImage &PNGLoader::loadImage(const std::string &path)
{
	auto it = this->_cache.find(path);

	if (it != this->_cache.end())
		return *it->second;
	this->_cache.emplace(path, std::make_unique<LoadedImage>());

	auto &img = *this->_cache.at(path);
	std::ifstream input{path, std::istream::binary};

	if (input.fail())
		return img;

	png_structp pngData = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, pngError, pngWarning);
	png_infop pngInfo = png_create_info_struct(pngData);
	png_set_read_fn(pngData, &input, pngRead);
	png_read_info(pngData, pngInfo);

	// Read Info
	img.width = png_get_image_width(pngData, pngInfo);
	img.height = png_get_image_height(pngData, pngInfo);

	// Register transformations
	int depth = png_get_bit_depth(pngData, pngInfo);

	if (depth > 8)
		png_set_strip_16(pngData);
	if (depth < 8)
		png_set_packing(pngData);

	int colorType = png_get_color_type(pngData, pngInfo);
	png_colorp palette = nullptr;
	int numPalettes;

	if (colorType & PNG_COLOR_MASK_PALETTE) {
		img.bitsPerPixel = 8;
		png_get_PLTE(pngData, pngInfo, &palette, &numPalettes);
		for (int i = 0; i < 256 && i < numPalettes; i++) {
			img.palette[i].r = palette[i].red;
			img.palette[i].g = palette[i].green;
			img.palette[i].b = palette[i].blue;
			img.palette[i].a = 255;
		}
		for (int i = numPalettes; i < 256; i++) {
			img.palette[i].r = 0;
			img.palette[i].g = 0;
			img.palette[i].b = 0;
			img.palette[i].a = 255;
		}
	} else {
		img.bitsPerPixel = colorType & PNG_COLOR_MASK_ALPHA ? 32 : 24;
		if (colorType == PNG_COLOR_TYPE_GRAY || colorType == PNG_COLOR_TYPE_GRAY_ALPHA)
			png_set_gray_to_rgb(pngData);
		if (!(colorType & PNG_COLOR_MASK_ALPHA))
			png_set_filler(pngData, 0xff, PNG_FILLER_AFTER);
		png_set_bgr(pngData);
	}
	png_read_update_info(pngData, pngInfo);
	img.paddedWidth = png_get_rowbytes(pngData, pngInfo);
	img.altSize = 0;
	img.alloc();
	memset(img.raw, 0, img.bufferSize);

	// Read Image
	auto pointers = new unsigned char *[img.height];

	for (size_t i = 0; i < img.height; ++i)
		pointers[i] = img.raw + i * img.paddedWidth;
	png_read_image(pngData, (png_bytepp)pointers);
	delete[] pointers;

	// Finish
	png_read_end(pngData, nullptr);
	png_destroy_read_struct(&pngData, &pngInfo, nullptr);
	return img;
}

PNGLoader pngLoader;
//
// Created by PinkySmile on 18/09/2021
//

#include "TextureManager.hpp"
#include "Game.hpp"
#include "../Logger.hpp"

namespace SpiralOfFate
{
	unsigned TextureManager::load(ShadyCore::PackageEx &package, const std::string &file, Vector2u *size)
	{
		if (this->_allocatedTextures[file].second != 0) {
			this->_allocatedTextures[file].second++;
			game->logger.debug("Returning already loaded file " + file);
			if (size)
				*size = this->_textures[this->_allocatedTextures[file].first].getSize();
			return this->_allocatedTextures[file].first;
		}
		game->logger.debug("Loading file " + file);

		auto entry = package.find(file, ShadyCore::FileType::TYPE_IMAGE);

		if (entry == package.end()) {
			game->logger.error("Could not find image " + file);
			return 0;
		}

		auto imageEntry = this->_loadedImages.find(file);
		ShadyCore::Image &resourceImage = this->_loadedImages[file];
		sf::Image image;
		unsigned index;

		if (imageEntry == this->_loadedImages.end()) {
			ShadyCore::getResourceReader(entry.fileType())(&resourceImage, entry.open());
			entry.close();
		}
		image.create(resourceImage.width, resourceImage.height, resourceImage.raw);

		if (this->_freedIndexes.empty()) {
			this->_lastIndex += this->_lastIndex == 0;
			index = this->_lastIndex;
			this->_lastIndex++;
		} else {
			index = this->_freedIndexes.back();
			this->_freedIndexes.pop_back();
		}

		if (!this->_textures[index].loadFromImage(image)) {
			game->logger.error("Failed to load texture " + file);
			this->_freedIndexes.push_back(index);
			return 0;
		}

		if (size)
			*size = this->_textures[index].getSize();

		this->_textures[index].setRepeated(true);
		this->_allocatedTextures[file].first = index;
		this->_allocatedTextures[file].second = 1;
		return index;
	}

	unsigned TextureManager::load(ShadyCore::PackageEx &package, const ShadyCore::Palette &palette, const std::string &palName, const std::string &file, Vector2u *size)
	{
		if (this->_allocatedTextures[palName + "|" + file].second != 0) {
			this->_allocatedTextures[palName + "|" + file].second++;
			game->logger.debug("Returning already loaded file " + file);
			if (size)
				*size = this->_textures[this->_allocatedTextures[palName + "|" + file].first].getSize();
			return this->_allocatedTextures[palName + "|" + file].first;
		}
		game->logger.debug("Loading file " + file + " with palette " + palName);

		auto entry = package.find(file, ShadyCore::FileType::TYPE_IMAGE);

		if (entry == package.end()) {
			game->logger.error("Could not find image " + file);
			return 0;
		}

		auto imageEntry = this->_loadedImages.find(file);
		ShadyCore::Image &resourceImage = this->_loadedImages[file];
		sf::Image image;
		unsigned index;

		if (imageEntry == this->_loadedImages.end()) {
			ShadyCore::getResourceReader(entry.fileType())(&resourceImage, entry.open());
			entry.close();
		}
		if (resourceImage.bitsPerPixel == 8) {
			image.create(resourceImage.width, resourceImage.height);
			for (unsigned i = 0; i < resourceImage.getRawSize(); ++i) {
				unsigned x = i % resourceImage.paddedWidth;
				unsigned y = i / resourceImage.paddedWidth;

				if (x >= resourceImage.width)
					continue;
				if (y >= resourceImage.height)
					continue;

				auto val = ((uint16_t*)palette.data)[resourceImage.raw[i]];
				sf::Color color{
					static_cast<sf::Uint8>((val & 0b0111110000000000) >> 7 | (val & 0b0111000000000000) >> 12),
					static_cast<sf::Uint8>((val & 0b0000001111100000) >> 2 | (val & 0b0000001110000000) >> 7),
					static_cast<sf::Uint8>((val & 0b0000000000011111) << 3 | (val & 0b0000000000011100) >> 2),
					static_cast<sf::Uint8>((val & 0x8000) ? 255 : 0)
				};

				image.setPixel(x, y, color);
			}
		} else {
			auto raw = new sf::Color[resourceImage.width * resourceImage.height];

			for (unsigned x = 0; x < resourceImage.width; x++)
				for (unsigned y = 0; y < resourceImage.height; y++)
					raw[y * resourceImage.width + x] = sf::Color{
						resourceImage.raw[(y * resourceImage.paddedWidth + x) * 4 + 2],
						resourceImage.raw[(y * resourceImage.paddedWidth + x) * 4 + 1],
						resourceImage.raw[(y * resourceImage.paddedWidth + x) * 4 + 0],
						resourceImage.raw[(y * resourceImage.paddedWidth + x) * 4 + 3]
					};
			image.create(resourceImage.width, resourceImage.height, reinterpret_cast<const sf::Uint8 *>(raw));
			delete[] raw;
		}

		if (this->_freedIndexes.empty()) {
			this->_lastIndex += this->_lastIndex == 0;
			index = this->_lastIndex;
			this->_lastIndex++;
		} else {
			index = this->_freedIndexes.back();
			this->_freedIndexes.pop_back();
		}

		if (!this->_textures[index].loadFromImage(image)) {
			game->logger.error("Failed to load texture " + file);
			this->_freedIndexes.push_back(index);
			return 0;
		}

		if (size)
			*size = this->_textures[index].getSize();

		this->_textures[index].setRepeated(true);
		this->_allocatedTextures[palName + "|" + file].first = index;
		this->_allocatedTextures[palName + "|" + file].second = 1;
		return index;
	}

	static std::string colorToString(Color color)
	{
		char buffer[10];

		color.a = 0;
		sprintf(buffer, "#%06x", color.value);
		return buffer;
	}

	void TextureManager::remove(unsigned int id)
	{
		if (!id)
			return;

		for (auto &[loadedPath, attr] : this->_allocatedTextures)
			if (attr.first == id && attr.second) {
				attr.second--;
				if (attr.second) {
					game->logger.debug("Remove ref to " + loadedPath);
					return;
				}
				game->logger.debug("Destroying texture " + loadedPath);
				break;
			}

		auto it = this->_textures.find(id);

		my_assert(it != this->_textures.end());
		this->_textures.erase(it);
		this->_freedIndexes.push_back(id);
	}

	void TextureManager::render(Sprite &sprite) const
	{
		if (!sprite.textureHandle)
			return;
		sprite.setTexture(this->_textures.at(sprite.textureHandle));
		game->screen->displayElement(sprite);
	}

	void TextureManager::setTexture(Sprite &sprite) const
	{
		if (!sprite.textureHandle) {
			sprite.setTexture(*(sf::Texture *)nullptr);
			return;
		}
		sprite.setTexture(this->_textures.at(sprite.textureHandle));
	}

	void TextureManager::addRef(unsigned int id)
	{
		if (id == 0)
			return;
		for (auto &[loadedPath, attr] : this->_allocatedTextures)
			if (attr.first == id && attr.second) {
				attr.second++;
				my_assert(attr.second > 1);
				game->logger.debug("Adding ref to " + loadedPath);
				return;
			}
	}

	Vector2u TextureManager::getTextureSize(unsigned int id) const
	{
		if (id == 0)
			return {0, 0};
		return this->_textures.at(id).getSize();
	}

	TextureManager::~TextureManager()
	{
		game->logger.debug("~TextureManager()");
	}

	const ShadyCore::Image &TextureManager::getUnderlyingImage(const std::string &file)
	{
		return this->_loadedImages.at(file);
	}

	void TextureManager::invalidatePalette(const std::string &palName)
	{
		std::string name = palName + "|";
		std::vector<std::string> keys;

		for (auto &[loadedPath, attr] : this->_allocatedTextures)
			if (loadedPath.substr(0, name.size()) == name)
				keys.push_back(loadedPath);
		for (auto &key : keys) {
			auto node = this->_allocatedTextures.extract(key);

			node.key() = "INVALID" + node.key();
			this->_allocatedTextures.insert(std::move(node));
		}
	}
}

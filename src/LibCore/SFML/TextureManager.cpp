//
// Created by PinkySmile on 18/09/2021.
//

#include "TextureManager.hpp"
#include "Resources/Game.hpp"
#include "Logger.hpp"
#include "Resources/Assert.hpp"

namespace SpiralOfFate
{
	TextureManager::TextureManager()
	{
		this->_dummy.clear(Color::Transparent);
		this->_dummy.display();
	}

	static std::string colorToString(Color color)
	{
		char buffer[10];

		color.a = 0;
		sprintf(buffer, "#%06x", color.value);
		return buffer;
	}

	unsigned TextureManager::_loadEmpty(AllocatedTexture &tex, unsigned id)
	{
		if (id != 0)
			tex.index = id;
		else if (this->_freedIndexes.empty()) {
			this->_lastIndex += this->_lastIndex == 0;
			tex.index = this->_lastIndex++;
		} else {
			tex.index = this->_freedIndexes.back();
			this->_freedIndexes.pop_back();
		}
		assert_exp(this->_textures[tex.index].resize({1, 1}));
		return tex.index;
	}

	unsigned TextureManager::_loadPaletted(const std::string &path, AllocatedTexture &tex, const std::array<Color, 256> &palette, unsigned id)
	{
		std::vector<unsigned char> buffer;
		ShadyCore::Image resourceImage;
		auto entry = game->package.find(path, ShadyCore::FileType::TYPE_IMAGE);

		if (entry == game->package.end()) {
			game->logger.error("Could not find image " + path);
			return 0;
		}

		auto &stream = entry.open();
		ShadyCore::getResourceReader(entry.fileType())(&resourceImage, stream);
		entry.close(stream);

		sf::Image image{{resourceImage.width, resourceImage.height}};

		if (resourceImage.bitsPerPixel == 8) {
			for (unsigned i = 0; i < resourceImage.getRawSize(); ++i) {
				unsigned x = i % resourceImage.paddedWidth;
				unsigned y = i / resourceImage.paddedWidth;

				if (x >= resourceImage.width)
					continue;
				if (y >= resourceImage.height)
					continue;
				image.setPixel({x, y}, palette[resourceImage.raw[i]]);
			}
		} else {
			for (unsigned y = 0; y < resourceImage.height; y++)
				for (unsigned x = 0; x < resourceImage.width; x++)
					image.setPixel({x, y}, sf::Color{
						resourceImage.raw[(y * resourceImage.paddedWidth + x) * 4 + 2],
						resourceImage.raw[(y * resourceImage.paddedWidth + x) * 4 + 1],
						resourceImage.raw[(y * resourceImage.paddedWidth + x) * 4 + 0],
						resourceImage.raw[(y * resourceImage.paddedWidth + x) * 4 + 3]
					});
		}

		if (id != 0)
			tex.index = id;
		else if (this->_freedIndexes.empty()) {
			this->_lastIndex += this->_lastIndex == 0;
			tex.index = this->_lastIndex;
			this->_lastIndex++;
		} else {
			tex.index = this->_freedIndexes.back();
			this->_freedIndexes.pop_back();
		}

		assert_exp(this->_textures[tex.index].loadFromImage(image));
		game->logger.verbose("Loaded texture" + path + " successfully");
		return tex.index;
	}

	unsigned TextureManager::load(const std::string &path, const std::array<Color, 256> &palette, Vector2u *size, bool repeated)
	{
		std::vector<unsigned char> buffer;
		std::string paletteName;
		auto oit = this->_overrideList.find(path);
		auto file = oit == this->_overrideList.end() ? path : oit->second;

		paletteName.resize(256 * 6);

		auto ptr = paletteName.data();

		for (auto color : palette) {
			sprintf(ptr, "%02X%02X%02X", color.r, color.g, color.b);
			ptr += 6;
		}

		std::string allocName = path + ":" + paletteName;

		if (this->_allocatedTextures[allocName].count != 0) {
			this->_allocatedTextures[allocName].count++;
			game->logger.verbose("Returning already loaded paletted file " + allocName);
			if (size)
				*size = this->_textures[this->_allocatedTextures[allocName].index].getSize();
			return this->_allocatedTextures[allocName].index;
		}

		if (file != path)
			game->logger.debug("Loading texture " + file + " (" + path + ") with palette " + paletteName);
		else
			game->logger.debug("Loading texture " + file + " with palette " + paletteName);

		AllocatedTexture texture{ 0, 1, path, palette };

		if (this->_loadPaletted(file, texture, palette, 0) == 0)
			this->_loadEmpty(texture, 0);

		if (size)
			*size = this->_textures[texture.index].getSize();

		this->_textures[texture.index].setRepeated(repeated);
		this->_allocatedTextures[allocName] = texture;
		this->_allocatedTexturesPaths[texture.index] = allocName;
		return texture.index;
	}

	void TextureManager::remove(unsigned int id)
	{
		if (id == 0)
			return;

		auto &path = this->_allocatedTexturesPaths.at(id);
		auto &texture = this->_allocatedTextures.at(path);

		if (texture.count) {
			texture.count--;
			if (texture.count) {
				game->logger.verbose("Remove ref to " + path);
				return;
			}
			game->logger.debug("Destroying texture " + path);
		}

		auto it = this->_textures.find(id);

		assert_exp(it != this->_textures.end());
		this->_textures.erase(it);
		this->_freedIndexes.push_back(id);
	}

	const sf::Texture &TextureManager::getTexture(unsigned handle) const
	{
		if (!handle)
			return this->_dummy.getTexture();
		return this->_textures.at(handle);
	}

	void TextureManager::addRef(unsigned int id)
	{
		if (id == 0)
			return;

		auto &path = this->_allocatedTexturesPaths.at(id);
		auto &texture = this->_allocatedTextures.at(path);

		if (texture.count) {
			texture.count++;
			assert_exp(texture.count > 1);
			game->logger.verbose("Adding ref to " + path);
		}
	}

	Vector2u TextureManager::getTextureSize(unsigned int id) const
	{
		if (id == 0)
			return {0, 0};
		return this->_textures.at(id).getSize();
	}

	void TextureManager::reloadEverything()
	{
		for (auto &[loadedPath, attr] : this->_allocatedTextures)
			if (attr.count) {
				game->logger.debug("Reloading " + loadedPath);
				if (this->_loadPaletted(attr.path, attr, attr.palette, attr.index))
					this->_loadEmpty(attr, attr.index);
			}
	}

	void TextureManager::addOverride(const std::string &base, const std::string &newVal)
	{
		this->_overrideList[base] = newVal;
	}

	void TextureManager::removeOverride(const std::string &base)
	{
		auto it = this->_overrideList.find(base);

		if (it == this->_overrideList.end())
			return;
		this->_overrideList.erase(it);
	}

	const std::array<Color, 256> &TextureManager::getPalette(unsigned int id)
	{
		auto &path = this->_allocatedTexturesPaths.at(id);
		auto &texture = this->_allocatedTextures.at(path);

		return texture.palette;
	}
}

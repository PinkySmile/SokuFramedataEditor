//
// Created by PinkySmile on 18/09/2021.
//

#include "TextureManager.hpp"
#include "Resources/Game.hpp"
#include "Logger.hpp"

namespace SpiralOfFate
{
	static unsigned pngCrcTable[256];

	static unsigned pngCrc32(const unsigned char *data, size_t length, unsigned crc = 0)
	{
		unsigned c = crc ^ 0xFFFFFFFF;

		for (size_t i = 0; i < length; i++)
			c = pngCrcTable[(c ^ data[i]) & 255] ^ ((c >> 8) & 0xFFFFFF);
		return c ^ 0xFFFFFFFF;
	}

	TextureManager::TextureManager()
	{
		this->_dummy.clear(Color::Transparent);
		this->_dummy.display();

		for (size_t i = 0; i < 256; i++) {
			size_t c = i;

			for (size_t k = 0; k < 8; k++) {
				if (c & 1)
					c = 0xEDB88320 ^ ((c >> 1) & 0x7FFFFFFF);
				else
					c = (c >> 1) & 0x7FFFFFFF;
			}
			pngCrcTable[i] = c;
		}
	}

	unsigned TextureManager::load(std::string path, Vector2u *size, bool repeated)
	{
		for (auto pos = path.find('\\'); pos != std::string::npos; pos = path.find('\\'))
			path[pos] = '/';

		auto oit = this->_overrideList.find(path);
		auto file = oit == this->_overrideList.end() ? path : oit->second;

		if (this->_allocatedTextures[file].count != 0) {
			this->_allocatedTextures[file].count++;
			game->logger.verbose("Returning already loaded file " + path);
			if (size)
				*size = this->_textures[this->_allocatedTextures[file].index].getSize();
			return this->_allocatedTextures[file].index;
		}

		AllocatedTexture texture;

		texture.path = path;
		texture.count = 1;
		if (file != path)
			game->logger.debug("Loading texture " + file + " (" + path + ")");
		else
			game->logger.debug("Loading texture " + file);
		if (this->_loadRegular(file, texture, 0) == 0) {
			game->logger.warn("Failed to load texture " + file);
			this->_loadEmpty(texture, 0);
		}

		if (size)
			*size = this->_textures[texture.index].getSize();

		this->_textures[texture.index].setRepeated(repeated);
		this->_allocatedTextures[file] = texture;
		this->_allocatedTexturesPaths[texture.index] = file;
		return texture.index;
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
		tex.palette.reset();
		tex.paletteData.reset();
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

	unsigned TextureManager::_load(const std::string &path, AllocatedTexture &tex, unsigned id)
	{
		unsigned result;

		if (tex.palette)
			result = this->_loadPaletted(path, tex, *tex.palette, id);
		else
			result = this->_loadRegular(path, tex, id);
		if (result == 0)
			result = this->_loadEmpty(tex, id);
		return result;
	}

	unsigned TextureManager::_loadRegular(const std::string &path, AllocatedTexture &tex, unsigned id)
	{
		sf::Texture texture;

		if (!texture.loadFromFile(path)) {
			game->logger.error("Failed to load " + path + ": " + strerror(errno));
			return 0;
		}
		tex.palette.reset();
		tex.paletteData.reset();
		if (id != 0);
		else if (this->_freedIndexes.empty()) {
			this->_lastIndex += this->_lastIndex == 0;
			id = this->_lastIndex++;
		} else {
			id = this->_freedIndexes.back();
			this->_freedIndexes.pop_back();
		}
		this->_textures[id].swap(texture);
		game->logger.verbose("Loaded texture" + path + " successfully");
		tex.index = id;
		return id;
	}

	unsigned TextureManager::_loadPaletted(const std::string &path, AllocatedTexture &tex, const std::array<Color, 256> &palette, unsigned id)
	{
		std::error_code err;
		std::vector<unsigned char> buffer;
		size_t fileSize = std::filesystem::file_size(path, err);
		if (err) {
			// TODO: Only works on Linux
			game->logger.error("Failed to load " + path + ": " + strerror(errno));
			return 0;
		}

		std::ifstream pngStream{path, std::fstream::binary};
		if (pngStream.fail()) {
			// TODO: Only works on Linux
			game->logger.error("Failed to load " + path + ": " + strerror(errno));
			return 0;
		}

		buffer.resize(fileSize);
		pngStream.read(reinterpret_cast<char *>(buffer.data()), buffer.size());
		pngStream.close();

		std::string_view view{reinterpret_cast<char *>(buffer.data()), buffer.size()};
		size_t pos = view.find("PLTE");

		if (pos != std::string::npos) {
			tex.paletteData.emplace();
			tex.palette = palette;

			size_t start = pos;

			(*tex.palette)[251] = game->typeColors[TYPECOLOR_NEUTRAL];
			(*tex.palette)[252] = game->typeColors[TYPECOLOR_NON_TYPED];
			(*tex.palette)[253] = game->typeColors[TYPECOLOR_MATTER];
			(*tex.palette)[254] = game->typeColors[TYPECOLOR_SPIRIT];
			(*tex.palette)[255] = game->typeColors[TYPECOLOR_VOID];
			for (size_t i = 0, j = 0; i < 256; i++, j += 3) {
				(*tex.paletteData)[j] = (*tex.palette)[i].r;
				(*tex.paletteData)[j + 1] = (*tex.palette)[i].g;
				(*tex.paletteData)[j + 2] = (*tex.palette)[i].b;
			}

			pos += 4;
			memcpy(&buffer[pos], tex.paletteData->data(), tex.paletteData->size());
			pos += tex.paletteData->size();

			unsigned crc = pngCrc32(&buffer[start], tex.paletteData->size() + 4);

			buffer[pos++] = (crc >> 24) & 0xFF;
			buffer[pos++] = (crc >> 16) & 0xFF;
			buffer[pos++] = (crc >> 8) & 0xFF;
			buffer[pos++] = (crc >> 0) & 0xFF;

			pos = buffer.size();
			buffer.resize(buffer.size() + 13);
			memcpy(&buffer[pos], "\x00\x00\x00\x01tRNS\x00\x40\xe6\xd8\x66", 13);
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

		assert_exp(this->_textures[tex.index].loadFromMemory(buffer.data(), buffer.size()));
		game->logger.verbose("Loaded texture" + path + " successfully");
		return tex.index;
	}

	unsigned TextureManager::_loadPaletted(const std::string &path, AllocatedTexture &tex, const std::string &palette, unsigned id)
	{
		std::error_code err;
		std::vector<unsigned char> buffer;
		size_t fileSize = std::filesystem::file_size(path, err);
		if (err) {
			// TODO: Only works on Linux
			game->logger.error("Failed to load " + path + ": " + strerror(errno));
			return 0;
		}

		std::ifstream pngStream{path, std::fstream::binary};
		if (pngStream.fail()) {
			// TODO: Only works on Linux
			game->logger.error("Failed to load " + path + ": " + strerror(errno));
			return 0;
		}

		buffer.resize(fileSize);
		pngStream.read(reinterpret_cast<char *>(buffer.data()), buffer.size());
		pngStream.close();

		std::string_view view{reinterpret_cast<char *>(buffer.data()), buffer.size()};
		size_t pos = view.find("PLTE");

		if (pos != std::string::npos) {
			size_t start = pos;
			std::ifstream palStream{palette, std::fstream::binary};
			if (palStream.fail()) {
				// TODO: Only works on Linux
				game->logger.error("Failed to load " + palette + ": " + strerror(errno));
				return 0;
			}

			tex.palette.emplace();
			tex.paletteData.emplace();

			assert_eq(std::filesystem::file_size(palette), 768ULL);
			palStream.read(reinterpret_cast<char *>(tex.paletteData->data()), tex.paletteData->size());
			palStream.close();

			(*tex.paletteData)[753] = game->typeColors[TYPECOLOR_NEUTRAL].r;
			(*tex.paletteData)[754] = game->typeColors[TYPECOLOR_NEUTRAL].g;
			(*tex.paletteData)[755] = game->typeColors[TYPECOLOR_NEUTRAL].b;
			(*tex.paletteData)[756] = game->typeColors[TYPECOLOR_NON_TYPED].r;
			(*tex.paletteData)[757] = game->typeColors[TYPECOLOR_NON_TYPED].g;
			(*tex.paletteData)[758] = game->typeColors[TYPECOLOR_NON_TYPED].b;
			(*tex.paletteData)[759] = game->typeColors[TYPECOLOR_MATTER].r;
			(*tex.paletteData)[760] = game->typeColors[TYPECOLOR_MATTER].g;
			(*tex.paletteData)[761] = game->typeColors[TYPECOLOR_MATTER].b;
			(*tex.paletteData)[762] = game->typeColors[TYPECOLOR_SPIRIT].r;
			(*tex.paletteData)[763] = game->typeColors[TYPECOLOR_SPIRIT].g;
			(*tex.paletteData)[764] = game->typeColors[TYPECOLOR_SPIRIT].b;
			(*tex.paletteData)[765] = game->typeColors[TYPECOLOR_VOID].r;
			(*tex.paletteData)[766] = game->typeColors[TYPECOLOR_VOID].g;
			(*tex.paletteData)[767] = game->typeColors[TYPECOLOR_VOID].b;

			for (size_t i = 0, j = 0; i < tex.palette->size(); i++, j += 3) {
				(*tex.palette)[i].r = (*tex.paletteData)[j];
				(*tex.palette)[i].g = (*tex.paletteData)[j + 1];
				(*tex.palette)[i].b = (*tex.paletteData)[j + 2];
			}

			pos += 4;
			memcpy(&buffer[pos], tex.paletteData->data(), tex.paletteData->size());
			pos += tex.paletteData->size();

			unsigned crc = pngCrc32(&buffer[start], tex.paletteData->size() + 4);

			buffer[pos++] = (crc >> 24) & 0xFF;
			buffer[pos++] = (crc >> 16) & 0xFF;
			buffer[pos++] = (crc >> 8) & 0xFF;
			buffer[pos++] = (crc >> 0) & 0xFF;

			pos = buffer.size();
			buffer.resize(buffer.size() + 13);
			memcpy(&buffer[pos], "\x00\x00\x00\x01tRNS\x00\x40\xe6\xd8\x66", 13);
		}

		if (id != 0)
			tex.index = id;
		else if (this->_freedIndexes.empty()) {
			this->_lastIndex += this->_lastIndex == 0;
			tex.index = this->_lastIndex++;
		} else {
			tex.index = this->_freedIndexes.back();
			this->_freedIndexes.pop_back();
		}

		assert_exp(this->_textures[tex.index].loadFromMemory(buffer.data(), buffer.size()));
		game->logger.verbose("Loaded texture" + path + " successfully");
		return tex.index;
	}

	unsigned TextureManager::load(const std::string &path, const std::string &palette, Vector2u *size, bool repeated)
	{
		if (palette.empty())
			return this->load(path, size, repeated);

		std::string allocName = path;
		auto oit = this->_overrideList.find(path);
		auto file = oit == this->_overrideList.end() ? path : oit->second;

		for (auto pos = allocName.find('\\'); pos != std::string::npos; pos = allocName.find('\\'))
			allocName[pos] = '/';
		allocName += ":";
		allocName += palette;
		if (this->_allocatedTextures[allocName].count != 0) {
			this->_allocatedTextures[allocName].count++;
			game->logger.verbose("Returning already loaded paletted file " + allocName);
			if (size)
				*size = this->_textures[this->_allocatedTextures[allocName].index].getSize();
			return this->_allocatedTextures[allocName].index;
		}

		if (file != path)
			game->logger.debug("Loading texture " + file + " (" + path + ") with palette " + palette);
		else
			game->logger.debug("Loading texture " + file + " with palette " + palette);

		AllocatedTexture texture;

		texture.path = path;
		texture.count = 1;
		if (this->_loadPaletted(file, texture, palette, 0) == 0)
			this->_loadEmpty(texture, 0);

		if (size)
			*size = this->_textures[texture.index].getSize();

		this->_textures[texture.index].setRepeated(repeated);
		this->_allocatedTextures[allocName] = texture;
		this->_allocatedTexturesPaths[texture.index] = allocName;
		return texture.index;
	}

	unsigned TextureManager::load(const std::string &path, const std::array<Color, 256> &palette, Vector2u *size, bool repeated)
	{
		if (palette.empty())
			return this->load(path, size, repeated);

		std::vector<unsigned char> buffer;
		std::string paletteName;
		std::error_code err;
		std::string allocName = path;
		auto oit = this->_overrideList.find(path);
		auto file = oit == this->_overrideList.end() ? path : oit->second;

		paletteName.resize(256 * 6);

		auto ptr = paletteName.data();

		for (auto color : palette) {
			sprintf(ptr, "%02X%02X%02X", color.r, color.g, color.b);
			ptr += 6;
		}

		for (auto pos = allocName.find('\\'); pos != std::string::npos; pos = allocName.find('\\'))
			allocName[pos] = '/';
		allocName += ":";
		allocName += paletteName;
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

		AllocatedTexture texture;

		texture.path = path;
		texture.count = 1;
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
				this->_load(attr.path, attr, attr.index);
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

	const std::optional<std::array<Color, 256>> &TextureManager::getPalette(unsigned int id)
	{
		auto &path = this->_allocatedTexturesPaths.at(id);
		auto &texture = this->_allocatedTextures.at(path);

		return texture.palette;
	}
}

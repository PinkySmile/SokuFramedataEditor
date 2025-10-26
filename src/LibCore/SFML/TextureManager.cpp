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

		if (this->_allocatedTextures[file].second != 0) {
			this->_allocatedTextures[file].second++;
			game->logger.verbose("Returning already loaded file " + path);
			if (size)
				*size = this->_textures[this->_allocatedTextures[file].first].getSize();
			return this->_allocatedTextures[file].first;
		}

		unsigned index;

		if (this->_freedIndexes.empty()) {
			this->_lastIndex += this->_lastIndex == 0;
			index = this->_lastIndex;
			this->_lastIndex++;
		} else {
			index = this->_freedIndexes.back();
			this->_freedIndexes.pop_back();
		}

		if (file != path)
			game->logger.debug("Loading texture " + file + " (" + path + ")");
		else
			game->logger.debug("Loading texture " + file);
		if (!this->_textures[index].loadFromFile(file)) {
			game->logger.warn("Failed to load texture " + file);
			assert_exp(this->_textures[index].resize({1, 1}));
		} else
			game->logger.verbose("Loaded texture" + file + " successfully");

		if (size)
			*size = this->_textures[index].getSize();

		this->_textures[index].setRepeated(repeated);
		this->_allocatedTextures[file].first = index;
		this->_allocatedTextures[file].second = 1;
		return index;
	}

	static std::string colorToString(Color color)
	{
		char buffer[10];

		color.a = 0;
		sprintf(buffer, "#%06x", color.value);
		return buffer;
	}

	unsigned TextureManager::load(const std::string &path, const std::string &palette, Vector2u *size, bool repeated)
	{
		if (palette.empty())
			return this->load(path, size, repeated);

		std::vector<unsigned char> buffer;
		std::array<unsigned char, 768> paletteData;
		std::error_code err;
		std::string allocName = path;
		auto oit = this->_overrideList.find(path);
		auto file = oit == this->_overrideList.end() ? path : oit->second;

		for (auto pos = allocName.find('\\'); pos != std::string::npos; pos = allocName.find('\\'))
			allocName[pos] = '/';
		allocName += ":";
		allocName += palette;
		if (this->_allocatedTextures[allocName].second != 0) {
			this->_allocatedTextures[allocName].second++;
			game->logger.verbose("Returning already loaded paletted file " + allocName);
			if (size)
				*size = this->_textures[this->_allocatedTextures[allocName].first].getSize();
			return this->_allocatedTextures[allocName].first;
		}

		if (file != path)
			game->logger.debug("Loading texture " + file + " (" + path + ") with palette " + palette);
		else
			game->logger.debug("Loading texture " + file + " with palette " + palette);

		size_t fileSize = std::filesystem::file_size(file, err);
		if (err) {
			// TODO: Only works on Linux
			game->logger.error("Failed to load " + file + ": " + strerror(errno));
			return this->load(path, size, repeated);
		}

		std::ifstream pngStream{file, std::fstream::binary};
		if (pngStream.fail()) {
			// TODO: Only works on Linux
			game->logger.error("Failed to load " + file + ": " + strerror(errno));
			return this->load(path, size, repeated);
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
				return this->load(path, size, repeated);
			}

			assert_eq(std::filesystem::file_size(palette), 768ULL);
			palStream.read(reinterpret_cast<char *>(paletteData.data()), paletteData.size());
			palStream.close();

			paletteData[753] = game->typeColors[TYPECOLOR_NEUTRAL].r;
			paletteData[754] = game->typeColors[TYPECOLOR_NEUTRAL].g;
			paletteData[755] = game->typeColors[TYPECOLOR_NEUTRAL].b;
			paletteData[756] = game->typeColors[TYPECOLOR_NON_TYPED].r;
			paletteData[757] = game->typeColors[TYPECOLOR_NON_TYPED].g;
			paletteData[758] = game->typeColors[TYPECOLOR_NON_TYPED].b;
			paletteData[759] = game->typeColors[TYPECOLOR_MATTER].r;
			paletteData[760] = game->typeColors[TYPECOLOR_MATTER].g;
			paletteData[761] = game->typeColors[TYPECOLOR_MATTER].b;
			paletteData[762] = game->typeColors[TYPECOLOR_SPIRIT].r;
			paletteData[763] = game->typeColors[TYPECOLOR_SPIRIT].g;
			paletteData[764] = game->typeColors[TYPECOLOR_SPIRIT].b;
			paletteData[765] = game->typeColors[TYPECOLOR_VOID].r;
			paletteData[766] = game->typeColors[TYPECOLOR_VOID].g;
			paletteData[767] = game->typeColors[TYPECOLOR_VOID].b;

			pos += 4;
			memcpy(&buffer[pos], paletteData.data(), paletteData.size());
			pos += paletteData.size();

			unsigned crc = pngCrc32(&buffer[start], paletteData.size() + 4);

			buffer[pos++] = (crc >> 24) & 0xFF;
			buffer[pos++] = (crc >> 16) & 0xFF;
			buffer[pos++] = (crc >> 8) & 0xFF;
			buffer[pos++] = (crc >> 0) & 0xFF;

			pos = buffer.size();
			buffer.resize(buffer.size() + 13);
			memcpy(&buffer[pos], "\x00\x00\x00\x01tRNS\x00\x40\xe6\xd8\x66", 13);
		}

		unsigned index;

		if (this->_freedIndexes.empty()) {
			this->_lastIndex += this->_lastIndex == 0;
			index = this->_lastIndex;
			this->_lastIndex++;
		} else {
			index = this->_freedIndexes.back();
			this->_freedIndexes.pop_back();
		}

		assert_exp(this->_textures[index].loadFromMemory(buffer.data(), buffer.size()));
		game->logger.verbose("Loaded texture" + file + " successfully");

		if (size)
			*size = this->_textures[index].getSize();

		this->_textures[index].setRepeated(repeated);
		this->_allocatedTextures[allocName].first = index;
		this->_allocatedTextures[allocName].second = 1;
		return index;
	}

	void TextureManager::remove(unsigned int id)
	{
		if (id == 0)
			return;

		for (auto &[loadedPath, attr] : this->_allocatedTextures)
			if (attr.first == id && attr.second) {
				attr.second--;
				if (attr.second) {
					game->logger.verbose("Remove ref to " + loadedPath);
					return;
				}
				game->logger.debug("Destroying texture " + loadedPath);
				break;
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
		for (auto &[loadedPath, attr] : this->_allocatedTextures)
			if (attr.first == id && attr.second) {
				attr.second++;
				assert_exp(attr.second > 1);
				game->logger.verbose("Adding ref to " + loadedPath);
				return;
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
			if (attr.second) {
				game->logger.debug("Reloading " + loadedPath);
				this->_reload(loadedPath, attr.first);
			}
	}

	void TextureManager::_reload(const std::string &path, unsigned id)
	{
		Vector2u realSize;
		std::string p = path;
		size_t pos = p.find(':');
		auto elem = p.substr(0, pos);

		//game->logger.debug("Reloading resulting image (" + std::to_string(pal1.size()) + " paletted colors)");

		//if (!this->_textures[id].loadFromImage(image))
		//	game->logger.error("Failed loading texture");
		assert_msg(false, "Not Implemented");
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
}

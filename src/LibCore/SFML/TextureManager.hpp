//
// Created by PinkySmile on 18/09/2021.
//

#ifndef SOFGV_TEXTUREMANAGER_HPP
#define SOFGV_TEXTUREMANAGER_HPP


#include <unordered_map>
#include <string>
#include <SFML/Graphics.hpp>
#include "Sprite.hpp"
#include "Data/Vector.hpp"
#include "Color.hpp"

namespace SpiralOfFate
{
	class TextureManager {
	private:
		struct AllocatedTexture {
			unsigned index;
			unsigned count;
			std::string path;
			std::optional<std::array<Color, 256>> palette;
			std::optional<std::array<unsigned char, 768>> paletteData;
		};

		sf::RenderTexture _dummy{{1, 1}};
		unsigned _lastIndex = 0;
		std::vector<unsigned> _freedIndexes;
		std::unordered_map<unsigned, sf::Texture> _textures;
		std::unordered_map<unsigned, std::string> _allocatedTexturesPaths;
		std::unordered_map<std::string, AllocatedTexture> _allocatedTextures;
		std::unordered_map<std::string, std::string> _overrideList;

		unsigned _loadEmpty(AllocatedTexture &tex, unsigned currentIndex);
		unsigned _load(const std::string &path, AllocatedTexture &tex, unsigned currentIndex);
		unsigned _loadRegular(const std::string &path, AllocatedTexture &tex, unsigned currentIndex);
		unsigned _loadPaletted(const std::string &path, AllocatedTexture &tex, const std::string &palette, unsigned currentIndex);
		unsigned _loadPaletted(const std::string &path, AllocatedTexture &tex, const std::array<Color, 256> &palette, unsigned currentIndex);
	public:
		TextureManager();

		unsigned load(std::string file, Vector2u *size = nullptr, bool repeated = false);
		unsigned load(const std::string &file, const std::string &palette, Vector2u *size = nullptr, bool repeated = false);
		unsigned load(const std::string &file, const std::array<Color, 256> &palette, Vector2u *size = nullptr, bool repeated = false);
		Vector2u getTextureSize(unsigned id) const;
		void addRef(unsigned id);
		void remove(unsigned id);
		const std::optional<std::array<Color, 256>> &getPalette(unsigned id);
		const sf::Texture &getTexture(unsigned id) const;
		void reloadEverything();
		void addOverride(const std::string &base, const std::string &newVal);
		void removeOverride(const std::string &base);
	};
}


#endif //SOFGV_TEXTUREMANAGER_HPP

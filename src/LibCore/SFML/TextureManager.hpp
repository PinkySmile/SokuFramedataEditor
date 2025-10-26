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
		sf::RenderTexture _dummy{{1, 1}};
		unsigned _lastIndex = 0;
		std::vector<unsigned> _freedIndexes;
		std::unordered_map<unsigned, sf::Texture> _textures;
		std::unordered_map<std::string, std::pair<unsigned, unsigned>> _allocatedTextures;
		std::unordered_map<std::string, std::string> _overrideList;

		void _reload(const std::string &path, unsigned id);
	public:
		TextureManager();

		unsigned load(std::string file, Vector2u *size = nullptr, bool repeated = false);
		unsigned load(const std::string &file, const std::string &palette, Vector2u *size = nullptr, bool repeated = false);
		Vector2u getTextureSize(unsigned id) const;
		void addRef(unsigned id);
		void remove(unsigned id);
		const sf::Texture &getTexture(unsigned id) const;
		void reloadEverything();
		void addOverride(const std::string &base, const std::string &newVal);
		void removeOverride(const std::string &base);
	};
}


#endif //SOFGV_TEXTUREMANAGER_HPP

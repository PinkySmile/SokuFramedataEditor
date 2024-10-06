//
// Created by PinkySmile on 18/09/2021
//

#ifndef BATTLE_TEXTUREMANAGER_HPP
#define BATTLE_TEXTUREMANAGER_HPP


#include <map>
#include <string>
#include <SFML/Graphics.hpp>
#include "Sprite.hpp"
#include "../Data/Vector.hpp"
#include "../Data/Color.hpp"
#include <package.hpp>

namespace SpiralOfFate
{
	class TextureManager {
	private:
		unsigned _lastIndex = 0;
		std::vector<unsigned> _freedIndexes;
		std::map<unsigned, sf::Texture> _textures;
		std::map<std::string, ShadyCore::Image> _loadedImages;
		std::map<std::string, std::pair<unsigned, unsigned>> _allocatedTextures;

	public:
		~TextureManager();
		const ShadyCore::Image &getUnderlyingImage(const std::string &file);
		unsigned load(ShadyCore::PackageEx &package, const std::string &file, Vector2u *size = nullptr);
		unsigned load(ShadyCore::PackageEx &package, const ShadyCore::Palette &palette, const std::string &palName, const std::string &file, Vector2u *size = nullptr);
		Vector2u getTextureSize(unsigned id) const;
		void addRef(unsigned id);
		void remove(unsigned id);
		void invalidatePalette(const std::string &palName);
		void render(Sprite &sprite) const;
		void setTexture(Sprite &sprite) const;
		void invalidateAll();
	};
}


#endif //BATTLE_TEXTUREMANAGER_HPP

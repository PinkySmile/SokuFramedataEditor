//
// Created by PinkySmile on 18/09/2021.
//

#ifndef SOFGV_SPRITE_HPP
#define SOFGV_SPRITE_HPP


#include <SFML/Graphics.hpp>
#include "Screen.hpp"

namespace SpiralOfFate
{
	struct Sprite : public sf::Sprite {
	private:
		unsigned _textureHandle = 0;

	public:
		Sprite();
		Sprite(unsigned handle);
		Sprite(const Sprite &o);
		~Sprite();

		using sf::Sprite::setTexture;
		void setTexture(unsigned handle, bool resetRect = false);
		unsigned getHandle() const;
		Vector2u getTextureSize() const;
		void setTextureRect(SpiralOfFate::IntRect rect);
	};
}


#endif //SOFGV_SPRITE_HPP

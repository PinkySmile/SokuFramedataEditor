//
// Created by PinkySmile on 01/03/25.
//

#include "Sprite.hpp"
#include "Resources/Game.hpp"

SpiralOfFate::Sprite::Sprite() :
	Sprite(0)
{
}

SpiralOfFate::Sprite::Sprite(unsigned int handle) :
	sf::Sprite(game->textureMgr.getTexture(handle)),
	_textureHandle(handle)
{
	game->textureMgr.addRef(handle);
}

SpiralOfFate::Sprite::Sprite(const Sprite &o) :
	sf::Sprite(o),
	_textureHandle(o.getHandle())
{
	game->textureMgr.addRef(o.getHandle());
}

SpiralOfFate::Sprite::~Sprite()
{
	if (this->_textureHandle)
		game->textureMgr.remove(this->_textureHandle);
}

void SpiralOfFate::Sprite::setTexture(unsigned int handle, bool resetRect)
{
	game->textureMgr.remove(this->_textureHandle);
	game->textureMgr.addRef(handle);
	this->_textureHandle = handle;
	this->sf::Sprite::setTexture(game->textureMgr.getTexture(handle), resetRect);
}

unsigned SpiralOfFate::Sprite::getHandle() const
{
	return this->_textureHandle;
}

SpiralOfFate::Vector2u SpiralOfFate::Sprite::getTextureSize() const
{
	return game->textureMgr.getTextureSize(this->_textureHandle);
}

void SpiralOfFate::Sprite::setTextureRect(SpiralOfFate::IntRect rect)
{
	this->sf::Sprite::setTextureRect(rect);
}

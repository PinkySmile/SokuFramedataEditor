//
// Created by PinkySmile on 18/09/2021.
//

#include <filesystem>
#include "Color.hpp"
#include "EditableObject.hpp"

using namespace SpiralOfFate;

EditableObject::EditableObject(const std::string &frameData) :
	_folder(frameData.substr(0, frameData.find_last_of(std::filesystem::path::preferred_separator)))
{
	this->_moves = FrameData::loadFile(frameData, this->_folder);
}

void EditableObject::render(sf::RenderTarget &target, sf::RenderStates states) const
{
	sf::RectangleShape rect;
	auto &data = this->_moves.at(this->_action)[this->_actionBlock][this->_animation];
	auto size = Vector2f{
		data.scale.x * data.textureBounds.size.x,
		data.scale.y * data.textureBounds.size.y
	};
	auto result = data.offset;
	Sprite sprite;

	result.y *= -1;
	result += Vector2f{
		size.x / -2.f,
		-size.y
	};
	result += Vector2f{
		data.textureBounds.size.x * data.scale.x / 2,
		data.textureBounds.size.y * data.scale.y / 2
	};
	sprite.setColor(Color::White);
	sprite.setOrigin(data.textureBounds.size / 2.f);
	sprite.setRotation(sf::radians(data.rotation));
	sprite.setPosition(result);
	sprite.setScale(data.scale);
	sprite.setTexture(data.textureHandle);
	sprite.setTextureRect(data.textureBounds);
	target.draw(sprite, states);
	if (data.oFlag.spiritElement == data.oFlag.matterElement && data.oFlag.matterElement == data.oFlag.voidElement)
		sprite.setColor(game->typeColors[data.oFlag.spiritElement ? TYPECOLOR_NEUTRAL : TYPECOLOR_NON_TYPED]);
	else if (data.oFlag.spiritElement)
		sprite.setColor(game->typeColors[TYPECOLOR_SPIRIT]);
	else if (data.oFlag.matterElement)
		sprite.setColor(game->typeColors[TYPECOLOR_MATTER]);
	else if (data.oFlag.voidElement)
		sprite.setColor(game->typeColors[TYPECOLOR_VOID]);
	sprite.setTexture(data.textureHandleEffects);
	target.draw(sprite, states);

	rect.setOutlineThickness(2);
	rect.setOutlineColor(Color::White);
	rect.setFillColor(Color::Black);
	rect.setPosition(Vector2f{-4, -4});
	rect.setSize({9, 9});
	target.draw(rect, states);
}

void EditableObject::update()
{
	auto *data = &this->_moves.at(this->_action)[this->_actionBlock][this->_animation];

	this->_animationCtr++;
	while (this->_animationCtr >= data->duration) {
		this->_animationCtr = 0;
		this->_animation++;
		this->_animation %= this->_moves.at(this->_action)[this->_actionBlock].size();
		if (this->_animation == 0)
			this->_position = {0, 0};
		data = &this->_moves.at(this->_action)[this->_actionBlock][this->_animation];
		game->soundMgr.play(data->soundHandle);
	}
	this->_position += this->_speed;
	this->_speed.y += this->_gravity;
}

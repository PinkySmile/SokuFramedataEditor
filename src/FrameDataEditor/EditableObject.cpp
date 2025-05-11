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
	auto result = data.offset + this->_position;
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
	sprite.setRotation(sf::radians(this->_rotation));
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
	rect.setPosition(Vector2f{-4 + this->_position.x, -4 - this->_position.y});
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
		this->resetState();
		game->soundMgr.play(data->soundHandle);
	}
	this->_simulate(*data);
}

SpiralOfFate::FrameData &EditableObject::getFrameData()
{
	return this->_moves.at(this->_action).at(this->_actionBlock).at(this->_animation);
}

const SpiralOfFate::FrameData &EditableObject::getFrameData() const
{
	return this->_moves.at(this->_action).at(this->_actionBlock).at(this->_animation);
}

std::vector<SpiralOfFate::Rectangle> EditableObject::_getModifiedBoxes(const FrameData &data, const std::vector<SpiralOfFate::Box> &boxes) const
{
	std::vector<SpiralOfFate::Rectangle> result;
	Vector2f center{
		static_cast<float>(data.offset.x),
		data.textureBounds.size.y * data.scale.y / -2.f - data.offset.y
	};

	for (auto &_box : boxes) {
		Rectangle __box;
		auto real = Vector2f{this->_position.x, -this->_position.y};

		__box.pt1 = real + _box.pos.rotation(this->_rotation, center);
		__box.pt2 = real + (_box.pos + Vector2f{0, static_cast<float>(_box.size.y)}).rotation(this->_rotation, center);
		__box.pt3 = real + (_box.pos + _box.size.to<int>()).rotation(this->_rotation, center);
		__box.pt4 = real + (_box.pos + Vector2f{static_cast<float>(_box.size.x), 0}).rotation(this->_rotation, center);
		result.push_back(__box);
	}
	return result;
}

std::vector<SpiralOfFate::Rectangle> EditableObject::_getModifiedHurtBoxes() const
{
	return this->_getModifiedBoxes(this->getFrameData(), this->getFrameData().hurtBoxes);
}

std::vector<SpiralOfFate::Rectangle> EditableObject::_getModifiedHitBoxes() const
{
	return this->_getModifiedBoxes(this->getFrameData(), this->getFrameData().hitBoxes);
}

void EditableObject::resetState()
{
	auto anim = this->_animation;
	auto ctr = this->_animationCtr;

	this->_rotation = 0;
	// TODO:
	this->_animation = 0;
	this->_animationCtr = 0;
	this->_speed = {0, this->getFrameData().dFlag.airborne ? 5.f : 0.f};
	this->_position = {0, this->getFrameData().dFlag.airborne ? 100.f : 0.f};

	while (this->_animation != anim) {
		auto &data = this->getFrameData();

		for (size_t i = 0; i < data.duration; i++)
			this->_simulate(data);
		this->_animation++;
	}
	while (this->_animationCtr <= ctr) {
		auto &data = this->getFrameData();

		this->_animationCtr++;
		this->_simulate(data);
	}
	this->_animationCtr--;
}

void EditableObject::_simulate(const SpiralOfFate::FrameData &data)
{
	if (data.dFlag.resetSpeed)
		this->_speed = {0, 0};
	if (data.dFlag.resetRotation)
		this->_rotation = 0;
	this->_rotation += data.rotation;
	this->_rotation = std::fmod(this->_rotation, 2 * M_PI);
	this->_speed += data.speed;
	this->_position += this->_speed;
	if (data.dFlag.airborne) {
		// TODO::
		this->_speed.x *= 0.985;
		this->_speed.y *= 1;
	} else
		// TODO:
		this->_speed.x *= 0.73;
	if (data.gravity)
		this->_speed += *data.gravity;
	else
		// TODO:
		this->_speed += Vector2f{0, -1};
	if (this->_position.y < 0) {
		this->_position.y = 0;
		this->_speed.y = 0;
	}
}

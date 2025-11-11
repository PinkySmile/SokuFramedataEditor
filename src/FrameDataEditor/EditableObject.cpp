//
// Created by PinkySmile on 18/09/2021.
//

#include <filesystem>
#include "Color.hpp"
#include "EditableObject.hpp"
#include "PngLoader.hpp"

using namespace SpiralOfFate;

// TODO: Use std::filesystem::path for paths
EditableObject::EditableObject(const std::string &frameData) :
	_folder(frameData.substr(0, frameData.find_last_of(std::filesystem::path::preferred_separator)))
{
	for (auto &pair : FrameData::loadFile(frameData, this->_folder))
		this->_moves[pair.first] = pair.second;
}

void EditableObject::render(sf::RenderTarget &target, sf::RenderStates states, bool displaceBoxes)
{
	if (this->_generateCd)
		this->_generateCd--;
	else if (this->_needGenerate) {
		this->_generateOverlaySprite();
		this->_generateCd = 4;
	}

	sf::RectangleShape rect;
	auto &data = this->_moves.at(this->_action)[this->_actionBlock][this->_animation];
	auto size = Vector2f{
		data.scale.x * data.textureBounds.size.x,
		data.scale.y * data.textureBounds.size.y
	};
	auto result = data.offset;

	data.checkReloadTexture();
	if (displaceBoxes)
		result += this->_position;
	result.y *= -1;
	result += Vector2f{
		size.x / -2.f,
		-size.y
	};
	result += Vector2f{
		data.textureBounds.size.x * data.scale.x / 2,
		data.textureBounds.size.y * data.scale.y / 2
	};

	this->_sprite.setColor(Color::White);
	this->_sprite.setOrigin(data.textureBounds.size / 2.f);
	if (displaceBoxes)
		this->_sprite.setRotation(sf::radians(this->_rotation));
	else
		this->_sprite.setRotation(sf::radians(0));
	this->_sprite.setPosition(result);
	this->_sprite.setScale(data.scale);
	this->_sprite.setTexture(data.textureHandle);
	this->_sprite.setTextureRect(data.textureBounds);
	target.draw(this->_sprite, states);

	if (this->_textureValid) {
		this->_overlaySprite.setColor(Color::White);
		this->_overlaySprite.setOrigin(data.textureBounds.size / 2.f);
		if (displaceBoxes)
			this->_overlaySprite.setRotation(sf::radians(this->_rotation));
		else
			this->_overlaySprite.setRotation(sf::radians(0));
		this->_overlaySprite.setPosition(result);
		this->_overlaySprite.setScale(data.scale);
		this->_overlaySprite.setTexture(this->_overlayTexture, true);
		target.draw(this->_overlaySprite, states);
	}

	this->_spriteEffect.setColor(Color::White);
	this->_spriteEffect.setOrigin(data.textureBounds.size / 2.f);
	if (displaceBoxes)
		this->_spriteEffect.setRotation(sf::radians(this->_rotation));
	else
		this->_spriteEffect.setRotation(sf::radians(0));
	this->_spriteEffect.setPosition(result);
	this->_spriteEffect.setScale(data.scale);
	this->_spriteEffect.setTexture(data.textureHandleEffects);
	this->_spriteEffect.setTextureRect(data.textureBounds);
	if (data.oFlag.spiritElement == data.oFlag.matterElement && data.oFlag.matterElement == data.oFlag.voidElement)
		this->_spriteEffect.setColor(game->typeColors[data.oFlag.spiritElement ? TYPECOLOR_NEUTRAL : TYPECOLOR_NON_TYPED]);
	else if (data.oFlag.spiritElement)
		this->_spriteEffect.setColor(game->typeColors[TYPECOLOR_SPIRIT]);
	else if (data.oFlag.matterElement)
		this->_spriteEffect.setColor(game->typeColors[TYPECOLOR_MATTER]);
	else if (data.oFlag.voidElement)
		this->_spriteEffect.setColor(game->typeColors[TYPECOLOR_VOID]);
	target.draw(this->_spriteEffect, states);

	rect.setOutlineThickness(1);
	rect.setScale(data.scale);
	rect.setOutlineColor(Color::White);
	rect.setFillColor(Color::Transparent);
	rect.setPosition(this->_sprite.getPosition());
	rect.setOrigin(data.textureBounds.size / 2.f);
	rect.setSize(data.textureBounds.size);
	target.draw(rect, states);

	rect.setOutlineThickness(2);
	rect.setOutlineColor(Color::White);
	rect.setFillColor(Color::Black);
	rect.setOrigin({0, 0});
	rect.setScale({1, 1});
	rect.setRotation(sf::radians(0));
	if (displaceBoxes)
		rect.setPosition(Vector2f{-4 + this->_position.x, -4 - this->_position.y});
	else
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
		this->resetState();
		game->soundMgr.play(data->soundHandle);
		this->_needGenerate = false;
		this->_generateOverlaySprite();
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

std::vector<SpiralOfFate::Rectangle> EditableObject::_getModifiedBoxes(bool displaceObject, const FrameData &data, const std::vector<SpiralOfFate::Box> &boxes) const
{
	std::vector<SpiralOfFate::Rectangle> result;
	Vector2f center{
		static_cast<float>(data.offset.x),
		data.textureBounds.size.y * data.scale.y / -2.f - data.offset.y
	};
	auto rotation = displaceObject ? this->_rotation : 0;
	auto real = displaceObject ? Vector2f{this->_position.x, -this->_position.y} : Vector2f{0, 0};

	result.reserve(boxes.size());
	for (auto &_box : boxes)
		result.push_back({
			.pt1 = real + _box.pos.rotation(rotation, center),
			.pt2 = real + (_box.pos + Vector2f{0, static_cast<float>(_box.size.y)}).rotation(rotation, center),
			.pt3 = real + (_box.pos + _box.size.to<int>()).rotation(rotation, center),
			.pt4 = real + (_box.pos + Vector2f{static_cast<float>(_box.size.x), 0}).rotation(rotation, center)
		});
	return result;
}

std::vector<SpiralOfFate::Rectangle> EditableObject::_getModifiedHurtBoxes(bool displaceObject) const
{
	return this->_getModifiedBoxes(displaceObject, this->getFrameData(), this->getFrameData().hurtBoxes);
}

std::vector<SpiralOfFate::Rectangle> EditableObject::_getModifiedHitBoxes(bool displaceObject) const
{
	return this->_getModifiedBoxes(displaceObject, this->getFrameData(), this->getFrameData().hitBoxes);
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
	this->_generateOverlaySprite();
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
		// TODO:
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

void EditableObject::_generateOverlaySprite()
{
	this->_textureValid = false;
	if (this->_paletteIndex <= 0)
		return;

	auto &data = this->_moves.at(this->_action)[this->_actionBlock][this->_animation];
	auto &img = pngLoader.loadImage(data.__folder + "/" + data.spritePath);

	if (img.bitsPerPixel != 8)
		return;

	sf::Image image;
	auto &pal = game->textureMgr.getPalette(data.textureHandle);
	const std::array<SpiralOfFate::Color, 256> *palette = &img.palette;

	if (pal)
		palette = &*pal;

	auto color = (*palette)[this->_paletteIndex];

	color.r = 255 - color.r;
	color.g = 255 - color.g;
	color.b = 255 - color.b;
	image.resize(data.textureBounds.size, sf::Color::Transparent);
	for (unsigned y_off = 0; y_off < data.textureBounds.size.y; y_off++) {
		int y = data.textureBounds.pos.y + y_off;

		if (y < 0)
			continue;
		else if ((unsigned)y >= img.height)
			break;
		for (unsigned x_off = 0; x_off < data.textureBounds.size.x; x_off++) {
			int x = data.textureBounds.pos.x + x_off;

			if (x < 0)
				continue;
			else if ((unsigned)x >= img.width)
				break;

			auto index = img.raw[y * img.paddedWidth + x];

			if (index != this->_paletteIndex)
				continue;
			image.setPixel(sf::Vector2u(x_off, y_off), color);
		}
	}
	assert_exp(this->_overlayTexture.loadFromImage(image));
	this->_textureValid = true;
}

void EditableObject::setMousePosition(const SpiralOfFate::Vector2f *pos)
{
	if (pos == nullptr) {
		this->_paletteIndex = -1;
		this->_textureValid = false;
		return;
	}

	auto sPos = this->_mousePosToImgPos(*pos);
	auto &data = this->_moves.at(this->_action)[this->_actionBlock][this->_animation];

	if (sPos.x < 0 || sPos.y < 0 || sPos.x >= data.textureBounds.size.x || sPos.y >= data.textureBounds.size.y) {
		this->_paletteIndex = -1;
		this->_textureValid = false;
		return;
	}

	auto &img = pngLoader.loadImage(data.__folder + "/" + data.spritePath);

	if (img.bitsPerPixel != 8) {
		this->_paletteIndex = -1;
		this->_textureValid = false;
		return;
	}
	sPos += data.textureBounds.pos;
	if (sPos.x < 0 || sPos.x >= img.width || sPos.y < 0 || sPos.y >= img.height)
		return;
	this->_paletteIndex = img.raw[static_cast<int>(sPos.y) * img.paddedWidth + static_cast<int>(sPos.x)];
	this->_needGenerate = true;
}

SpiralOfFate::Vector2f EditableObject::_mousePosToImgPos(const SpiralOfFate::Vector2i &mouse)
{
	auto &data = this->_moves.at(this->_action)[this->_actionBlock][this->_animation];
	auto size = Vector2f{
		data.scale.x * data.textureBounds.size.x,
		data.scale.y * data.textureBounds.size.y
	};
	auto result = data.offset.to<float>();

	result.y *= -1;
	result -= Vector2f{
		size.x / 2.f,
		size.y
	};
	result = mouse - result;
	result.x /= data.scale.x;
	result.y /= data.scale.y;
	this->_mousePos = result;
	return result;
}

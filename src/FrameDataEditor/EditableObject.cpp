//
// Created by PinkySmile on 18/09/2021
//

#include "EditableObject.hpp"

EditableObject::EditableObject(const std::string &chr, const ShadyCore::Schema &schema, const ShadyCore::Palette &palette, const std::string &palName)
{
	this->_moves = SpiralOfFate::FrameData::loadSchema(chr, schema, palette, palName);
}

void EditableObject::render() const
{
	sf::RectangleShape rect;
	auto &data = this->_moves.at(this->_action)[this->_actionBlock][this->_animation];
	auto translate = this->displayScaled ? this->translate : SpiralOfFate::Vector2f{0, 0};
	auto s = this->displayScaled ? this->scale : 1.f;

	if (data.needReload)
		data.reloadTexture();

	auto scale = SpiralOfFate::Vector2f{
		s * (data.blendOptions.scaleX ? data.blendOptions.scaleX : 200) / 100.f,
		s * (data.blendOptions.scaleY ? data.blendOptions.scaleY : 200) / 100.f
	};
	auto bounds = SpiralOfFate::Vector2f{
		static_cast<float>(data.texWidth),
		static_cast<float>(data.texHeight)
	};
	auto texBounds = sf::IntRect{
		data.texOffsetX,
		data.texOffsetY,
		data.texWidth,
		data.texHeight
	};
	auto result = SpiralOfFate::Vector2f{
		static_cast<float>(-data.offsetX) * s,
		static_cast<float>(-data.offsetY) * s
	};

	if (data.blendOptions.flipHorz) {
		texBounds.left += texBounds.width;
		texBounds.width *= -1;
	}
	if (data.blendOptions.flipVert) {
		texBounds.top += texBounds.height;
		texBounds.height *= -1;
	}
	//result.y *= -1;
	//result += SpiralOfFate::Vector2f{
	//	size.x / -2.f,
	//	-static_cast<float>(size.y)
	//};
	result += SpiralOfFate::Vector2f{
		data.texWidth * scale.x / 2,
		data.texHeight * scale.y / 2
	};
	result += translate;
	this->_sprite.setOrigin(bounds / 2.f);
	this->_sprite.setRotation(data.blendOptions.angle);
	this->_sprite.setPosition(result);
	this->_sprite.setScale(scale);
	this->_sprite.textureHandle = data.textureHandle;
	this->_sprite.setTextureRect(texBounds);
	SpiralOfFate::game->textureMgr.render(this->_sprite);

	rect.setOutlineThickness(2);
	rect.setOutlineColor(sf::Color::White);
	rect.setFillColor(sf::Color::Black);
	rect.setPosition(this->_position - SpiralOfFate::Vector2f{4, 4});
	rect.setSize({9, 9});
	SpiralOfFate::game->screen->draw(rect);
}

void EditableObject::update()
{
	auto *data = &this->_moves.at(this->_action)[this->_actionBlock][this->_animation];

	this->_animationCtr++;
	while (this->_animationCtr >= data->duration) {
		this->_animationCtr = 0;
		this->_animation++;
		this->_animation %= this->_moves.at(this->_action)[this->_actionBlock].size();
		data = &this->_moves.at(this->_action)[this->_actionBlock][this->_animation];
		SpiralOfFate::game->soundMgr.play(data->hitSoundHandle);
		if (!data->duration)
			break;
	}
	this->_position += this->_speed;
	this->_speed.y += this->_gravity;
}

void EditableObject::reset()
{
}

bool EditableObject::isDead() const
{
	return false;
}

bool EditableObject::hits(const IObject &) const
{
	return false;
}

void EditableObject::hit(SpiralOfFate::IObject &, const SpiralOfFate::FrameData *)
{
}

void EditableObject::getHit(SpiralOfFate::IObject &, const SpiralOfFate::FrameData *)
{
}

const SpiralOfFate::FrameData *EditableObject::getCurrentFrameData() const
{
	return nullptr;
}

bool EditableObject::collides(const SpiralOfFate::IObject &) const
{
	return false;
}

void EditableObject::collide(IObject &)
{
}

void EditableObject::kill()
{

}

unsigned int EditableObject::getBufferSize() const
{
	return 0;
}

void EditableObject::copyToBuffer(void *) const
{

}

void EditableObject::restoreFromBuffer(void *)
{

}

unsigned int EditableObject::getClassId() const
{
	return 0;
}

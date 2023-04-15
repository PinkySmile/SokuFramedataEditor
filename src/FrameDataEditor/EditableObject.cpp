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

	SpiralOfFate::Vector2f scale{
		s * (data.frame->hasBlendOptions() ? data.frame->blendOptions.scaleX : 200) / 100.f,
		s * (data.frame->hasBlendOptions() ? data.frame->blendOptions.scaleY : 200) / 100.f
	};
	SpiralOfFate::Vector2f scaleReal{
		(data.frame->hasBlendOptions() ? data.frame->blendOptions.scaleX : 100) / 100.f,
		(data.frame->hasBlendOptions() ? data.frame->blendOptions.scaleY : 100) / 100.f
	};
	SpiralOfFate::Vector2f bounds{
		static_cast<float>(data.frame->texWidth),
		static_cast<float>(data.frame->texHeight)
	};
	sf::IntRect texBounds{
		data.frame->texOffsetX,
		data.frame->texOffsetY,
		data.frame->texWidth,
		data.frame->texHeight
	};
	SpiralOfFate::Vector2f result{
		static_cast<float>(-data.frame->offsetX) * s * scaleReal.x,
		static_cast<float>(-data.frame->offsetY) * s * scaleReal.y
	};

	if (data.frame->blendOptions.flipHorz) {
		texBounds.left += texBounds.width;
		texBounds.width *= -1;
	}
	if (data.frame->blendOptions.flipVert) {
		texBounds.top += texBounds.height;
		texBounds.height *= -1;
	}
	//result.y *= -1;
	//result += SpiralOfFate::Vector2f{
	//	size.x / -2.f,
	//	-static_cast<float>(size.y)
	//};
	result += SpiralOfFate::Vector2f{
		data.frame->texWidth * scale.x / 2,
		data.frame->texHeight * scale.y / 2
	};
	result += translate;
	this->_sprite.setOrigin(bounds / 2);
	if (data.frame->hasBlendOptions()) {
		this->_sprite.setRotation(data.frame->blendOptions.angle);
		this->_sprite.setPosition(result.rotation(data.frame->blendOptions.angle * M_PI / 180, {0, 0}));
	} else {
		this->_sprite.setRotation(0);
		this->_sprite.setPosition(result);
	}
	this->_sprite.setScale(scale);
	this->_sprite.textureHandle = data.textureHandle;
	this->_sprite.setTextureRect(texBounds);
	SpiralOfFate::game->textureMgr.setTexture(this->_sprite);
	if (!data.frame->hasBlendOptions() || data.frame->blendOptions.mode == 0)
		SpiralOfFate::game->screen->draw(this->_sprite);
	else if (data.frame->blendOptions.mode == 1)
		SpiralOfFate::game->screen->draw(this->_sprite, sf::BlendAdd);
	else if (data.frame->blendOptions.mode == 2)
		SpiralOfFate::game->screen->draw(this->_sprite, sf::BlendMode{
			sf::BlendMode::Zero,
			sf::BlendMode::OneMinusSrcColor,
			sf::BlendMode::Add
		});
	else
		SpiralOfFate::game->screen->draw(this->_sprite);

	rect.setOutlineThickness(1);
	rect.setOutlineColor(sf::Color::White);
	rect.setFillColor(sf::Color::Black);
	rect.setPosition(this->_position - SpiralOfFate::Vector2f{2, 2});
	rect.setSize({4, 4});
	SpiralOfFate::game->screen->draw(rect);
}

void EditableObject::update()
{
	auto &act = this->_moves.at(this->_action);
	auto &block = act[this->_actionBlock];
	auto *data = &block[this->_animation];

	this->_animationCtr++;
	while (this->_animationCtr >= data->frame->duration) {
		this->_animationCtr = 0;
		this->_animation++;
		if (this->_animation >= block.size()) {
			if (!data->parent->loop)
				this->_actionBlock = (this->_actionBlock + 1) % act.size();
			this->_animation = 0;
		}
		data = &act[this->_actionBlock][this->_animation];
		SpiralOfFate::game->soundMgr.play(data->hitSoundHandle);
		if (!data->frame->duration)
			break;
	}
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

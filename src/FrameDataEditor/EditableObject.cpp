//
// Created by PinkySmile on 18/09/2021
//

#include "EditableObject.hpp"

EditableObject::EditableObject(const std::string &chr, const ShadyCore::Schema &schema, const ShadyCore::Palette &palette, const std::string &palName)
{
	this->_moves = SpiralOfFate::FrameData::loadSchema(chr, schema, palette, palName);
	this->_action = this->_moves.begin()->first;
}

void EditableObject::render(bool setup) const
{
	sf::RectangleShape rect;

	if (setup)
		this->setupSprite();

	auto &data = *this->_setuped;

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
	rect.setPosition(SpiralOfFate::Vector2f{-2, -2});
	rect.setSize({4, 4});
	SpiralOfFate::game->screen->draw(rect);
}

std::pair<SpiralOfFate::Vector2i, SpiralOfFate::Vector2u> EditableObject::setupSprite() const
{
	auto &data = this->_moves.at(this->_action)[this->_actionBlock][this->_animation];
	auto translate = this->displayScaled ? this->translate : SpiralOfFate::Vector2f{0, 0};
	auto s = this->displayScaled ? this->scale : 1.f;

	if (data.needReload)
		data.reloadTexture();
	this->_setuped = &data;

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
	result += SpiralOfFate::Vector2f{
		data.frame->texWidth * scale.x / 2,
		data.frame->texHeight * scale.y / 2
	};
	result += translate;
	this->_sprite.setOrigin(bounds / 2);
	if (data.frame->hasBlendOptions()) {
		auto pos = result;
		auto trueScale = scale;

		this->_sprite.setRotation(data.frame->blendOptions.angle);
		this->_sprite.setColor(SokuColor(data.frame->blendOptions.color));

		// X rotation
		pos.y *= std::cos(data.frame->blendOptions.flipVert * M_PI / 180);
		trueScale.y *= std::cos(data.frame->blendOptions.flipVert * M_PI / 180);

		// Y rotation
		pos.x *= std::cos(data.frame->blendOptions.flipHorz * M_PI / 180);
		trueScale.x *= std::cos(data.frame->blendOptions.flipHorz * M_PI / 180);

		pos.rotate(data.frame->blendOptions.angle * M_PI / 180, {0, 0});
		this->_sprite.setPosition(pos);
		this->_sprite.setScale(trueScale);
	} else {
		this->_sprite.setRotation(0);
		this->_sprite.setPosition(result);
		this->_sprite.setColor(sf::Color::White);
		this->_sprite.setScale(scale);
	}
	this->_sprite.textureHandle = data.textureHandle;
	this->_sprite.setTextureRect(texBounds);
	SpiralOfFate::game->textureMgr.setTexture(this->_sprite);

	auto size = bounds;

	size.x *= scale.x;
	size.y *= scale.y;

	auto angle = this->_sprite.getRotation() * M_PI / 180;
	auto aroundSize = SpiralOfFate::Vector2d{
		std::abs(size.x * cos(angle)) + std::abs(size.y * sin(angle)),
		std::abs(size.y * cos(angle)) + std::abs(size.x * sin(angle))
	};

	return {
		{
			static_cast<int>(this->_sprite.getPosition().x - aroundSize.x / 2),
			static_cast<int>(this->_sprite.getPosition().y - aroundSize.y / 2)
		},
		aroundSize.to<unsigned>()
	};
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

void EditableObject::render() const
{
	this->render(true);
}

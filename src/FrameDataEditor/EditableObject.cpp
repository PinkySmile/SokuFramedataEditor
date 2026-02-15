//
// Created by PinkySmile on 18/09/2021.
//

#include <filesystem>
#include "Color.hpp"
#include "EditableObject.hpp"
#include "PngLoader.hpp"
#include "Resources/Assert.hpp"

using namespace SpiralOfFate;

struct SokuColor {
	union {
		struct {
			unsigned char a;
			unsigned char r;
			unsigned char g;
			unsigned char b;
		};
		unsigned color;
	};

	SokuColor(unsigned c) : color(c) {}
	SokuColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a) : a(a), r(r), g(g), b(b) {}
	operator sf::Color() { return {r, g, b, a}; }
};

EditableObject::EditableObject(const std::string &folder, const std::string &frameData, const std::array<Color, 256> *palette) :
	_folder(folder)
{
	this->_schema = FrameData::loadFile(frameData, folder, palette);
}

EditableObject::EditableObject(const std::string &folder, const std::filesystem::path &frameData, const std::array<Color, 256> *palette) :
	_folder(folder)
{
	this->_schema = FrameData::loadFile(frameData, folder, palette);
}

void EditableObject::render(sf::RenderTarget &target, sf::RenderStates states)
{
	auto &data = this->_schema.framedata.at(this->_action)[this->_actionBlock][this->_animation];
	Vector2f scale;

	data.checkReloadTexture();
	if (data.renderGroup == 0) {
		scale.x = 2;
		scale.y = 2;
	} else if (data.renderGroup == 2) {
		scale.x = data.blendOptions.scaleX / 100.f;
		scale.y = data.blendOptions.scaleY / 100.f;
	} else {
		scale.x = 1;
		scale.y = 1;
	}
	Vector2f scaleReal{
		(data.hasBlendOptions() ? data.blendOptions.scaleX : 100) / 100.f,
		(data.hasBlendOptions() ? data.blendOptions.scaleY : 100) / 100.f
	};
	Vector2f bounds{
		static_cast<float>(data.texWidth),
		static_cast<float>(data.texHeight)
	};
	IntRect texBounds{
		{data.texOffsetX, data.texOffsetY},
		{data.texWidth, data.texHeight}
	};
	Vector2f result{
		static_cast<float>(-data.offsetX) * scaleReal.x,
		static_cast<float>(-data.offsetY) * scaleReal.y
	};
	result += Vector2f{
		data.texWidth * scale.x / 2,
		data.texHeight * scale.y / 2
	};
	this->_sprite.setOrigin(bounds / 2);
	if (data.hasBlendOptions()) {
		auto pos = result;
		auto trueScale = scale;

		this->_sprite.setRotation(sf::degrees(data.blendOptions.angle));
		this->_sprite.setColor(SokuColor(data.blendOptions.color));

		// X rotation
		pos.y *= std::cos(data.blendOptions.flipVert * M_PI / 180);
		trueScale.y *= std::cos(data.blendOptions.flipVert * M_PI / 180);

		// Y rotation
		pos.x *= std::cos(data.blendOptions.flipHorz * M_PI / 180);
		trueScale.x *= std::cos(data.blendOptions.flipHorz * M_PI / 180);

		pos.rotate(data.blendOptions.angle * M_PI / 180, {0, 0});
		this->_sprite.setPosition(pos);
		this->_sprite.setScale(trueScale);
	} else {
		this->_sprite.setRotation(sf::degrees(0));
		this->_sprite.setPosition(result);
		this->_sprite.setColor(sf::Color::White);
		this->_sprite.setScale(scale);
	}
	this->_sprite.setTexture(data.textureHandle);
	this->_sprite.setTextureRect(texBounds);

	if (!data.hasBlendOptions() || data.blendOptions.mode == 0)
		target.draw(this->_sprite, states);
	else if (data.blendOptions.mode == 1) {
		states.blendMode = sf::BlendAdd;
		target.draw(this->_sprite, states);
	} else if (data.blendOptions.mode == 2) {
		states.blendMode = {
			sf::BlendMode::Factor::Zero,
			sf::BlendMode::Factor::OneMinusSrcColor,
			sf::BlendMode::Equation::Add
		};
		target.draw(this->_sprite, states);
	} else
		target.draw(this->_sprite, states);

	if (this->_textureValid) {
		this->_sprite.setTexture(this->_overlayTexture);
		target.draw(this->_sprite, states);
	}
}

void EditableObject::update()
{
	auto &action = this->_schema.framedata.at(this->_action);
	auto &seq = action[this->_actionBlock];
	auto *data = &seq[this->_animation];

	this->_animationCtr++;
	while (this->_animationCtr >= data->duration) {
		this->_animationCtr = 0;
		this->_animation++;
		if (this->_animation >= seq.size()) {
			this->_animation = 0;
			if (!seq.loop) {
				this->_actionBlock++;
				this->_actionBlock %= action.size();
			}
		}
		data = &action[this->_actionBlock][this->_animation];
		this->resetState();
		this->_needGenerate = false;
		if (this->_mousePos)
			this->setMousePosition(&*this->_mousePos);
		this->_generateOverlaySprite();
	}
}

FrameData &EditableObject::getFrameData()
{
	return this->_schema.framedata.at(this->_action).at(this->_actionBlock).at(this->_animation);
}

const FrameData &EditableObject::getFrameData() const
{
	return this->_schema.framedata.at(this->_action).at(this->_actionBlock).at(this->_animation);
}

FrameData::Sequence &EditableObject::getSequence()
{
	return this->_schema.framedata.at(this->_action).at(this->_actionBlock);
}

const FrameData::Sequence &EditableObject::getSequence() const
{
	return this->_schema.framedata.at(this->_action).at(this->_actionBlock);
}

std::vector<FDE::Rectangle> EditableObject::_getModifiedBoxes(const FrameData &data, const std::vector<ShadyCore::Schema::Sequence::BBox> &boxes) const
{
	std::vector<FDE::Rectangle> result;
	//Vector2f center{
	//	static_cast<float>(data.offsetX),
	//	data.texHeight * data.scale.y / -2.f - data.offset.y
	//};
	Vector2f center = {0, 0};;
	auto rotation = data.getBlendOptions().angle * M_PI / 180;
	Vector2f real = {0, 0};

	result.reserve(boxes.size());
	for (auto &_box : boxes) {
		Vector2i pos{ _box.left, _box.up };
		Vector2u size{
			static_cast<unsigned>(_box.right - _box.left),
			static_cast<unsigned>(_box.down - _box.up)
		};

		result.push_back({
			.pt1 = real + pos.rotation(rotation, center),
			.pt2 = real + (pos + Vector2f{0, static_cast<float>(size.y)}).rotation(rotation, center),
			.pt3 = real + (pos + size.to<int>()).rotation(rotation, center),
			.pt4 = real + (pos + Vector2f{static_cast<float>(size.x), 0}).rotation(rotation, center)
		});
	}
	return result;
}

std::vector<FDE::Rectangle> EditableObject::_getModifiedHurtBoxes() const
{
	return this->_getModifiedBoxes(this->getFrameData(), this->getFrameData().hBoxes);
}

std::vector<FDE::Rectangle> EditableObject::_getModifiedHitBoxes() const
{
	return this->_getModifiedBoxes(this->getFrameData(), this->getFrameData().aBoxes);
}

void EditableObject::resetState()
{
	this->_generateOverlaySprite();
}

void EditableObject::tick()
{
	if (this->_needGenerate)
		this->_generateOverlaySprite();
}

void EditableObject::_generateOverlaySprite()
{
	this->_textureValid = false;
	if (this->_paletteIndex <= 0)
		return;

	auto &data = this->_schema.framedata.at(this->_action)[this->_actionBlock][this->_animation];
	auto &img = pngLoader.loadImage(data.__folder + data.spritePath);

	if (img.bitsPerPixel != 8)
		return;
	data.checkReloadTexture();

	sf::Image image;
	auto &pal = game->textureMgr.getPalette(data.textureHandle);
	auto color = pal[this->_paletteIndex];

	color.r = 255 - color.r;
	color.g = 255 - color.g;
	color.b = 255 - color.b;
	image.resize({data.texWidth, data.texHeight}, sf::Color::Transparent);
	for (unsigned y_off = 0; y_off < data.texHeight; y_off++) {
		int y = data.texOffsetY + y_off;

		if (y < 0)
			continue;
		if (static_cast<unsigned>(y) >= img.height)
			break;
		for (unsigned x_off = 0; x_off < data.texHeight; x_off++) {
			int x = data.texOffsetX + x_off;

			if (x < 0)
				continue;
			if (static_cast<unsigned>(x) >= img.width)
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

void EditableObject::setMousePosition(const Vector2f *pos)
{
	if (pos == nullptr) {
		this->_mousePos.reset();
		if (this->_onHoverChange)
			this->_onHoverChange(this->_paletteIndex, -1);
		this->_paletteIndex = -1;
		this->_textureValid = false;
		return;
	}
	this->_mousePos = *pos;

	auto sPos = this->_mousePosToImgPos(*pos);
	auto &data = this->_schema.framedata.at(this->_action)[this->_actionBlock][this->_animation];

	if (sPos.x < 0 || sPos.y < 0 || sPos.x >= data.texWidth || sPos.y >= data.texHeight) {
		if (this->_onHoverChange)
			this->_onHoverChange(this->_paletteIndex, -1);
		this->_paletteIndex = -1;
		this->_textureValid = false;
		return;
	}

	auto &img = pngLoader.loadImage(data.__folder+ data.spritePath);

	if (img.bitsPerPixel != 8) {
		if (this->_onHoverChange)
			this->_onHoverChange(this->_paletteIndex, -1);
		this->_paletteIndex = -1;
		this->_textureValid = false;
		return;
	}
	sPos.x += data.texOffsetX;
	sPos.y += data.texOffsetY;
	if (sPos.x < 0 || sPos.x >= img.width || sPos.y < 0 || sPos.y >= img.height)
		return;

	unsigned nColor = img.raw[static_cast<int>(sPos.y) * img.paddedWidth + static_cast<int>(sPos.x)];

	if (nColor == 0) {
		if (this->_onHoverChange)
			this->_onHoverChange(this->_paletteIndex, -1);
		this->_paletteIndex = -1;
		this->_textureValid = false;
		return;
	}
	if (this->_onHoverChange)
		this->_onHoverChange(this->_paletteIndex, nColor);
	this->_paletteIndex = nColor;
	this->_needGenerate = this->_needGenerate || this->_oldPaletteIndex != this->_paletteIndex;
	this->_oldPaletteIndex = this->_paletteIndex;
}

Vector2f EditableObject::_mousePosToImgPos(const Vector2i &mouse)
{
	auto &data = this->_schema.framedata.at(this->_action)[this->_actionBlock][this->_animation];
	Vector2f scale;

	data.checkReloadTexture();
	if (data.renderGroup == 0) {
		scale.x = 2;
		scale.y = 2;
	} else if (data.renderGroup == 2) {
		scale.x = data.blendOptions.scaleX / 100.f;
		scale.y = data.blendOptions.scaleY / 100.f;
	} else {
		scale.x = 1;
		scale.y = 1;
	}
	Vector2f scaleReal{
		(data.hasBlendOptions() ? data.blendOptions.scaleX : 100) / 100.f,
		(data.hasBlendOptions() ? data.blendOptions.scaleY : 100) / 100.f
	};
	Vector2f bounds{
		static_cast<float>(data.texWidth),
		static_cast<float>(data.texHeight)
	};
	Vector2f result{
		static_cast<float>(-data.offsetX) * scaleReal.x,
		static_cast<float>(-data.offsetY) * scaleReal.y
	};
	result += Vector2f{
		data.texWidth * scale.x / 2,
		data.texHeight * scale.y / 2
	};
	if (!data.hasBlendOptions()) {
		Vector2f p = mouse;

		p -= result;
		p.x /= scale.x;
		p.y /= scale.y;
		p += bounds / 2;
		p.y -= 0.5;
		return p;
	}

	auto pos = result;
	auto trueScale = scale;

	// X rotation
	pos.y *= std::cos(data.blendOptions.flipVert * M_PI / 180);
	trueScale.y *= std::cos(data.blendOptions.flipVert * M_PI / 180);

	// Y rotation
	pos.x *= std::cos(data.blendOptions.flipHorz * M_PI / 180);
	trueScale.x *= std::cos(data.blendOptions.flipHorz * M_PI / 180);

	pos.rotate(data.blendOptions.angle * M_PI / 180, {0, 0});

	auto r = mouse - (pos - bounds / 2);

	r.x /= trueScale.x;
	r.y /= trueScale.y;
	r += bounds / 2;
	r.rotate(-data.blendOptions.angle * M_PI / 180, bounds / 2);
	return r;
}

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

	auto size = bounds;

	size.x *= scale.x;
	size.y *= scale.y;

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

	sf::RectangleShape rect;
	rect.setOutlineThickness(1);
	rect.setScale(scale);
	rect.setOutlineColor(Color::White);
	rect.setFillColor(Color::Transparent);
	rect.setPosition(this->_sprite.getPosition());
	rect.setOrigin({data.texWidth / 2.f, data.texHeight / 2.f});
	rect.setSize({static_cast<float>(data.texWidth), static_cast<float>(data.texHeight)});
	target.draw(rect, states);

	rect.setOutlineThickness(2);
	rect.setOutlineColor(Color::White);
	rect.setFillColor(Color::Black);
	rect.setOrigin({0, 0});
	rect.setScale({1, 1});
	rect.setRotation(sf::radians(0));
	rect.setPosition(Vector2f{-4, -4});
	rect.setSize({9, 9});
	target.draw(rect, states);
}

void EditableObject::update()
{
	auto *data = &this->_schema.framedata.at(this->_action)[this->_actionBlock][this->_animation];

	this->_animationCtr++;
	while (this->_animationCtr >= data->duration) {
		this->_animationCtr = 0;
		this->_animation++;
		this->_animation %= this->_schema.framedata.at(this->_action)[this->_actionBlock].size();
		data = &this->_schema.framedata.at(this->_action)[this->_actionBlock][this->_animation];
		this->resetState();
		this->_needGenerate = false;
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

std::vector<Rectangle> EditableObject::_getModifiedBoxes(const FrameData &data, const std::vector<ShadyCore::Schema::Sequence::BBox> &boxes) const
{
	std::vector<Rectangle> result;
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

std::vector<Rectangle> EditableObject::_getModifiedHurtBoxes() const
{
	return this->_getModifiedBoxes(this->getFrameData(), this->getFrameData().hBoxes);
}

std::vector<Rectangle> EditableObject::_getModifiedHitBoxes() const
{
	return this->_getModifiedBoxes(this->getFrameData(), this->getFrameData().aBoxes);
}

void EditableObject::resetState()
{
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
		this->_paletteIndex = -1;
		this->_textureValid = false;
		return;
	}

	auto sPos = this->_mousePosToImgPos(*pos);
	auto &data = this->_schema.framedata.at(this->_action)[this->_actionBlock][this->_animation];

	if (sPos.x < 0 || sPos.y < 0 || sPos.x >= data.texWidth || sPos.y >= data.texHeight) {
		this->_paletteIndex = -1;
		this->_textureValid = false;
		return;
	}

	auto &img = pngLoader.loadImage(data.__folder+ data.spritePath);

	if (img.bitsPerPixel != 8) {
		this->_paletteIndex = -1;
		this->_textureValid = false;
		return;
	}
	sPos.x += data.texOffsetX;
	sPos.y += data.texOffsetY;
	if (sPos.x < 0 || sPos.x >= img.width || sPos.y < 0 || sPos.y >= img.height)
		return;
	this->_paletteIndex = img.raw[static_cast<int>(sPos.y) * img.paddedWidth + static_cast<int>(sPos.x)];
	this->_needGenerate = true;
}

Vector2f EditableObject::_mousePosToImgPos(const Vector2i &mouse)
{
	auto &data = this->_schema.framedata.at(this->_action)[this->_actionBlock][this->_animation];
	auto blend = data.getBlendOptions();
	auto size = Vector2f{
		blend.scaleX * data.texWidth / 100.f,
		blend.scaleY * data.texHeight / 100.f
	};
	Vector2f result = {
		static_cast<float>(data.offsetX),
		static_cast<float>(data.offsetY)
	};

	result.y *= -1;
	result -= Vector2f{
		size.x / 2.f,
		size.y
	};
	result = mouse - result;
	result.x /= blend.scaleX / 100.f;
	result.y /= blend.scaleY / 100.f;
	this->_mousePos = result;
	return result;
}

//
// Created by PinkySmile on 26/10/2025.
//

#include <cmath>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <TGUI/Backend/Renderer/SFML-Graphics/BackendRenderTargetSFML.hpp>
#include <utility>
#include "ColorSliderWidget.hpp"
#include "Resources/Assert.hpp"

ColorSliderWidget::ColorSliderWidget(
	ColorConversionCb colorConversion,
	ColorSpaceRanges colorSpace,
	unsigned char component,
	bool dir
) :
	tgui::ClickableWidget(ColorSliderWidget::StaticWidgetType),
	_colorConversion(std::move(colorConversion)),
	_colorSpace(std::move(colorSpace)),
	_component(component),
	_dir(dir)
{
	assert_exp(this->_component < this->_color.size());
	assert_exp(this->_selector.loadFromFile("assets/gui/editor/selectorBar.png"));
	this->_selectorSprite.setTexture(this->_selector, true);
	this->_selectorSprite.setOrigin({
		std::floor(this->_selector.getSize().x / 2.f),
		std::floor(this->_selector.getSize().y / 2.f)
	});
	this->_sprite.setPosition({0, 10});
}

void ColorSliderWidget::setColorSpace(ColorConversionCb colorConversion, ColorSpaceRanges colorSpace)
{
	this->_colorSpace = std::move(colorSpace);
	this->_colorConversion = std::move(colorConversion);
	this->_recreateTexture();
	this->_setPointerPosition();
}

void ColorSliderWidget::draw(tgui::BackendRenderTarget &target, tgui::RenderStates states) const
{
	auto &sfmlTarget = dynamic_cast<tgui::BackendRenderTargetSFML &>(target);
	const std::array<float, 16> &transformMatrix = states.transform.getMatrix();
	sf::RenderStates statesSFML;
	auto realTarget = sfmlTarget.getTarget();

	tgui::Vector2f size = this->getSize();

	if (size == this->_lastSize) {
		this->_lastSize = size;
		this->_recreateTexture();
		this->_setPointerPosition();
	}
	statesSFML.transform = sf::Transform(
		transformMatrix[0], transformMatrix[4], transformMatrix[12],
		transformMatrix[1], transformMatrix[5], transformMatrix[13],
		transformMatrix[3], transformMatrix[7], transformMatrix[15]
	);
	statesSFML.coordinateType = sf::CoordinateType::Normalized;
	realTarget->draw(this->_sprite, statesSFML);
	realTarget->draw(this->_selectorSprite, statesSFML);
}

void ColorSliderWidget::_recreateTexture() const
{
	tgui::Vector2f size = this->getSize();
	sf::Vector2u actualSize = {
		static_cast<unsigned int>(std::ceil(size.x)),
		static_cast<unsigned int>(std::ceil(size.y - 20))
	};
	sf::Image image{actualSize};
	unsigned computed[3] = {
		this->_color[0],
		this->_color[1],
		this->_color[2]
	};

	for (unsigned y = 0; y < actualSize.y; y++)
		for (unsigned x = 0; x < actualSize.x; x++) {
			auto &first = this->_colorSpace[this->_component];

			computed[this->_component] = first.first + (first.second - first.first) * (this->_dir ? y : x) / (this->_dir ? actualSize.y : actualSize.x);

			// TODO: Optimize by caching last color and last line
			sf::Color color = this->_colorConversion(computed[0], computed[1], computed[2]);

			image.setPixel({x, y}, color);
		}
	assert_exp(this->_space.loadFromImage(image));
	this->_sprite.setTexture(this->_space, true);
}

ColorSliderWidget::Ptr ColorSliderWidget::create(
	ColorConversionCb colorConversion,
	ColorSpaceRanges colorSpace,
	unsigned char component,
	bool dir,
	const tgui::Layout2d &size
)
{
	auto widget = std::make_shared<ColorSliderWidget>(std::move(colorConversion), std::move(colorSpace), component, dir);

	widget->setSize(size);
	return widget;
}

void ColorSliderWidget::setColor(WidgetColor color)
{
	this->_color = std::move(color);
	// TODO: Check if actually need to recreate texture
	this->_recreateTexture();
	this->_setPointerPosition();
}

WidgetColor ColorSliderWidget::getColor() const
{
	return this->_color;
}

sf::Color ColorSliderWidget::getColorRGB() const
{
	return this->_colorConversion(this->_color[0], this->_color[1], this->_color[2]);
}

void ColorSliderWidget::setComponent(unsigned char component, unsigned int v)
{
	assert_exp(component < this->_color.size());
	this->_color[component] = v;
	if (component != this->_component)
		this->_recreateTexture();
	else
		this->_setPointerPosition();
}

void ColorSliderWidget::mouseMovedAbs(tgui::Vector2f pos)
{
	if (!this->_dragging)
		return;

	auto size = this->getSize();
	float actualSize = size.y - 20;
	float planePos = pos.y - 10;
	auto &c = this->_colorSpace[this->_component];
	sf::Vector2f bounds{static_cast<float>(c.first), static_cast<float>(c.second)};

	if (planePos < 0)
		planePos = 0;
	if (planePos >= actualSize)
		planePos = actualSize;
	this->_color[this->_component] = static_cast<unsigned>(bounds.x + planePos * (bounds.y - bounds.x) / actualSize);
	this->_setPointerPosition();
	this->onChanged.emit(this, this->_color);
}

void ColorSliderWidget::leftMouseButtonNoLongerDown()
{
	if (this->_dragging)
		this->onChangeEnded.emit(this);
	this->_dragging = false;
	Widget::leftMouseButtonNoLongerDown();
}

bool ColorSliderWidget::leftMousePressed(tgui::Vector2f pos)
{
	this->_dragging = true;
	this->onChangeStarted.emit(this);

	auto size = this->getSize();
	float actualSize = size.y - 20;
	float planePos = pos.y - 10;
	auto &c = this->_colorSpace[this->_component];

	if (planePos < 0)
		planePos = 0;
	if (planePos >= actualSize)
		planePos = actualSize;
	this->_color[this->_component] = (float)c.first + planePos * ((float)c.second - (float)c.first) / actualSize;
	this->_setPointerPosition();
	this->onChanged.emit(this, this->_color);
	return ClickableWidget::leftMousePressed(pos);
}

void ColorSliderWidget::_setPointerPosition() const
{
	sf::Vector2f pos = this->getSize();

	pos.x /= 2;
	pos.y -= 20;
	pos.y *= (this->_color[this->_component]- this->_colorSpace[this->_component].first)/ (float)this->_colorSpace[this->_component].second;
	pos += this->_sprite.getPosition();
	this->_selectorSprite.setPosition(pos);
}

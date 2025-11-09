//
// Created by PinkySmile on 26/10/2025.
//

#include <cmath>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <TGUI/Backend/Renderer/SFML-Graphics/BackendRenderTargetSFML.hpp>
#include <utility>
#include "ColorPlaneWidget.hpp"
#include "Resources/Assert.hpp"

ColorPlaneWidget::ColorPlaneWidget(
	ColorConversionCb colorConversion,
	ColorSpaceRanges colorSpace,
	std::pair<unsigned char, unsigned char> components
) :
	tgui::ClickableWidget(ColorPlaneWidget::StaticWidgetType),
	_colorConversion(std::move(colorConversion)),
	_colorSpace(std::move(colorSpace)),
	_components(std::move(components))
{
	assert_exp(this->_components.first < this->_color.size());
	assert_exp(this->_components.second < this->_color.size());
	assert_exp(this->_cross.loadFromFile("assets/gui/editor/cross.png"));
	if (this->_components.first != 0 && this->_components.second != 0)
		this->_lastComponent = 0;
	else if (this->_components.first != 1 && this->_components.second != 1)
		this->_lastComponent = 1;
	else
		this->_lastComponent = 2;
	this->_sprite.setPosition({10, 10});
	this->_crossSprite.setTexture(this->_cross, true);
	this->_crossSprite.setOrigin({this->_cross.getSize().x / 2.f, this->_cross.getSize().y / 2.f});
}

void ColorPlaneWidget::setColorSpace(ColorConversionCb colorConversion, ColorSpaceRanges colorSpace)
{
	this->_colorSpace = std::move(colorSpace);
	this->_colorConversion = std::move(colorConversion);
	this->_recreateTexture();
	this->_setPointerPosition();
}

void ColorPlaneWidget::draw(tgui::BackendRenderTarget &target, tgui::RenderStates states) const
{
	auto &sfmlTarget = dynamic_cast<tgui::BackendRenderTargetSFML &>(target);
	const std::array<float, 16> &transformMatrix = states.transform.getMatrix();
	sf::RenderStates statesSFML;
	auto realTarget = sfmlTarget.getTarget();
	tgui::Vector2f size = this->getSize();

	if (size != this->_lastSize) {
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
	realTarget->draw(this->_crossSprite, statesSFML);
}

void ColorPlaneWidget::_recreateTexture() const
{
	tgui::Vector2f size = this->getSize();
	sf::Vector2u actualSize = {
		static_cast<unsigned int>(std::ceil(size.x - 20)),
		static_cast<unsigned int>(std::ceil(size.y - 20))
	};
	sf::Image image{actualSize};
	unsigned computed[3];
	unsigned *components[3];

	components[this->_components.first] = &computed[0];
	components[this->_components.second] = &computed[1];
	components[this->_lastComponent] = &computed[2];
	computed[2] = this->_color[2];
	for (unsigned y = 0; y < actualSize.y; y++)
		for (unsigned x = 0; x < actualSize.x; x++) {
			auto &first = this->_colorSpace[this->_components.first];
			auto &second = this->_colorSpace[this->_components.second];

			computed[0] = first.first  + (first.second  - first.first)  * x / actualSize.x;
			computed[1] = second.first + (second.second - second.first) * y / actualSize.y;

			// TODO: Optimize by caching last color and last line
			sf::Color color = this->_colorConversion(
				*components[0],
				*components[1],
				*components[2]
			);

			image.setPixel({x, y}, color);
		}
	assert_exp(this->_space.loadFromImage(image));
	this->_sprite.setTexture(this->_space, true);
}

ColorPlaneWidget::Ptr ColorPlaneWidget::create(
	ColorConversionCb colorConversion,
	ColorSpaceRanges colorSpace,
	std::pair<unsigned char, unsigned char> components,
	const tgui::Layout2d &size
)
{
	auto widget = std::make_shared<ColorPlaneWidget>(std::move(colorConversion), std::move(colorSpace), std::move(components));

	widget->setSize(size);
	return widget;
}

void ColorPlaneWidget::setColor(WidgetColor color)
{
	auto old = this->_color[this->_lastComponent];

	this->_color = std::move(color);
	if (old != this->_color[this->_lastComponent])
		this->_recreateTexture();
	this->_setPointerPosition();
	this->onChanged.emit(this, this->_color);
}

WidgetColor ColorPlaneWidget::getColor() const
{
	return this->_color;
}

sf::Color ColorPlaneWidget::getColorRGB() const
{
	return this->_colorConversion(this->_color[0], this->_color[1], this->_color[2]);
}

void ColorPlaneWidget::setComponent(unsigned char component, unsigned int v)
{
	assert_exp(component < this->_color.size());
	this->_color[component] = v;
	if (component == this->_lastComponent)
		this->_recreateTexture();
	else
		this->_setPointerPosition();
}

void ColorPlaneWidget::mouseMoved(tgui::Vector2f pos)
{
	if (!this->_dragging)
		return;

	auto size = this->getSize();
	sf::Vector2f actualSize = {
		std::ceil(size.x - 20),
		std::ceil(size.y - 20)
	};
	sf::Vector2f planePos = {
		pos.x - 10,
		pos.y - 10
	};
	auto &c1 = this->_colorSpace[this->_components.first];
	auto &c2 = this->_colorSpace[this->_components.second];

	if (planePos.x < 0)
		planePos.x = 0;
	if (planePos.y < 0)
		planePos.y = 0;
	if (planePos.x >= actualSize.x)
		planePos.x = actualSize.x;
	if (planePos.y >= actualSize.y)
		planePos.y = actualSize.y;
	this->_color[this->_components.first] = (float)c1.first + planePos.x * ((float)c1.second - (float)c1.first) / actualSize.x;
	this->_color[this->_components.second]= (float)c2.first + planePos.y * ((float)c2.second - (float)c2.first) / actualSize.y;
	this->_setPointerPosition();
	this->onChanged.emit(this, this->_color);
}

void ColorPlaneWidget::leftMouseButtonNoLongerDown()
{
	if (this->_dragging)
		this->onChangeEnded.emit(this);
	this->_dragging = false;
	Widget::leftMouseButtonNoLongerDown();
}

bool ColorPlaneWidget::leftMousePressed(tgui::Vector2f pos)
{
	this->_dragging = true;
	this->onChangeStarted.emit(this);

	auto size = this->getSize();
	sf::Vector2f actualSize = {
		std::ceil(size.x - 20),
		std::ceil(size.y - 20)
	};
	sf::Vector2f planePos = {
		pos.x - 10,
		pos.y - 10
	};
	auto &c1 = this->_colorSpace[this->_components.first];
	auto &c2 = this->_colorSpace[this->_components.second];
	float min1 = c1.first;
	float min2 = c2.first;
	float max1 = c1.second;
	float max2 = c2.second;

	if (planePos.x < 0)
		planePos.x = 0;
	if (planePos.y < 0)
		planePos.y = 0;
	if (planePos.x >= actualSize.x)
		planePos.x = actualSize.x;
	if (planePos.y >= actualSize.y)
		planePos.y = actualSize.y;
	this->_color[this->_components.first] = min1 + planePos.x * (max1 - min1) / actualSize.x;
	this->_color[this->_components.second]= min2 + planePos.y * (max2 - min2) / actualSize.y;
	this->_setPointerPosition();
	this->onChanged.emit(this, this->_color);
	return ClickableWidget::leftMousePressed(pos);
}

void ColorPlaneWidget::_setPointerPosition() const
{
	sf::Vector2f pos = this->getSize();

	pos.x -= 20;
	pos.y -= 20;
	pos.x *= (this->_color[this->_components.first] - this->_colorSpace[this->_components.first].first) / (float)this->_colorSpace[this->_components.first].second;
	pos.y *= (this->_color[this->_components.second]- this->_colorSpace[this->_components.second].first)/ (float)this->_colorSpace[this->_components.second].second;
	pos += this->_sprite.getPosition();
	this->_crossSprite.setPosition(pos);
}

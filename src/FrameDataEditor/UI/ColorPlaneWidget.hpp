//
// Created by PinkySmile on 26/10/2025.
//

#ifndef SOFGV_COLORPLANEWIDGET_HPP
#define SOFGV_COLORPLANEWIDGET_HPP


#include <array>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <TGUI/Widgets/ClickableWidget.hpp>
#include "ColorWidgets.hpp"

class ColorPlaneWidget : public tgui::ClickableWidget {
public:
	using Ptr = std::shared_ptr<ColorPlaneWidget>; //!< Shared widget pointer
	using ConstPtr = std::shared_ptr<const ColorPlaneWidget>; //!< Shared constant widget pointer

	static constexpr const char StaticWidgetType[] = "ColorPlaneWidget"; //!< Type name of the widget

	ColorPlaneWidget(ColorConversionCb colorConversion, ColorSpaceRanges colorSpace, std::pair<unsigned char, unsigned char> components);
	void draw(tgui::BackendRenderTarget &target, tgui::RenderStates states) const override;
	void setColor(WidgetColor color);
	void setComponent(unsigned char component, unsigned value);
	WidgetColor getColor() const;
	sf::Color getColorRGB() const;
	void setColorSpace(ColorConversionCb colorConversion, ColorSpaceRanges colorSpace);

	void mouseMoved(tgui::Vector2f pos) override;
	void leftMouseButtonNoLongerDown() override;
	bool leftMousePressed(tgui::Vector2f pos) override;

	TGUI_NODISCARD static ColorPlaneWidget::Ptr create(
		ColorConversionCb colorConversion,
		ColorSpaceRanges colorSpace,
		std::pair<unsigned char, unsigned char> components,
		const tgui::Layout2d &size = {"100%", "100%"}
	);

	tgui::SignalTyped<WidgetColor> onChanged = {"Changed"};
	tgui::Signal onChangeStarted = {"ChangedStarted"};
	tgui::Signal onChangeEnded = {"ChangedEnded"};

private:
	std::function<sf::Color(unsigned, unsigned, unsigned)> _colorConversion;
	ColorSpaceRanges _colorSpace;
	WidgetColor _color = {0, 0, 0};
	std::pair<unsigned char, unsigned char> _components;
	mutable tgui::Vector2f _lastSize{0, 0};
	mutable sf::Texture _space;
	mutable sf::Sprite _sprite{this->_space};
	mutable sf::Texture _cross;
	mutable sf::Sprite _crossSprite{this->_cross};
	unsigned char _lastComponent;
	bool _dragging = false;

	void _setPointerPosition() const;
	void _recreateTexture() const;
};


#endif //SOFGV_COLORPLANEWIDGET_HPP

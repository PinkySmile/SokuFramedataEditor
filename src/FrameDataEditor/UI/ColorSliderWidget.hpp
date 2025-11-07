//
// Created by PinkySmile on 26/10/2025.
//

#ifndef SOFGV_COLORSLIDERWIDGET_HPP
#define SOFGV_COLORSLIDERWIDGET_HPP


#include <array>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <TGUI/Widgets/ClickableWidget.hpp>
#include "ColorWidgets.hpp"

class ColorSliderWidget : public tgui::ClickableWidget {
public:
	using Ptr = std::shared_ptr<ColorSliderWidget>; //!< Shared widget pointer
	using ConstPtr = std::shared_ptr<const ColorSliderWidget>; //!< Shared constant widget pointer

	static constexpr const char StaticWidgetType[] = "ColorSliderWidget"; //!< Type name of the widget

	ColorSliderWidget(ColorConversionCb colorConversion, ColorSpaceRanges colorSpace, unsigned char component, bool dir);
	void draw(tgui::BackendRenderTarget &target, tgui::RenderStates states) const override;
	void setColor(WidgetColor color);
	void setComponent(unsigned char component, unsigned value);
	WidgetColor getColor() const;
	sf::Color getColorRGB() const;

	void mouseMoved(tgui::Vector2f pos) override;
	void leftMouseButtonNoLongerDown() override;
	bool leftMousePressed(tgui::Vector2f pos) override;

	TGUI_NODISCARD static ColorSliderWidget::Ptr create(
		ColorConversionCb colorConversion,
		ColorSpaceRanges colorSpace,
		unsigned char component,
		bool dir,
		const tgui::Layout2d &size = {"100%", "100%"}
	);

	tgui::SignalTyped<WidgetColor> onChanged = {"Changed"};
	tgui::Signal onChangeStarted = {"ChangedStarted"};
	tgui::Signal onChangeEnded = {"ChangedEnded"};

private:
	std::function<sf::Color(unsigned, unsigned, unsigned)> _colorConversion;
	ColorSpaceRanges _colorSpace;
	WidgetColor _color = {0, 0, 0};
	unsigned char _component;
	bool _dir;
	bool _dragging = false;
	mutable tgui::Vector2f _lastSize{0, 0};
	mutable sf::Texture _space;
	mutable sf::Sprite _sprite{this->_space};
	mutable sf::Texture _selector;
	mutable sf::Sprite _selectorSprite{this->_selector};

	void _recreateTexture() const;
	void _setPointerPosition() const;
};


#endif //SOFGV_COLORSLIDERWIDGET_HPP

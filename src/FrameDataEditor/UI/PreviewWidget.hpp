//
// Created by PinkySmile on 07/05/25.
//

#ifndef SOFGV_PREVIEWWIDGET_HPP
#define SOFGV_PREVIEWWIDGET_HPP


#include <TGUI/Widgets/ClickableWidget.hpp>
#include "../EditableObject.hpp"

namespace SpiralOfFate
{
	class PreviewWidget : public tgui::ClickableWidget {
	private:
		sf::Texture _stageTexture;
		mutable sf::Sprite _stageSprite;
		const EditableObject &_object;
		bool _dragStarted = false;
		std::vector<size_t> _hoveredBoxes;
		unsigned _boxCounter = 0;
		unsigned _boxHovered = 0;
		unsigned _boxSelected = 0;
		unsigned _cornerSelected = 0;

		void _updateHover(const tgui::Vector2f &pos);
		void _drawBox(const Rectangle &box, const Color &color, sf::RenderStates &states, bool hovered, bool selected, bool rotate) const;
	public:
		typedef std::shared_ptr<PreviewWidget> Ptr; //!< Shared widget pointer
		typedef std::shared_ptr<const PreviewWidget> ConstPtr; //!< Shared constant widget pointer

		bool displayBoxes = true;
		bool displaceObject = true;

		PreviewWidget(const EditableObject &object);
		~PreviewWidget() override = default;

		void frameChanged();
		bool leftMousePressed(tgui::Vector2f pos) override;
		bool scrolled(float delta, tgui::Vector2f pos, bool touch) override;
		void mouseMoved(tgui::Vector2f pos) override;

		void draw(tgui::BackendRenderTarget &target, tgui::RenderStates states) const override;
	};
}


#endif //SOFGV_PREVIEWWIDGET_HPP

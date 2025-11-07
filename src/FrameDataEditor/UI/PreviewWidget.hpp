//
// Created by PinkySmile on 07/05/25.
//

#ifndef SOFGV_PREVIEWWIDGET_HPP
#define SOFGV_PREVIEWWIDGET_HPP


#include <TGUI/Widgets/ClickableWidget.hpp>
#include "../EditableObject.hpp"
#include "../Operations/BoxModificationOperation.hpp"

namespace SpiralOfFate
{
	class MainWindow;
	class FrameDataEditor;

	class PreviewWidget : public tgui::ClickableWidget {
	private:
		sf::Clock _doubleClick;
		sf::Texture _stageTexture;
		mutable sf::Sprite _stageSprite;
		EditableObject &_object;
		const FrameDataEditor &_editor;
		MainWindow &_main;
		bool _commited = false;
		bool _dragStarted = false;
		bool _translateDragStarted = false;
		unsigned char _selectedColor = 0;
		float _scale = 1;
		tgui::Vector2f _translate = {0, 0};
		tgui::Vector2f _lastMousePos;
		tgui::Vector2f _startMousePos;
		std::vector<size_t> _hoveredBoxes;
		Box _boxSaved;
		unsigned _boxCounter = 0;
		unsigned _boxHovered = 0;
		unsigned _boxSelected = 0;
		unsigned _cornerHovered = 0;
		unsigned _cornerSelected = 0;

		void _handleBoxResize(const tgui::Vector2f &pos);
		void _handleBoxMove(const tgui::Vector2f &pos);
		void _updateHover(const tgui::Vector2f &pos);
		void _updateBoxSliderHover(const tgui::Vector2f &pos, tgui::Cursor::Type &cursor);
		void _updateBoxSliderHover(const Rectangle &box, const tgui::Vector2f &pos, bool rotate, tgui::Cursor::Type &cursor);
		void _drawBoxBorder(const Rectangle &box, sf::RenderStates &states, bool rotate) const;
		void _drawBox(const Rectangle &box, const Color &color, sf::RenderStates &states, bool hovered, bool selected) const;
	public:
		tgui::SignalTyped2<BoxType, unsigned> onBoxSelect = {"BoxSelected"};
		tgui::Signal onBoxUnselect = {"BoxUnselected"};
		tgui::SignalUInt onColorSelect = {"ColorSelected"};

		typedef std::shared_ptr<PreviewWidget> Ptr; //!< Shared widget pointer
		typedef std::shared_ptr<const PreviewWidget> ConstPtr; //!< Shared constant widget pointer

		static constexpr const char StaticWidgetType[] = "FDPreviewWidget"; //!< Type name of the widget

		bool displayBoxes = true;
		bool displaceObject = true;
		bool showingPalette = false;

		PreviewWidget(const FrameDataEditor &editor, MainWindow &main, EditableObject &object);
		~PreviewWidget() override = default;

		std::pair<BoxType, unsigned> getSelectedBox();
		Box *getSelectedBoxRef();
		void frameChanged();
		bool leftMousePressed(tgui::Vector2f pos) override;
		void leftMouseButtonNoLongerDown() override;
		bool scrolled(float delta, tgui::Vector2f pos, bool touch) override;
		void mouseMoved(tgui::Vector2f pos) override;
		void draw(tgui::BackendRenderTarget &target, tgui::RenderStates states) const override;
		void mouseNoLongerOnWidget() override;
		void keyPressed(const tgui::Event::KeyEvent &event) override;
		bool canHandleKeyPress(const tgui::Event::KeyEvent &event) override;
		void setSelectedColor(unsigned char index);
		unsigned char getSelectedColor() const;
		void setPalette(const std::array<Color, 256> *palette);
		void invalidatePalette();
	};
}


#endif //SOFGV_PREVIEWWIDGET_HPP

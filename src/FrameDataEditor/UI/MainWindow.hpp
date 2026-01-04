//
// Created by PinkySmile on 05/05/25.
//

#ifndef SOFGV_MAINWINDOW_HPP
#define SOFGV_MAINWINDOW_HPP


#include <TGUI/Widgets/ChildWindow.hpp>
#include "LocalizedContainer.hpp"
#include "PreviewWidget.hpp"
#include "../Operations/Operation.hpp"
#include "ColorPlaneWidget.hpp"
#include "ColorSliderWidget.hpp"

namespace SpiralOfFate
{
	class MainWindow : public LocalizedContainer<tgui::ChildWindow> {
	public:
		class Renderer : public tgui::ChildWindowRenderer {
		public:
			using tgui::ChildWindowRenderer::ChildWindowRenderer;

			void setTitleColorFocused(tgui::Color color);
			TGUI_NODISCARD tgui::Color getTitleColorFocused() const;
			void setTitleBarColorFocused(tgui::Color color);
			TGUI_NODISCARD tgui::Color getTitleBarColorFocused() const;
			void setTextureTitleBarFocused(const tgui::Texture &texture);
			TGUI_NODISCARD const tgui::Texture &getTextureTitleBarFocused() const;
			void setCloseButtonFocused(std::shared_ptr<tgui::RendererData> rendererData);
			TGUI_NODISCARD std::shared_ptr<tgui::RendererData> getCloseButtonFocused() const;
			void setMaximizeButtonFocused(std::shared_ptr<tgui::RendererData> rendererData);
			TGUI_NODISCARD std::shared_ptr<tgui::RendererData> getMaximizeButtonFocused() const;
			void setMinimizeButtonFocused(std::shared_ptr<tgui::RendererData> rendererData);
			TGUI_NODISCARD std::shared_ptr<tgui::RendererData> getMinimizeButtonFocused() const;
		};
		struct Palette {
			std::string path;
			std::array<Color, 256> colors;
			bool modified = false;
		};

		using Ptr = std::shared_ptr<MainWindow>; //!< Shared widget pointer
		using ConstPtr = std::shared_ptr<const MainWindow>; //!< Shared constant widget pointer

		static constexpr const char StaticWidgetType[] = "FDEMainWindow"; //!< Type name of the widget

		MainWindow(const std::filesystem::path &frameDataPath, FrameDataEditor &editor);

		Renderer *getSharedRenderer() override;
		const Renderer *getSharedRenderer() const override;
		Renderer *getRenderer() override;

		void mouseMovedAbsolute(tgui::Vector2f pos);
		void undo();
		void redo();
		void copyFrame();
		void pasteFrame();
		void pasteBoxData();
		void pasteAnimData();
		void save();
		void save(const std::filesystem::path &path);
		void setPath(const std::filesystem::path &path);
		void autoSave();
		void applyOperation(Operation *operation);
		void startTransaction(Operation *operation = nullptr);
		void updateTransaction(const std::function<Operation *()> &operation);
		void commitTransaction();
		void cancelTransaction();
		bool isModified() const noexcept;
		bool hasUndoData() const noexcept;
		bool hasRedoData() const noexcept;
		void refreshMenuItems() const;
		void tick();
		void refreshInterface();
		void keyPressed(const tgui::Event::KeyEvent &event) override;
		bool canHandleKeyPress(const tgui::Event::KeyEvent &event) override;

		void navToNextFrame();
		void navToPrevFrame();
		void navToNextBlock();
		void navToPrevBlock();
		void navToNextAction();
		void navToPrevAction();

		void newFrame();
		void newEndFrame();
		void newAnimationBlock();
		void newAction();
		void newHurtBox();
		void newHitBox();

		void removeFrame();
		void removeAnimationBlock();
		void removeAction();
		void removeBox();

		void copyBoxesFromLastFrame();
		void copyBoxesFromNextFrame();
		void flattenThisMoveCollisionBoxes();
		void reloadTextures();
		void invertColors();
		void reversePalette();

		tgui::SignalChildWindow onRealClose = {"RealClosed"}; //!< The window was closed. Optional parameter: pointer to the window

	protected:
		std::set<const tgui::Container *> _containers;
		std::map<const tgui::Container *, std::vector<std::function<void()>>> _updateFrameElements;

		tgui::Color _titleColorFocusedCached;
		tgui::Color _titleColorUnfocusedCached;
		tgui::Color _titleBarColorFocusedCached;
		tgui::Color _titleBarColorUnfocusedCached;
		std::shared_ptr<tgui::RendererData> _closeButtonRendererFocusedCached;
		std::shared_ptr<tgui::RendererData> _closeButtonRendererUnFocusedCached;
		std::shared_ptr<tgui::RendererData> _minimizeButtonRendererFocusedCached;
		std::shared_ptr<tgui::RendererData> _minimizeButtonRendererUnFocusedCached;
		std::shared_ptr<tgui::RendererData> _maximizeButtonRendererFocusedCached;
		std::shared_ptr<tgui::RendererData> _maximizeButtonRendererUnFocusedCached;

		std::string _localizeActionName(unsigned id);
		void _updateTitleButtons();
		void _updateTextureTitleBar();
		void rendererChanged(const tgui::String &property) override;

	private:
		FrameDataEditor &_editor;
		PreviewWidget::Ptr _preview;
		std::vector<ColorPlaneWidget::Ptr> _planes;
		std::vector<ColorSliderWidget::Ptr> _sliders;
		unsigned char _timer = 0;
		bool _paused = true;
		bool _showingPalette = false;
		bool _requireReload = false;
		unsigned char _colorChangeSource = 255;
		std::filesystem::path _path;
		std::filesystem::path _pathBak;
		std::filesystem::path _chrPath;
		std::string _character;
		std::unique_ptr<Operation> _pendingTransaction;
		std::unique_ptr<EditableObject> _object;
		std::vector<std::unique_ptr<Operation>> _operationQueue;
		size_t _operationIndex = 0;
		size_t _operationSaved = 0;
		std::vector<Palette> _palettes;
		unsigned _selectedPalette = 0;
		unsigned _selectColorMethod = 0;
		unsigned _selectedColor = 0;
		nlohmann::json _characterData;

		void _initSidePanel(tgui::Container &panel);
		void _reinitSidePanel(tgui::Container &panel);
		void _createMoveListPopup(const std::function<void(unsigned)> &onConfirm, bool showNotAdded);
		void _placeUIHooks(tgui::Container &container);
		void _createGenericPopup(const std::string &path);
		LocalizedContainer<tgui::ChildWindow>::Ptr _createPopup(const std::string &path);
		void _populateData(const tgui::Container &container);
		void _populateFrameData(const tgui::Container &container);
		void _populateColorData(const tgui::Container &container);
		void _rePopulateData();
		void _rePopulateColorData();
		void _rePopulateFrameData();
	};
}


#endif //SOFGV_MAINWINDOW_HPP

//
// Created by PinkySmile on 05/05/25.
//

#ifndef SOFGV_MAINWINDOW_HPP
#define SOFGV_MAINWINDOW_HPP


#include <TGUI/Widgets/ChildWindow.hpp>
#include "LocalizedContainer.hpp"
#include "../Operations/IOperation.hpp"

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

	public:
		using Ptr = std::shared_ptr<MainWindow>; //!< Shared widget pointer
		using ConstPtr = std::shared_ptr<const MainWindow>; //!< Shared constant widget pointer

		static constexpr const char StaticWidgetType[] = "FDEMainWindow"; //!< Type name of the widget

		MainWindow(const std::string &frameDataPath, const FrameDataEditor &editor);

		Renderer *getSharedRenderer() override;
		const Renderer *getSharedRenderer() const override;
		Renderer *getRenderer() override;

		void undo();
		void redo();
		void save();
		void save(const std::string &path);
		void applyOperation(IOperation *operation);
		bool isModified() const noexcept;
		void tick();

	protected:
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

		void _updateTitleButtons();
		void _updateTextureTitleBar();
		void rendererChanged(const tgui::String &property) override;

	private:
		bool _paused = true;
		std::string _path;
		std::string _character;
		std::unique_ptr<EditableObject> _object;
		std::vector<std::unique_ptr<IOperation>> _operationQueue;
		size_t _operationIndex = 0;
		size_t _operationSaved = 0;

		void _placeUIHooks(const tgui::Container &container);
		void _populateData(const tgui::Container &container);
		void _populateFrameData(const tgui::Container &container);
	};
}


#endif //SOFGV_MAINWINDOW_HPP

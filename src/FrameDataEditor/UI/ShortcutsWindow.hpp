//
// Created by PinkySmile on 03/01/2026.
//

#ifndef SOFGV_SHORTCUTSWINDOW_HPP
#define SOFGV_SHORTCUTSWINDOW_HPP


#include <memory>
#include "LocalizedContainer.hpp"

namespace SpiralOfFate
{
	class ShortcutsWindow : public LocalizedContainer<tgui::ChildWindow> {
	private:
		FrameDataEditor::Shortcut _tmp;
		std::pair<std::string, tgui::Button::Ptr> _changingShortcut;
		std::map<std::string, FrameDataEditor::Shortcut> _shortcuts;

	public:
		using Ptr = std::shared_ptr<ShortcutsWindow>; //!< Shared widget pointer
		using ConstPtr = std::shared_ptr<const ShortcutsWindow>; //!< Shared constant widget pointer

		ShortcutsWindow(FrameDataEditor &editor);
		void keyPressed(const tgui::Event::KeyEvent &event) override;
		bool canHandleKeyPress(const tgui::Event::KeyEvent &event) override;
		void keyReleased(const sf::Event::KeyReleased &event);

		static ShortcutsWindow::Ptr create(FrameDataEditor &editor);
	};
}


#endif //SOFGV_SHORTCUTSWINDOW_HPP
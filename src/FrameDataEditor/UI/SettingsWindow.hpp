//
// Created by PinkySmile on 08/05/25.
//

#ifndef SOFGV_SETTINGSWINDOW_HPP
#define SOFGV_SETTINGSWINDOW_HPP


#include <TGUI/Widgets/ChildWindow.hpp>
#include "LocalizedContainer.hpp"

namespace SpiralOfFate
{
	class SettingsWindow : public LocalizedContainer<tgui::ChildWindow> {
	private:
		std::string _oldLocale;
		std::string _oldTheme;
		bool _saved = false;

		void _resize();

	public:
		using Ptr = std::shared_ptr<SettingsWindow>; //!< Shared widget pointer
		using ConstPtr = std::shared_ptr<const SettingsWindow>; //!< Shared constant widget pointer

		SettingsWindow(FrameDataEditor &editor);

		static SettingsWindow::Ptr create(FrameDataEditor &editor);
	};
}


#endif //SOFGV_SETTINGSWINDOW_HPP

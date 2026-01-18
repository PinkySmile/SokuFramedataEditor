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
		std::filesystem::path _oldSwr;
		std::filesystem::path _oldSoku;
		std::filesystem::path _oldSoku2;
		std::vector<std::filesystem::path> _oldExtra;
		bool _saved = false;

		void _resize();
		bool _checkSwrPackages();
		bool _checkSokuPackages();
		bool _checkSoku2Packages();

	public:
		using Ptr = std::shared_ptr<SettingsWindow>; //!< Shared widget pointer
		using ConstPtr = std::shared_ptr<const SettingsWindow>; //!< Shared constant widget pointer

		SettingsWindow(FrameDataEditor &editor);
		bool checkPackages();

		static SettingsWindow::Ptr create(FrameDataEditor &editor);
	};
}


#endif //SOFGV_SETTINGSWINDOW_HPP

//
// Created by PinkySmile on 06/02/2026.
//

#ifndef SOKUFDEDITOR_FILEDIALOG_HPP
#define SOKUFDEDITOR_FILEDIALOG_HPP


#include <TGUI/Widgets/FileDialog.hpp>

class FileDialog : public tgui::FileDialog {
public:
	using Ptr = std::shared_ptr<FileDialog>; //!< Shared widget pointer
	using ConstPtr = std::shared_ptr<const FileDialog>; //!< Shared constant widget pointer

	static constexpr const char StaticWidgetType[] = "FileDialog2"; //!< Type name of the widget

	FileDialog();

        TGUI_NODISCARD static FileDialog::Ptr create(const tgui::String& title = "Open file", const tgui::String& confirmButtonText = "Open", bool allowCreateFolder = false);
};


#endif //SOKUFDEDITOR_FILEDIALOG_HPP
//
// Created by PinkySmile on 06/02/2026.
//

#include "FileDialog.hpp"

FileDialog::FileDialog()
{
	this->m_comboBoxFileTypes->onItemSelect.disconnectAll();
	this->m_comboBoxFileTypes->onItemSelect([this](int id) {
		TGUI_ASSERT(id >= 0, "There always needs to be a file type filter selected in FileDialog");
		TGUI_ASSERT(static_cast<std::size_t>(id) < m_fileTypeFilters.size(), "Can't select file type filter that doesn't exist");
		this->m_selectedFileTypeFilter = id;
		this->setSelectingDirectory(id == 0);
		this->m_comboBoxFileTypes->setVisible(true);
		this->m_editBoxFilename->setPosition({
			"#TGUI_INTERNAL$ComboBoxFileTypes#.x - #TGUI_INTERNAL$ComboBoxFileTypes#.width - 10",
			this->m_editBoxFilename->getPositionLayout().y
		});
	});
	this->setSelectingDirectory(true);
	this->m_comboBoxFileTypes->setVisible(true);
	this->m_editBoxFilename->setPosition({
		"#TGUI_INTERNAL$ComboBoxFileTypes#.x - #TGUI_INTERNAL$ComboBoxFileTypes#.width - 10",
		this->m_editBoxFilename->getPositionLayout().y
	});
}

FileDialog::Ptr FileDialog::create(const tgui::String &title, const tgui::String &confirmButtonText, bool allowCreateFolder)
{
	auto fileDialog = std::make_shared<FileDialog>();

	fileDialog->setTitle(title);
	fileDialog->setConfirmButtonText(confirmButtonText);
	fileDialog->setAllowCreateFolder(allowCreateFolder);
	return fileDialog;
}

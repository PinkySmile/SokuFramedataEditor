/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// TGUI - Texus' Graphical User Interface
// Copyright (C) 2012-2026 Bruno Van de Velde (vdv_b@tgui.eu)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Altered to change the data source from filesystem to shady-loader packages


#include "PackageFileDialog.hpp"
#include <TGUI/FileDialogIconLoader.hpp>

#include <vector>
#include <map>
#include <ctime>
#include <TGUI/Widgets/Scrollbar.hpp>
// From shady
#include <util/encodingConverter.hpp>
#include "Resources/Game.hpp"

#ifdef TGUI_SYSTEM_WINDOWS
	#include <TGUI/extlibs/IncludeWindows.hpp>
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace tgui
{
	bool SignalStrings::emit(const Widget *widget, const std::vector<String> &paths)
	{
		if (m_handlers.empty())
			return false;

		assert(!paths.empty());
		m_parameters[1] = static_cast<const void*>(&paths[0]);
		m_parameters[2] = static_cast<const void*>(&paths);
		return Signal::emit(widget);
	}

#if TGUI_COMPILED_WITH_CPP_VER < 17
	constexpr const char PackageFileDialog::StaticWidgetType[];
#endif

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	PackageFileDialog::PackageFileDialog(const char* typeName, bool initRenderer) :
		ChildWindow{typeName, false}
	{
		setClientSize({1000, 400});

		m_icons = {
			Texture{ "assets/icons/unknown.png" },
			Texture{ "assets/icons/dialog.png" },
			Texture{ "assets/icons/table.png" },
			Texture{ "assets/icons/musicLoop.png" },
			Texture{ "assets/icons/image.png" },
			Texture{ "assets/icons/palette.png" },
			Texture{ "assets/icons/sfx.png" },
			Texture{ "assets/icons/music.png" },
			Texture{ "assets/icons/chr.png" },
			Texture{ "assets/icons/menu.png" },
			Texture{ "assets/icons/folder.png" }
		};

		m_previewPanel = Panel::create();
		m_previewError = Label::create();
		m_playPause = BitmapButton::create();
		m_playSlider = Slider::create();
		m_playLabel = Label::create();
		m_previewPicture = Picture::create();
		m_previewPalette = Picture::create();
		m_previewPicSize = Label::create();
		m_previewText = TextArea::create();

		m_filesPanel = Panel::create();
		m_buttonBack = Button::create();
		m_buttonForward = Button::create();
		m_buttonUp = Button::create();
		m_editBoxPath = EditBox::create();
		m_listView = ListView::create();
		m_labelFilename = Label::create();
		m_editBoxFilename = EditBox::create();
		m_comboBoxFileTypes = ComboBox::create();
		m_buttonCancel = Button::create();
		m_buttonConfirm = Button::create();

		m_buttonCancel->setText("Cancel");
		m_buttonConfirm->setText("Open");

		m_previewPanel->add(m_playPause, "#TGUI_INTERNAL$PlayPause#");
		m_previewPanel->add(m_playSlider, "#TGUI_INTERNAL$PlaySlider#");
		m_previewPanel->add(m_playLabel, "#TGUI_INTERNAL$PlayLabel#");
		m_previewPanel->add(m_previewPicture, "#TGUI_INTERNAL$PreviewPic#");
		m_previewPanel->add(m_previewPalette, "#TGUI_INTERNAL$PreviewPal#");
		m_previewPanel->add(m_previewPicSize, "#TGUI_INTERNAL$PreviewPicSize#");
		m_previewPanel->add(m_previewText, "#TGUI_INTERNAL$PreviewText#");
		m_previewPanel->add(m_previewError, "#TGUI_INTERNAL$PreviewError#");
		add(m_previewPanel, "#TGUI_INTERNAL$PreviewPanel#");
		m_filesPanel->add(m_buttonBack, "#TGUI_INTERNAL$ButtonBack#");
		m_filesPanel->add(m_buttonForward, "#TGUI_INTERNAL$ButtonForward#");
		m_filesPanel->add(m_buttonUp, "#TGUI_INTERNAL$ButtonUp#");
		m_filesPanel->add(m_editBoxPath, "#TGUI_INTERNAL$EditBoxPath#");
		m_filesPanel->add(m_listView, "#TGUI_INTERNAL$ListView#");
		m_filesPanel->add(m_labelFilename, "#TGUI_INTERNAL$LabelFilename#");
		m_filesPanel->add(m_editBoxFilename, "#TGUI_INTERNAL$EditBoxFilename#");
		m_filesPanel->add(m_comboBoxFileTypes, "#TGUI_INTERNAL$ComboBoxFileTypes#");
		m_filesPanel->add(m_buttonCancel, "#TGUI_INTERNAL$ButtonCancel#");
		m_filesPanel->add(m_buttonConfirm, "#TGUI_INTERNAL$ButtonConfirm#");
		add(m_filesPanel, "#TGUI_INTERNAL$FilesPanel#");

		// ------------- Previews ------------- //
		m_previewPanel->setPosition({"&.w - w", 0});
		m_previewPanel->setSize({500, "100%"});

		m_previewError->setPosition({10, 10});
		m_previewError->setSize({"100% - 20", 20});
		m_previewError->setHorizontalAlignment(HorizontalAlignment::Center);
		m_previewError->setVerticalAlignment(VerticalAlignment::Center);

		m_playPause->setPosition({10, 10});
		m_playPause->setSize({20, 20});

		m_playSlider->setPosition({"#TGUI_INTERNAL$PlayPause#.x + #TGUI_INTERNAL$PlayPause#.w + 15", 14});
		m_playSlider->setSize({"#TGUI_INTERNAL$PlayLabel#.x - x - 20", 12});
		m_playSlider->setStep(0.01f);

		m_playLabel->setPosition("&.w - w - 10", 13);

		m_previewPicture->setPosition("(&.w - w) / 2", 10);

		m_previewPalette->setPosition("(100% - w) / 2", "#TGUI_INTERNAL$PreviewPic#.y + #TGUI_INTERNAL$PreviewPic#.h + 10");
		m_previewPalette->setSize(256, 16);

		m_previewPicSize->setPosition(10, "#TGUI_INTERNAL$PreviewPic#.y + #TGUI_INTERNAL$PreviewPic#.h + 10");
		m_previewPicSize->setSize("&.w - 20", 20);
		m_previewPicSize->setHorizontalAlignment(HorizontalAlignment::Center);

		m_previewText->setPosition({10, 10});
		m_previewText->setSize({"100% - 20", "100% - 20"});

		// ------------- Files ------------- //
		m_filesPanel->setPosition({0, 0});
		m_filesPanel->setSize({"#TGUI_INTERNAL$PreviewPanel#.x", "100%"});

		m_buttonConfirm->setOrigin(1, 1);
		m_buttonCancel->setOrigin(1, 1);
		m_comboBoxFileTypes->setOrigin(1, 1);
		m_editBoxFilename->setOrigin(1, 1);

#ifdef TGUI_SYSTEM_WINDOWS
		m_buttonCancel->setPosition("100% - 10", "100% - 10");
		m_buttonConfirm->setPosition("#TGUI_INTERNAL$ButtonCancel#.left - 10", "100% - 10");
#else
		m_buttonConfirm->setPosition("100% - 10", "100% - 10");
		m_buttonCancel->setPosition("#TGUI_INTERNAL$ButtonConfirm#.left - 10", "100% - 10");
#endif
		m_comboBoxFileTypes->setWidth("33%");
		m_comboBoxFileTypes->setPosition("100% - 10", "#TGUI_INTERNAL$ButtonConfirm#.top - 15");

		m_labelFilename->setPosition({10, "#TGUI_INTERNAL$ComboBoxFileTypes#.top + (#TGUI_INTERNAL$ComboBoxFileTypes#.height - #TGUI_INTERNAL$LabelFilename#.height) / 2"});

		m_editBoxFilename->setPosition("#TGUI_INTERNAL$ComboBoxFileTypes#.left - 10", "#TGUI_INTERNAL$ComboBoxFileTypes#.y");
		m_editBoxFilename->setWidth("#TGUI_INTERNAL$EditBoxFilename#.x - #TGUI_INTERNAL$LabelFilename#.width - #TGUI_INTERNAL$LabelFilename#.x - 5");

		m_buttonBack->setPosition({10, 10});
		m_buttonBack->setSize({"height", "#TGUI_INTERNAL$EditBoxPath#.height"});

		m_buttonForward->setPosition({"#TGUI_INTERNAL$ButtonBack#.x + #TGUI_INTERNAL$ButtonBack#.width + 5", 10});
		m_buttonForward->setSize({"height", "#TGUI_INTERNAL$EditBoxPath#.height"});

		m_buttonUp->setPosition({"#TGUI_INTERNAL$ButtonForward#.x + #TGUI_INTERNAL$ButtonForward#.width + 15", 10});
		m_buttonUp->setSize({"height", "#TGUI_INTERNAL$EditBoxPath#.height"});

		m_editBoxPath->setPosition({"#TGUI_INTERNAL$ButtonUp#.x + #TGUI_INTERNAL$ButtonUp#.width + 5", 10});
		m_editBoxPath->setWidth("100% - #TGUI_INTERNAL$EditBoxPath#.x - 10");

		m_listView->setPosition({10, "#TGUI_INTERNAL$EditBoxPath#.y + #TGUI_INTERNAL$EditBoxPath#.height + 10"});
		m_listView->setSize({"100% - 20", "#TGUI_INTERNAL$EditBoxFilename#.top - top - 10"});

		m_listView->addColumn("Name", 0);
		m_listView->addColumn("Size", 75, HorizontalAlignment::Right);
		m_listView->addColumn("Type", 75, HorizontalAlignment::Right);
		m_listView->addColumn("Format", 95, HorizontalAlignment::Right);
		m_listView->setColumnExpanded(0, true);

		m_listView->setFixedIconSize({static_cast<float>(m_listView->getItemHeight()) * 0.8f, 0});

		m_labelFilename->setText("Filename:");
		m_editBoxPath->setText(m_currentDirectory);
		m_buttonBack->setText(U"\u2190");
		m_buttonForward->setText(U"\u2192");
		m_buttonUp->setText(U"\u2191");
		m_playPause->setImage("assets/gui/editor/play.png");
		m_previewText->getHorizontalScrollbar()->setPolicy(Scrollbar::Policy::Automatic);

		if (initRenderer)
		{
			m_renderer = aurora::makeCopied<FileDialogRenderer>();
			setRenderer(Theme::getDefault()->getRendererNoThrow(m_type));
		}

		setFileTypeFilters({});

		setPath("");
		connectSignals();
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	PackageFileDialog::PackageFileDialog(const PackageFileDialog& other) :
		ChildWindow             {other},
		onFileSelect            {other.onFileSelect},
		onCancel                {other.onCancel},
		m_root                  {other.m_root},
		m_fileList              {other.m_fileList},
		m_currentDirectory      {other.m_currentDirectory},
		m_filesInDirectory      {other.m_filesInDirectory},
		m_icons                 {other.m_icons},
		m_fileIcons             {other.m_fileIcons},
		m_sortColumnIndex       {other.m_sortColumnIndex},
		m_sortInversed          {other.m_sortInversed},
		m_pathHistory           {other.m_pathHistory},
		m_pathHistoryIndex      {other.m_pathHistoryIndex},
		m_fileMustExist         {other.m_fileMustExist},
		m_selectingDirectory    {other.m_selectingDirectory},
		m_multiSelect           {other.m_multiSelect},
		m_fileTypeFilters       {other.m_fileTypeFilters},
		m_selectedFileTypeFilter{other.m_selectedFileTypeFilter},
		m_selectedFiles         {other.m_selectedFiles}
	{
		identifyChildWidgets();
		connectSignals();

		// Update the file list (to refresh icons)
		changePath(m_currentDirectory, false);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	PackageFileDialog::PackageFileDialog(PackageFileDialog&& other) noexcept :
		ChildWindow             {std::move(other)},
		onFileSelect            {std::move(other.onFileSelect)},
		onCancel                {std::move(other.onCancel)},
		m_root                  {std::move(other.m_root)},
		m_fileList              {std::move(other.m_fileList)},
		m_source                {std::move(other.m_source)},
		m_ownSource             {std::move(other.m_ownSource)},
		m_buttonBack            {std::move(other.m_buttonBack)},
		m_buttonForward         {std::move(other.m_buttonForward)},
		m_buttonUp              {std::move(other.m_buttonUp)},
		m_editBoxPath           {std::move(other.m_editBoxPath)},
		m_listView              {std::move(other.m_listView)},
		m_labelFilename         {std::move(other.m_labelFilename)},
		m_editBoxFilename       {std::move(other.m_editBoxFilename)},
		m_comboBoxFileTypes     {std::move(other.m_comboBoxFileTypes)},
		m_buttonCancel          {std::move(other.m_buttonCancel)},
		m_buttonConfirm         {std::move(other.m_buttonConfirm)},
		m_currentDirectory      {std::move(other.m_currentDirectory)},
		m_filesInDirectory      {std::move(other.m_filesInDirectory)},
		m_icons                 {std::move(other.m_icons)},
		m_fileIcons             {std::move(other.m_fileIcons)},
		m_sortColumnIndex       {std::move(other.m_sortColumnIndex)},
		m_sortInversed          {std::move(other.m_sortInversed)},
		m_pathHistory           {std::move(other.m_pathHistory)},
		m_pathHistoryIndex      {std::move(other.m_pathHistoryIndex)},
		m_fileMustExist         {std::move(other.m_fileMustExist)},
		m_selectingDirectory    {std::move(other.m_selectingDirectory)},
		m_multiSelect           {std::move(other.m_multiSelect)},
		m_fileTypeFilters       {std::move(other.m_fileTypeFilters)},
		m_selectedFileTypeFilter{std::move(other.m_selectedFileTypeFilter)},
		m_selectedFiles         {std::move(other.m_selectedFiles)}
	{
		m_sound.setLooping(true);
		connectSignals();
	}

	PackageFileDialog::~PackageFileDialog()
	{
		if (m_ownSource)
			delete m_source;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	PackageFileDialog& PackageFileDialog::operator=(const PackageFileDialog& other)
	{
		if (this != &other)
		{
			ChildWindow::operator=(other);

			onFileSelect = other.onFileSelect;
			onCancel = other.onCancel;
			m_root = other.m_root;
			m_fileList = other.m_fileList;
			m_currentDirectory = other.m_currentDirectory;
			m_filesInDirectory = other.m_filesInDirectory;
			m_icons = other.m_icons,
			m_fileIcons = other.m_fileIcons;
			m_sortColumnIndex = other.m_sortColumnIndex;
			m_sortInversed = other.m_sortInversed;
			m_pathHistory = other.m_pathHistory;
			m_pathHistoryIndex = other.m_pathHistoryIndex;
			m_fileMustExist = other.m_fileMustExist;
			m_selectingDirectory = other.m_selectingDirectory;
			m_multiSelect = other.m_multiSelect;
			m_fileTypeFilters = other.m_fileTypeFilters;
			m_selectedFileTypeFilter = other.m_selectedFileTypeFilter;
			m_selectedFiles = other.m_selectedFiles;

			identifyChildWidgets();
			connectSignals();

			// Update the file list (to refresh icons)
			changePath(m_currentDirectory, false);
		}

		return *this;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	PackageFileDialog& PackageFileDialog::operator=(PackageFileDialog&& other) noexcept
	{
		if (this != &other)
		{
			onFileSelect = std::move(other.onFileSelect);
			onCancel = std::move(other.onCancel);
			m_root = std::move(other.m_root);
			m_fileList = std::move(other.m_fileList);
			m_buttonBack = std::move(other.m_buttonBack);
			m_buttonForward = std::move(other.m_buttonForward);
			m_buttonUp = std::move(other.m_buttonUp);
			m_editBoxPath = std::move(other.m_editBoxPath);
			m_listView = std::move(other.m_listView);
			m_labelFilename = std::move(other.m_labelFilename);
			m_editBoxFilename = std::move(other.m_editBoxFilename);
			m_comboBoxFileTypes = std::move(other.m_comboBoxFileTypes);
			m_buttonCancel = std::move(other.m_buttonCancel);
			m_buttonConfirm = std::move(other.m_buttonConfirm);
			m_currentDirectory = std::move(other.m_currentDirectory);
			m_filesInDirectory = std::move(other.m_filesInDirectory);
			m_icons = std::move(other.m_icons),
			m_fileIcons = std::move(other.m_fileIcons);
			m_sortColumnIndex = std::move(other.m_sortColumnIndex);
			m_sortInversed = std::move(other.m_sortInversed);
			m_pathHistory = std::move(other.m_pathHistory);
			m_pathHistoryIndex = std::move(other.m_pathHistoryIndex);
			m_fileMustExist = std::move(other.m_fileMustExist);
			m_selectingDirectory = std::move(other.m_selectingDirectory);
			m_multiSelect = std::move(other.m_multiSelect);
			m_fileTypeFilters = std::move(other.m_fileTypeFilters);
			m_selectedFileTypeFilter = std::move(other.m_selectedFileTypeFilter);
			m_selectedFiles = std::move(other.m_selectedFiles);
			ChildWindow::operator=(std::move(other));

			connectSignals();
		}

		return *this;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	PackageFileDialog::Ptr PackageFileDialog::create(ShadyCore::Package *source, bool takeOwnership, const String& title, const String& confirmButtonText)
	{
		auto fileDialog = std::make_shared<PackageFileDialog>();
		fileDialog->setSource(source, takeOwnership);
		fileDialog->setTitle(title);
		fileDialog->setConfirmButtonText(confirmButtonText);
		return fileDialog;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	PackageFileDialog::Ptr PackageFileDialog::copy(const PackageFileDialog::ConstPtr& dialog)
	{
		if (dialog)
			return std::static_pointer_cast<PackageFileDialog>(dialog->clone());
		return nullptr;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	FileDialogRenderer* PackageFileDialog::getSharedRenderer()
	{
		return aurora::downcast<FileDialogRenderer*>(Widget::getSharedRenderer());
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	const FileDialogRenderer* PackageFileDialog::getSharedRenderer() const
	{
		return aurora::downcast<const FileDialogRenderer*>(Widget::getSharedRenderer());
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	FileDialogRenderer* PackageFileDialog::getRenderer()
	{
		return aurora::downcast<FileDialogRenderer*>(Widget::getRenderer());
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	const std::vector<String>& PackageFileDialog::getSelectedPaths() const
	{
		return m_selectedFiles;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PackageFileDialog::setPath(const String& path)
	{
		m_pathHistoryIndex = 0;
		m_pathHistory.clear();
		m_pathHistory.push_back(path.toStdString());
		historyChanged();

		changePath(path.toStdString(), false);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	PackageFileDialog::FileInfo &PackageFileDialog::getFolder(const String &path)
	{
		auto it = m_fileList.find(path);

		if (it != m_fileList.end()) {
			TGUI_ASSERT(it->second->directory, "File inside a file");
			return *it->second;
		}

		auto pos = path.find_last_of('/');
		auto parent = path.substr(0, pos);
		auto &folder = pos == String::npos ? this->m_root : this->getFolder(parent);
		FileInfo entry{
			.filename = pos == String::npos ? path : path.substr(pos + 1),
			.path = path,
			.directory = true,
			.fileSize = 0,
			.type = { ShadyCore::FileType::TYPE_UNKNOWN, ShadyCore::FileType::FORMAT_UNKNOWN },
			.children = {}
		};

		folder.children.push_back(entry);
		this->m_fileList[path] = &folder.children.back();
		return folder.children.back();
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PackageFileDialog::addEntry(const String &path, std::size_t size, ShadyCore::FileType type)
	{
		TGUI_ASSERT(!m_fileList.contains(path), "Duplicate file/folder in package");

		auto pos = path.find_last_of('/');
		auto parent = path.substr(0, pos);
		auto &folder = pos == String::npos ? this->m_root : this->getFolder(parent);
		FileInfo entry{
			.filename = pos == String::npos ? path : path.substr(pos + 1),
			.path = path,
			.directory = false,
			.fileSize = size,
			.type = type,
			.children = {}
		};

		folder.children.push_back(entry);
		this->m_fileList[path] = &folder.children.back();
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PackageFileDialog::setSource(ShadyCore::Package *source, bool takeOwnership)
	{
		if (m_ownSource)
			delete m_source;
		m_source = source;
		m_ownSource = takeOwnership;
		m_fileList.clear();
		m_fileList[""] = &this->m_root;
		m_root.children.clear();
		for (auto &[key, entry] : *source)
			this->addEntry(shiftJISDecode(key.actualName), entry->getSize(), key.fileType);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	const String& PackageFileDialog::getPath() const
	{
		return m_currentDirectory;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PackageFileDialog::setFilename(const String& filename)
	{
		m_editBoxFilename->setText(filename);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	const String& PackageFileDialog::getFilename() const
	{
		return m_editBoxFilename->getText();
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PackageFileDialog::setFileTypeFilters(const std::vector<std::pair<String, std::vector<String>>>& filters, std::size_t defaultFilterIndex)
	{
		m_fileTypeFilters.clear();
		for (const auto& filter : filters)
		{
			std::vector<String> expressions = filter.second;
			for (auto& expression : expressions)
			{
				if (expression.empty() || (expression == U"*") || (expression == U"*.*"))
				{
					// The expression doesn't filter any files and shouldn't exist
					expressions.clear();
					break;
				}

				expression = expression.toLower();
			}

			m_fileTypeFilters.emplace_back(filter.first, expressions);
		}

		if (m_fileTypeFilters.empty())
			m_fileTypeFilters.emplace_back("All files (*)", std::vector<String>());

		m_comboBoxFileTypes->removeAllItems();

		for (const auto& filter : m_fileTypeFilters)
			m_comboBoxFileTypes->addItem(filter.first);

		if (defaultFilterIndex > m_comboBoxFileTypes->getItemCount())
		{
			defaultFilterIndex = m_comboBoxFileTypes->getItemCount() - 1;
			TGUI_PRINT_WARNING("defaultIndex too high in PackageFileDialog::setFileTypeFilters");
		}

		m_comboBoxFileTypes->setSelectedItemByIndex(defaultFilterIndex);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	const std::vector<std::pair<String, std::vector<String>>>& PackageFileDialog::getFileTypeFilters() const
	{
		return m_fileTypeFilters;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::size_t PackageFileDialog::getFileTypeFiltersIndex() const
	{
		TGUI_ASSERT(m_comboBoxFileTypes->getSelectedItemIndex() >= 0, "File type combo box always need to have item selected");
		return static_cast<std::size_t>(m_comboBoxFileTypes->getSelectedItemIndex());
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PackageFileDialog::setConfirmButtonText(const String& text)
	{
		m_buttonConfirm->setText(text);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	const String& PackageFileDialog::getConfirmButtonText() const
	{
		return m_buttonConfirm->getText();
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PackageFileDialog::setCancelButtonText(const String& text)
	{
		m_buttonCancel->setText(text);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	const String& PackageFileDialog::getCancelButtonText() const
	{
		return m_buttonCancel->getText();
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PackageFileDialog::setFilenameLabelText(const String& labelText)
	{
		m_labelFilename->setText(labelText);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	const String& PackageFileDialog::getFilenameLabelText() const
	{
		return m_labelFilename->getText();
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PackageFileDialog::setListViewColumnCaptions(const String& nameColumnText, const String& sizeColumnText, const String& typeColumnText, const String &formatColumnText)
	{
		m_listView->setColumnText(0, nameColumnText);
		m_listView->setColumnText(1, sizeColumnText);
		m_listView->setColumnText(2, typeColumnText);
		m_listView->setColumnText(3, formatColumnText);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::tuple<String, String, String, String> PackageFileDialog::getListViewColumnCaptions() const
	{
		return std::make_tuple(m_listView->getColumnText(0), m_listView->getColumnText(1), m_listView->getColumnText(2), m_listView->getColumnText(3));
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PackageFileDialog::setFileMustExist(bool enforceExistence)
	{
		m_fileMustExist = enforceExistence;
		updateConfirmButtonEnabled();
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PackageFileDialog::getFileMustExist() const
	{
		return m_fileMustExist;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PackageFileDialog::setSelectingDirectory(bool selectDirectories)
	{
		m_selectingDirectory = selectDirectories;

		if (selectDirectories)
		{
			m_comboBoxFileTypes->setVisible(false);
			m_editBoxFilename->setPosition({"100% - 10", m_editBoxFilename->getPositionLayout().y});
		}
		else
		{
			m_comboBoxFileTypes->setVisible(true);
			m_editBoxFilename->setPosition({"#TGUI_INTERNAL$ComboBoxFileTypes#.x - #TGUI_INTERNAL$ComboBoxFileTypes#.width - 10", m_editBoxFilename->getPositionLayout().y});
		}

		changePath(m_currentDirectory, false);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PackageFileDialog::getSelectingDirectory() const
	{
		return m_selectingDirectory;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PackageFileDialog::setMultiSelect(bool multiSelect)
	{
		m_multiSelect = multiSelect;
		m_listView->setMultiSelect(multiSelect);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PackageFileDialog::getMultiSelect() const
	{
		return m_multiSelect;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PackageFileDialog::keyPressed(const Event::KeyEvent& event)
	{
		if ((event.code == Event::KeyboardKey::Enter) && (!m_editBoxPath->isFocused()))
		{
			// Simulate a press on the confirm button
			if (m_buttonConfirm->isEnabled())
				confirmButtonPressed();
		}
		else
		{
			ChildWindow::keyPressed(event);
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PackageFileDialog::canHandleKeyPress(const Event::KeyEvent& event)
	{
		if ((event.code == Event::KeyboardKey::Enter) || (event.code == Event::KeyboardKey::Escape))
			return true;
		return ChildWindow::canHandleKeyPress(event);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PackageFileDialog::textEntered(char32_t key)
	{
		// We will only search through the list view if no other widget was focused. Pass the event to the focused widget otherwise.
		if (m_focusedWidget && (m_focusedWidget != m_listView))
		{
			ChildWindow::textEntered(key);
			return;
		}

		// We currently only search for the typed character. In the future we should remember previously typed characters too.
		const String searchStr{key};

		// Select the first item in the list view that starts with the search string (case-insensitive), starting the
		// search from the currently selected item.
		const std::size_t startIndex = static_cast<std::size_t>(std::max(0, m_listView->getSelectedItemIndex()));
		const std::size_t itemCount = m_listView->getItemCount();
		for (std::size_t i = 0; i < itemCount; ++i)
		{
			const std::size_t index = (startIndex + i) % itemCount;
			const String& item = m_listView->getItem(index);
			if (item.empty() || (item.length() < searchStr.length()))
				continue;

			if (item.startsWithIgnoreCase(searchStr))
			{
				m_listView->setSelectedItem(index);
				break;
			}
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PackageFileDialog::changePath(const String& path, bool updateHistory)
	{
		if (updateHistory && (m_currentDirectory != path))
		{
			if (m_pathHistoryIndex + 1 < m_pathHistory.size())
				m_pathHistory.erase(m_pathHistory.cbegin() + static_cast<std::ptrdiff_t>(m_pathHistoryIndex + 1), m_pathHistory.cend());

			m_pathHistory.push_back(path);
			m_pathHistoryIndex = m_pathHistory.size() - 1;
			historyChanged();
		}

		m_currentDirectory = path;
		m_editBoxPath->setText(m_currentDirectory);

		auto entry = this->m_fileList[path];

		if (entry)
			m_filesInDirectory = entry->children;
		else
			m_filesInDirectory.clear();

		// If only directories should be shown then remove the files from the list
		if (m_selectingDirectory)
		{
			m_filesInDirectory.erase(std::remove_if(
				m_filesInDirectory.begin(), m_filesInDirectory.end(), 
				[](const FileInfo& info){ return !info.directory; }), m_filesInDirectory.end()
			);
		}

		m_fileIcons.clear();
		m_fileIcons.reserve(m_filesInDirectory.size());
		for (const auto& file : m_filesInDirectory) {
			if (file.directory)
				m_fileIcons.push_back(m_icons[10]);
			else if (file.type.type == ShadyCore::FileType::TYPE_TEXTURE)
				m_fileIcons.push_back(m_icons[0]);
			else if (file.type.format == ShadyCore::FileType::SCHEMA_GAME_GUI)
				m_fileIcons.push_back(m_icons[9]);
			else
				m_fileIcons.push_back(m_icons[file.type.type]);
		}

		if (m_source) {
			auto it = m_source->find((m_currentDirectory + "/palette000.pal").toStdString());

			if (it != m_source->end()) {
				ShadyCore::Palette pal;
				auto &stream = it->second->open();

				ShadyCore::getResourceReader(it->first.fileType)(&pal, stream);
				it->second->close(stream);
				pal.unpack();

				auto ptr = reinterpret_cast<uint32_t *>(pal.data);

				for (size_t i = 0 ; i < 256; i++) {
					m_palette[i].r = ptr[i] >> 16 & 0xFF;
					m_palette[i].g = ptr[i] >> 8  & 0xFF;
					m_palette[i].b = ptr[i] >> 0  & 0xFF;
					m_palette[i].a = i == 0 ? 0 : 255;
				}
			}
		}

		m_listView->getVerticalScrollbar()->setValue(0);
		sortFilesInListView();
		updateConfirmButtonEnabled();
		updatePreview();
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PackageFileDialog::historyChanged()
	{
		m_buttonBack->setEnabled(m_pathHistoryIndex > 0);
		m_buttonForward->setEnabled(m_pathHistoryIndex + 1 < m_pathHistory.size());
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PackageFileDialog::sortFilesInListView()
	{
		const auto oldScrollbarValue = m_listView->getVerticalScrollbar()->getValue();

		std::vector<std::pair<FileInfo, Texture>> items;
		items.reserve(m_filesInDirectory.size());
		if (!m_fileIcons.empty())
		{
			auto it = m_filesInDirectory.begin();

			TGUI_ASSERT(m_filesInDirectory.size() == m_fileIcons.size(), "Icon count must match file count in PackageFileDialog::sortFilesInListView");
			for (std::size_t i = 0; i < m_filesInDirectory.size(); ++i) {
				items.emplace_back(*it, m_fileIcons[i]);
				++it;
			}
		}
		else // There are no file icons
		{
			for (const auto& file : m_filesInDirectory)
				items.emplace_back(file, Texture{});
		}

		std::sort(items.begin(), items.end(), [this](const auto& leftItem, const auto& rightItem){
			const FileInfo& left = leftItem.first;
			const FileInfo& right = rightItem.first;
			if (m_sortColumnIndex == 3) // Sort by format
			{
				auto l = left.type.format << 8 | left.type.type;
				auto r = right.type.format << 8 | right.type.type;

				if (m_sortInversed)
					return l < r;
				return l > r;
			}
			if (m_sortColumnIndex == 2) // Sort by file size
			{
				if (left.directory != right.directory)
					return right.directory; // Place directories at the end of the list
				if (left.directory) // Both are directories, sort them alphabetically by filename since they have no size
					return left.filename.toLower() < right.filename.toLower();
				// Both are files, sort them by file size
				if (m_sortInversed)
					return left.type.type < right.type.type;
				return left.type.type > right.type.type;
			}
			if (m_sortColumnIndex == 1) // Sort by file size
			{
				if (left.directory != right.directory)
					return right.directory; // Place directories at the end of the list
				if (left.directory) // Both are directories, sort them alphabetically by filename since they have no size
					return left.filename.toLower() < right.filename.toLower();
				// Both are files, sort them by file size
				if (m_sortInversed)
					return left.fileSize < right.fileSize;
				return left.fileSize > right.fileSize;
			}
			// Sort by filename
			if (left.directory != right.directory)
				return left.directory; // Place directories in front of files
			// Both are directories or both are files, so sort alphabetically
			if (m_sortInversed)
				return left.filename.toLower() > right.filename.toLower();
			return left.filename.toLower() < right.filename.toLower();
		});

		if (!m_listView->getHeaderVisible())
		{
			m_listView->setHeaderVisible(true);
			m_listView->setShowVerticalGridLines(true);
		}
		m_listView->removeAllItems();
		for (const auto& item : items)
		{
			const FileInfo& file = item.first;
			const Texture& icon = item.second;

			// Filter the files
			if (!file.directory && !m_fileTypeFilters[m_selectedFileTypeFilter].second.empty())
			{
				const String& lowercaseFilename = file.filename.toLower();

				bool fileAccepted = false;
				for (const auto& filter : m_fileTypeFilters[m_selectedFileTypeFilter].second)
				{
					TGUI_ASSERT(!filter.empty(), "FileDialog::sortFilesInListView can't have empty filter, they are removed in setFileTypeFilters");

					if (filter[0] == '*')
					{
						if (
							(lowercaseFilename.length() >= filter.length() - 1) &&
							(lowercaseFilename.compare(lowercaseFilename.length() - (filter.length() - 1), filter.length() - 1, filter, 1, filter.length() - 1) == 0)
						)
						{
							fileAccepted = true;
							break;
						}
					}
					// Check if the filter matches exactly with the filename
					else if (lowercaseFilename == filter)
					{
						fileAccepted = true;
						break;
					}
				}

				if (!fileAccepted)
					continue;
			}

			String fileSizeStr;
			if (!file.directory)
			{
				if (file.fileSize == 0)
					fileSizeStr = U"0.0 KB";
				else if (file.fileSize < static_cast<std::uintmax_t>(100))
					fileSizeStr = U"0.1 KB";
				else if (file.fileSize < static_cast<std::uintmax_t>(1000)*1000)
					fileSizeStr = String::fromNumberRounded(file.fileSize / 1000.f, 1) + U" KB";
				else if (file.fileSize < static_cast<std::uintmax_t>(1000)*1000*1000)
					fileSizeStr = String::fromNumberRounded(file.fileSize / 1000.f / 1000.f, 1) + U" MB";
				else if (file.fileSize < static_cast<std::uintmax_t>(1000)*1000*1000*1000)
					fileSizeStr = String::fromNumberRounded(file.fileSize / 1000.f / 1000.f / 1000.f, 1) + U" GB";
				else if (file.fileSize < static_cast<std::uintmax_t>(1000)*1000*1000*1000*1000)
					fileSizeStr = String::fromNumberRounded(file.fileSize / 1000.f / 1000.f / 1000.f / 1000.f, 1) + U" TB";
			}

			String type;
			switch (file.type.type) {
			default:
				if (!file.directory)
					type = "Unknown";
				break;
			case ShadyCore::FileType::TYPE_TEXT:
				type = "Text";
				break;
			case ShadyCore::FileType::TYPE_TABLE:
				type = "Table";
				break;
			case ShadyCore::FileType::TYPE_LABEL:
				type = "Loops";
				break;
			case ShadyCore::FileType::TYPE_IMAGE:
				type = "Image";
				break;
			case ShadyCore::FileType::TYPE_PALETTE:
				type = "Palette";
				break;
			case ShadyCore::FileType::TYPE_SFX:
				type = "Sfx";
				break;
			case ShadyCore::FileType::TYPE_BGM:
				type = "Music";
				break;
			case ShadyCore::FileType::TYPE_SCHEMA:
				type = "Schema";
				break;
			case ShadyCore::FileType::TYPE_TEXTURE:
				type = "Texture";
				break;
			}

			String format;
			switch (file.type.format) {
			case ShadyCore::FileType::FORMAT_UNKNOWN:
				if (!file.directory)
					format = "???";
				break;
			case ShadyCore::FileType::TEXT_GAME:
				format = "cv0";
				break;
			case ShadyCore::FileType::TEXT_NORMAL:
				format = "txt";
				break;
			case ShadyCore::FileType::TABLE_GAME:
				format = "cv1";
				break;
			case ShadyCore::FileType::TABLE_CSV:
				format = "csv";
				break;
			case ShadyCore::FileType::LABEL_RIFF:
				format = "Riff";
				break;
			case ShadyCore::FileType::LABEL_LBL:
				format = "lbl";
				break;
			case ShadyCore::FileType::IMAGE_GAME:
				format = "cv2";
				break;
			case ShadyCore::FileType::IMAGE_PNG:
				format = "png";
				break;
			case ShadyCore::FileType::IMAGE_BMP:
				format = "bmp";
				break;
			case ShadyCore::FileType::PALETTE_PAL:
				format = "pal";
				break;
			case ShadyCore::FileType::PALETTE_ACT:
				format = "act";
				break;
			case ShadyCore::FileType::SFX_GAME:
				format = "cv3";
				break;
			case ShadyCore::FileType::SFX_WAV:
				format = "wav";
				break;
			case ShadyCore::FileType::BGM_OGG:
				format = "ogg";
				break;
			case ShadyCore::FileType::SCHEMA_XML:
				format = "xml";
				break;
			case ShadyCore::FileType::SCHEMA_GAME_GUI:
				format = "gui";
				break;
			case ShadyCore::FileType::SCHEMA_GAME_ANIM:
				format = "pat1";
				break;
			case ShadyCore::FileType::SCHEMA_GAME_PATTERN:
				format = "pat2";
				break;
			case ShadyCore::FileType::TEXTURE_DDS:
				format = "dds";
				break;
			}

			const std::size_t itemIndex = m_listView->addItem({file.filename, fileSizeStr, type, format});
			m_listView->setItemData(itemIndex, file.directory);

			if (icon.getData())
				m_listView->setItemIcon(itemIndex, icon);
		}

		m_listView->getVerticalScrollbar()->setValue(oldScrollbarValue);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PackageFileDialog::filesSelected(std::vector<String> selectedFiles)
	{
		m_selectedFiles = std::move(selectedFiles);

		if (!m_selectedFiles.empty())
			onFileSelect.emit(this, m_selectedFiles);
		else
			onCancel.emit(this);

		close();
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PackageFileDialog::updateConfirmButtonEnabled()
	{
		const String& filename = m_editBoxFilename->getText();
		const bool enabled = ((filename != U".") && (filename != U"..")) // Always disabled for "." and ".." filenames
			&& (m_selectingDirectory || !m_listView->getSelectedItemIndices().empty() || !filename.empty()) // Disabled when no file selected
			&& (!m_fileMustExist || m_fileList.contains(m_currentDirectory.empty() ? filename : m_currentDirectory + "/" + filename)); // Disabled for non-existent files

		if (enabled != m_buttonConfirm->isEnabled())
			m_buttonConfirm->setEnabled(enabled);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PackageFileDialog::updatePreview()
	{
		auto indices = m_listView->getSelectedItemIndices();

		m_sound.stop();
		m_music.stop();
		m_previewError->setVisible(false);
		m_playPause->setVisible(false);
		m_playSlider->setVisible(false);
		m_playLabel->setVisible(false);
		m_previewPicture->setVisible(false);
		m_previewPalette->setVisible(false);
		m_previewPicSize->setVisible(false);
		m_previewText->setVisible(false);
		if (indices.size() != 1 || !m_source)
			return;

		auto selected = m_listView->getItem(*indices.begin());
		auto path = m_currentDirectory.empty() ? selected : m_currentDirectory + "/" + selected;
		auto it = m_source->find(shiftJISEncode(path.toUtf32()));

		if (it == m_source->end())
			return;

		auto &entry = it->second->open();
		std::stringstream stream;
		ShadyCore::FileType type = it->first.fileType;

		switch (it->first.fileType.type) {
		case ShadyCore::FileType::TYPE_TEXT: {
			ShadyCore::TextResource res;

			if (path.ends_with(".cv0"))
				type.format = ShadyCore::FileType::TEXT_GAME;
			else
				type.format = ShadyCore::FileType::TEXT_NORMAL;
			ShadyCore::getResourceReader(type)(&res, entry);
			ShadyCore::getResourceWriter({ShadyCore::FileType::TYPE_TEXT, ShadyCore::FileType::TEXT_NORMAL})(&res, stream);
			m_previewText->setText(shiftJISDecode(stream.str()));
			m_previewText->getVerticalScrollbar()->setValue(0);
			m_previewText->setVisible(true);
			break;
		}
		case ShadyCore::FileType::TYPE_TABLE: {
			ShadyCore::TextResource res;

			if (path.ends_with(".cv1"))
				type.format = ShadyCore::FileType::TABLE_GAME;
			else if (path.ends_with(".csv"))
				type.format = ShadyCore::FileType::TABLE_CSV;
			else {
				m_previewError->setVisible(true);
				m_previewError->setText("Table of unknown format");
				break;
			}
			ShadyCore::getResourceReader(type)(&res, entry);
			ShadyCore::getResourceWriter({ShadyCore::FileType::TYPE_TABLE, ShadyCore::FileType::TABLE_CSV})(&res, stream);
			m_previewText->setText(shiftJISDecode(stream.str()));
			m_previewText->getVerticalScrollbar()->setValue(0);
			m_previewText->setVisible(true);
			break;
		}
		case ShadyCore::FileType::TYPE_LABEL: {
			ShadyCore::TextResource res;

			ShadyCore::getResourceReader(type)(&res, entry);
			ShadyCore::getResourceWriter({ShadyCore::FileType::TYPE_LABEL, ShadyCore::FileType::LABEL_LBL})(&res, stream);
			m_previewText->setText(shiftJISDecode(stream.str()));
			m_previewText->getVerticalScrollbar()->setValue(0);
			m_previewText->setVisible(true);
			break;
		}
		case ShadyCore::FileType::TYPE_IMAGE: {
			ShadyCore::Image res;

			ShadyCore::getResourceReader(type)(&res, entry);

			sf::Image image{{res.width, res.height}};

			if (res.bitsPerPixel == 8) {
				for (unsigned i = 0; i < res.getRawSize(); ++i) {
					unsigned x = i % res.paddedWidth;
					unsigned y = i / res.paddedWidth;

					if (x >= res.width)
						continue;
					if (y >= res.height)
						continue;
					image.setPixel({x, y}, m_palette[res.raw[i]]);
				}
			} else {
				for (unsigned y = 0; y < res.height; y++)
					for (unsigned x = 0; x < res.width; x++)
						image.setPixel({x, y}, sf::Color{
							res.raw[(y * res.paddedWidth + x) * 4 + 2],
							res.raw[(y * res.paddedWidth + x) * 4 + 1],
							res.raw[(y * res.paddedWidth + x) * 4 + 0],
							res.raw[(y * res.paddedWidth + x) * 4 + 3]
						});
			}

			Texture t;
			auto size = m_previewPanel->getInnerSize();

			size.x -= 20;
			size.y -= 50;
			t.loadFromPixelData(image.getSize(), image.getPixelsPtr());
			if (res.width > size.x || res.height > size.y) {
				if (res.width * size.y < size.x * res.height) {
					size.x = size.y * res.width / res.height;
				} else {
					size.y = size.x * res.height / res.width;
				}
			} else {
				size.x = res.width;
				size.y = res.height;
			}
			m_previewPicture->getRenderer()->setTexture(t);
			m_previewPicture->setSize(size);
			m_previewPicture->setVisible(true);
			m_previewPicSize->setText(std::to_string(res.width) + "x" + std::to_string(res.height));
			m_previewPicSize->setVisible(true);
			break;
		}
		case ShadyCore::FileType::TYPE_PALETTE: {
			ShadyCore::Palette pal;
			ShadyCore::Image res;

			ShadyCore::getResourceReader(type)(&pal, entry);
			pal.unpack();

			auto ptr = reinterpret_cast<uint32_t *>(pal.data);

			for (size_t i = 0 ; i < 256; i++) {
				m_palette[i].r = ptr[i] >> 16 & 0xFF;
				m_palette[i].g = ptr[i] >> 8  & 0xFF;
				m_palette[i].b = ptr[i] >> 0  & 0xFF;
				m_palette[i].a = i == 0 ? 0 : 255;
			}

			auto its = m_source->find(shiftJISEncode((m_currentDirectory + "/stand000.bmp").toUtf32()));

			if (its == m_source->end())
				its = m_source->find(shiftJISEncode((m_currentDirectory + "/stand000 .bmp").toUtf32()));
			if (its == m_source->end()) {
				m_previewError->setVisible(true);
				m_previewError->setText("Cannot find stand000.bmp");
				break;
			}

			auto &pstream = its->second->open();
			ShadyCore::getResourceReader(its->first.fileType)(&res, pstream);
			its->second->close(pstream);
			sf::Image image{{res.width, res.height}};

			if (res.bitsPerPixel == 8) {
				for (unsigned i = 0; i < res.getRawSize(); ++i) {
					unsigned x = i % res.paddedWidth;
					unsigned y = i / res.paddedWidth;

					if (x >= res.width)
						continue;
					if (y >= res.height)
						continue;
					image.setPixel({x, y}, m_palette[res.raw[i]]);
				}
			} else {
				for (unsigned y = 0; y < res.height; y++)
					for (unsigned x = 0; x < res.width; x++)
						image.setPixel({x, y}, sf::Color{
							res.raw[(y * res.paddedWidth + x) * 4 + 2],
							res.raw[(y * res.paddedWidth + x) * 4 + 1],
							res.raw[(y * res.paddedWidth + x) * 4 + 0],
							res.raw[(y * res.paddedWidth + x) * 4 + 3]
						});
			}

			sf::Image img{{256, 16}};

			for (unsigned i = 0; i < 256; i++)
				for (unsigned x = 0; x < 4; x++)
					for (unsigned y = 0; y < 4; y++)
						img.setPixel({(i % 64) * 4 + x, (i / 64) * 4 + y}, m_palette[i]);

			Texture t;
			Texture t2;
			auto size = m_previewPanel->getInnerSize();

			size.x -= 20;
			size.y -= 50;
			t.loadFromPixelData(image.getSize(), image.getPixelsPtr());
			t2.loadFromPixelData(img.getSize(), img.getPixelsPtr());
			if (res.width > size.x || res.height > size.y) {
				if (res.width * size.y < size.x * res.height) {
					size.x = size.y * res.width / res.height;
				} else {
					size.y = size.x * res.height / res.width;
				}
			} else {
				size.x = res.width;
				size.y = res.height;
			}
			m_previewPicture->getRenderer()->setTexture(t);
			m_previewPicture->setSize(size);
			m_previewPicture->setVisible(true);
			m_previewPalette->getRenderer()->setTexture(t2);
			m_previewPalette->setVisible(true);
			break;
		}
		case ShadyCore::FileType::TYPE_SFX: {
			ShadyCore::Sfx sfx;

			ShadyCore::getResourceReader(type)(&sfx, entry);
			if (sfx.bitsPerSample != 16) {
				m_previewError->setVisible(true);
				m_previewError->setText("Unsupported bits per sample");
				break;
			}

			std::vector channels{sfx.channels, sf::SoundChannel::Mono};

			if (!m_soundBuffer.loadFromSamples(
				reinterpret_cast<const std::int16_t *>(sfx.data),
				sfx.size / 2,
				sfx.channels,
				sfx.sampleRate, channels
			)) {
				m_previewError->setVisible(true);
				m_previewError->setText("Failed to load samples");
				break;
			}
			m_usesMusic = false;
			m_playPause->setVisible(true);
			m_playSlider->setMaximum(m_soundBuffer.getDuration().asSeconds());
			m_playSlider->setValue(0);
			m_playSlider->setVisible(true);
			m_playLabel->setVisible(true);
			break;
		}
		case ShadyCore::FileType::TYPE_BGM: {
			auto pstream = std::make_unique<PackageStream>(it->second);

			if (!m_music.openFromStream(*pstream)) {
				m_previewError->setVisible(true);
				m_previewError->setText("Failed to load from stream");
				break;
			}
			m_musicStream.swap(pstream);
			m_usesMusic = true;
			m_playPause->setVisible(true);
			m_playSlider->setMaximum(m_music.getDuration().asSeconds());
			m_playSlider->setValue(0);
			m_playSlider->setVisible(true);
			m_playLabel->setVisible(true);
			break;
		}
		case ShadyCore::FileType::TYPE_SCHEMA: {
			ShadyCore::Schema res;
			std::set<unsigned> actions;
			unsigned frames = 0;

			if (type.format == ShadyCore::FileType::FORMAT_UNKNOWN) {
				if (path.ends_with(".xml"))
					type.format = ShadyCore::FileType::SCHEMA_XML;
				else if (path.ends_with(".dat"))
					type.format = ShadyCore::FileType::SCHEMA_GAME_GUI;
				else if (path.ends_with("effect.pat") || path.ends_with("stand.pat"))
					type.format = ShadyCore::FileType::SCHEMA_GAME_ANIM;
				else
					type.format = ShadyCore::FileType::SCHEMA_GAME_PATTERN;
			}
			if (type.format == ShadyCore::FileType::SCHEMA_GAME_GUI) {
				m_previewError->setVisible(true);
				m_previewError->setText("GUIs not supported");
				break;
			}
			ShadyCore::getResourceReader(type)(&res, entry);
			for (auto &seq : res.objects) {
				actions.emplace(seq->getId());
				if (seq->getType() != 7)
					frames += reinterpret_cast<ShadyCore::Schema::Sequence *>(seq)->frames.size();
			}
			m_previewText->setText(std::to_string(actions.size()) + " ids\n" + std::to_string(res.objects.size()) + " sequences\n" + std::to_string(frames) + " frames");
			m_previewText->setVisible(true);
			break;
		}
		default:
			m_previewError->setVisible(true);
			m_previewError->setText("File of unknown format");
			break;
		}
		it->second->close(entry);
		m_previewText->getHorizontalScrollbar()->setValue(0);
		m_previewText->getVerticalScrollbar()->setValue(0);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PackageFileDialog::confirmButtonPressed()
	{
		if (m_multiSelect && (m_listView->getSelectedItemIndices().size() > 1))
		{
			const auto& indices = m_listView->getSelectedItemIndices();

			std::vector<String> paths;
			for (const auto index : indices)
			{
				// Filter out directories when we are attempting to select files
				if (!m_selectingDirectory && m_listView->getItemData<bool>(index))
					continue;

				auto p = m_listView->getItem(index);

				paths.push_back(m_currentDirectory.empty() ? p : m_currentDirectory + "/" + p);
			}

			filesSelected(std::move(paths));
		}
		else // Only one file selected
		{
			const auto start = m_currentDirectory.empty() ? "" : m_currentDirectory + "/";
			const auto path = start + m_editBoxFilename->getText();

			// If we were looking for a file but a directory was selected then just enter that directory
			if (!m_selectingDirectory && (m_listView->getSelectedItemIndices().size() == 1)
				  && (m_listView->getItemData<bool>(*m_listView->getSelectedItemIndices().begin())))
			{
				changePath(start + m_listView->getItem(*m_listView->getSelectedItemIndices().begin()), true);
				m_editBoxFilename->setText(U"");
			}
			else if (!m_selectingDirectory && !m_editBoxFilename->getText().empty() && m_fileList.contains(path) && m_fileList[path]->directory)
			{
				changePath(path, true);
				m_editBoxFilename->setText(U"");
			}
			else
				filesSelected({path});
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PackageFileDialog::isValidFolderName(const String& name)
	{
		if (name.empty() || (name == U".") || (name == U".."))
			return false;

#ifdef TGUI_SYSTEM_WINDOWS
		if (name.find_first_of(U"\\/:*?\"<>|") != String::npos)
#else
		if (name.contains(U'/'))
#endif
			return false;

		return true;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PackageFileDialog::identifyChildWidgets()
	{
		m_buttonBack = get<Button>("#TGUI_INTERNAL$ButtonBack#");
		m_buttonForward = get<Button>("#TGUI_INTERNAL$ButtonForward#");
		m_buttonUp = get<Button>("#TGUI_INTERNAL$ButtonUp#");
		m_editBoxPath = get<EditBox>("#TGUI_INTERNAL$EditBoxPath#");
		m_listView = get<ListView>("#TGUI_INTERNAL$ListView#");
		m_labelFilename = get<Label>("#TGUI_INTERNAL$LabelFilename#");
		m_editBoxFilename = get<EditBox>("#TGUI_INTERNAL$EditBoxFilename#");
		m_comboBoxFileTypes = get<ComboBox>("#TGUI_INTERNAL$ComboBoxFileTypes#");
		m_buttonCancel = get<Button>("#TGUI_INTERNAL$ButtonCancel#");
		m_buttonConfirm = get<Button>("#TGUI_INTERNAL$ButtonConfirm#");
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PackageFileDialog::connectSignals()
	{
		m_closeButton->onPress.disconnectAll(); // Replace the signal handler of ChildWindow

		m_buttonBack->onPress.disconnectAll();
		m_buttonForward->onPress.disconnectAll();
		m_buttonUp->onPress.disconnectAll();
		m_editBoxPath->onReturnKeyPress.disconnectAll();
		m_comboBoxFileTypes->onItemSelect.disconnectAll();
		m_listView->onItemSelect.disconnectAll();
		m_listView->onHeaderClick.disconnectAll();
		m_listView->onDoubleClick.disconnectAll();
		m_buttonCancel->onPress.disconnectAll();
		m_buttonConfirm->onPress.disconnectAll();
		m_editBoxFilename->onTextChange.disconnectAll();
		m_playPause->onPress.disconnectAll();
		m_playSlider->onValueChange.disconnectAll();

		m_buttonBack->onPress([this]{
			--m_pathHistoryIndex;
			changePath(m_pathHistory[m_pathHistoryIndex], false);
			historyChanged();
		});
		m_buttonForward->onPress([this]{
			++m_pathHistoryIndex;
			changePath(m_pathHistory[m_pathHistoryIndex], false);
			historyChanged();
		});
		m_buttonUp->onPress([this]{
			auto pos = m_currentDirectory.find_last_of('/');
			auto parent = m_currentDirectory.substr(0, pos);

			// If the path ended with a slash but without a filename, then getParentPath simply
			// removed the slash. We however want to go up one level higher.
			if (pos == m_currentDirectory.size() - 1) {
				pos = parent.find_last_of('/');
				parent = parent.substr(0, pos);
			} else if (pos == String::npos)
				parent = "";

			changePath(parent, true);
		});
		m_editBoxPath->onReturnKeyPress([this]{
			changePath(m_editBoxPath->getText(), true);
		});
		m_comboBoxFileTypes->onItemSelect([this](int selectedItem){
			TGUI_ASSERT(selectedItem >= 0, "There always needs to be a file type filter selected in FileDialog");
			TGUI_ASSERT(static_cast<std::size_t>(selectedItem) < m_fileTypeFilters.size(), "Can't select file type filter that doesn't exist");
			m_selectedFileTypeFilter = static_cast<std::size_t>(selectedItem);
			sortFilesInListView();
		});
		m_listView->onItemSelect([this](int itemIndex){
			updateConfirmButtonEnabled();
			updatePreview();
			if (itemIndex < 0)
				return;

			if (m_multiSelect && (m_listView->getSelectedItemIndices().size() > 1))
				m_editBoxFilename->setText(U"");
			else if (m_selectingDirectory || !m_listView->getItemData<bool>(static_cast<std::size_t>(itemIndex)))
				m_editBoxFilename->setText(m_listView->getItem(static_cast<std::size_t>(itemIndex)));
		});
		m_listView->onHeaderClick([this](int itemIndex){
			TGUI_ASSERT(itemIndex >= 0, "Can't click on list view header that doesn't exist");
			if (m_sortColumnIndex != static_cast<std::size_t>(itemIndex))
			{
				m_sortColumnIndex = static_cast<std::size_t>(itemIndex);
				m_sortInversed = false;
			}
			else
				m_sortInversed = !m_sortInversed;

			sortFilesInListView();
		});
		m_listView->onDoubleClick([this](int itemIndex){
			if (itemIndex < 0)
				return;

			if (m_listView->getItemData<bool>(static_cast<std::size_t>(itemIndex)))
			{
				if (m_currentDirectory.empty())
				{
					changePath(m_listView->getItem(static_cast<std::size_t>(itemIndex)), true);
					return;
				}
				changePath(m_currentDirectory + "/" + m_listView->getItem(static_cast<std::size_t>(itemIndex)), true);
				if (m_selectingDirectory)
					m_editBoxFilename->setText(U"");
			}
			else
			{
				m_editBoxFilename->setText(m_listView->getItem(static_cast<std::size_t>(itemIndex)));
				filesSelected({m_currentDirectory + "/" + m_editBoxFilename->getText()});
			}
		});
		m_buttonCancel->onPress([this]{ filesSelected({}); });
		m_buttonConfirm->onPress([this]{ confirmButtonPressed(); });
		m_editBoxFilename->onTextChange([this]{
			// Deselect items when the user starts typing while there are still selected items
			if (
				!m_listView->getSelectedItemIndices().empty() && !m_editBoxFilename->getText().empty() &&
				(m_listView->getItem(*m_listView->getSelectedItemIndices().begin()) != m_editBoxFilename->getText())
			)
			{
				m_listView->setSelectedItems({});
			}

			updateConfirmButtonEnabled();
		});
		m_playPause->onPress([this] {
			auto status = m_usesMusic ? m_music.getStatus() : m_sound.getStatus();

			if (status == sf::Sound::Status::Stopped) {
				m_playPause->setImage("assets/gui/editor/pause.png");
				if (m_usesMusic) {
					(void)m_music.openFromStream(*m_musicStream);
					m_music.play();
				} else
					m_sound.play();
			} else if (status == sf::Sound::Status::Playing) {
				m_playPause->setImage("assets/gui/editor/play.png");
				if (m_usesMusic)
					m_music.pause();
				else
					m_sound.pause();
			} else {
				m_playPause->setImage("assets/gui/editor/pause.png");
				if (m_usesMusic)
					m_music.play();
				else
					m_sound.play();
			}
		});
		m_playSlider->onValueChange([this] {
			if (m_usesMusic)
				m_music.setPlayingOffset(sf::seconds(m_playSlider->getValue()));
			else
				m_sound.setPlayingOffset(sf::seconds(m_playSlider->getValue()));
		});

		m_closeButton->onPress([this]{ filesSelected({}); });
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Signal& PackageFileDialog::getSignal(String signalName)
	{
		if (signalName == onFileSelect.getName())
			return onFileSelect;
		if (signalName == onCancel.getName())
			return onCancel;
		return ChildWindow::getSignal(std::move(signalName));
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PackageFileDialog::rendererChanged(const String& property)
	{
		if (property == U"ListView")
		{
			m_listView->setRenderer(getSharedRenderer()->getListView());
		}
		else if (property == U"EditBox")
		{
			const auto& renderer = getSharedRenderer()->getEditBox();
			m_editBoxFilename->setRenderer(renderer);
			m_editBoxPath->setRenderer(renderer);
		}
		else if (property == U"Button")
		{
			const auto& renderer = getSharedRenderer()->getButton();
			m_buttonCancel->setRenderer(renderer);
			m_buttonConfirm->setRenderer(renderer);

			if (!getSharedRenderer()->getBackButton())
				m_buttonBack->setRenderer(renderer);
			if (!getSharedRenderer()->getForwardButton())
				m_buttonForward->setRenderer(renderer);
			if (!getSharedRenderer()->getUpButton())
				m_buttonUp->setRenderer(renderer);
		}
		else if (property == U"BackButton")
		{
			if (getSharedRenderer()->getBackButton())
				m_buttonBack->setRenderer(getSharedRenderer()->getBackButton());
			else
				m_buttonBack->setRenderer(getSharedRenderer()->getButton());
		}
		else if (property == U"ForwardButton")
		{
			if (getSharedRenderer()->getForwardButton())
				m_buttonForward->setRenderer(getSharedRenderer()->getForwardButton());
			else
				m_buttonForward->setRenderer(getSharedRenderer()->getButton());
		}
		else if (property == U"UpButton")
		{
			if (getSharedRenderer()->getUpButton())
				m_buttonUp->setRenderer(getSharedRenderer()->getUpButton());
			else
				m_buttonUp->setRenderer(getSharedRenderer()->getButton());
		}
		else if (property == U"FilenameLabel")
		{
			m_labelFilename->setRenderer(getSharedRenderer()->getFilenameLabel());
		}
		else if (property == U"FileTypeComboBox")
		{
			m_comboBoxFileTypes->setRenderer(getSharedRenderer()->getFileTypeComboBox());
		}
		else if (property == U"ArrowsOnNavigationButtonsVisible")
		{
			if (getSharedRenderer()->getArrowsOnNavigationButtonsVisible())
			{
				m_buttonBack->setText(U"\u2190");
				m_buttonForward->setText(U"\u2192");
				m_buttonUp->setText(U"\u2191");
			}
			else
			{
				m_buttonBack->setText(U"");
				m_buttonForward->setText(U"");
				m_buttonUp->setText(U"");
			}
		}
		else if (property == U"Font")
		{
			ChildWindow::rendererChanged(property);

			m_buttonBack->setInheritedFont(m_fontCached);
			m_buttonForward->setInheritedFont(m_fontCached);
			m_buttonUp->setInheritedFont(m_fontCached);
			m_editBoxPath->setInheritedFont(m_fontCached);
			m_listView->setInheritedFont(m_fontCached);
			m_labelFilename->setInheritedFont(m_fontCached);
			m_editBoxFilename->setInheritedFont(m_fontCached);
			m_comboBoxFileTypes->setInheritedFont(m_fontCached);
			m_buttonCancel->setInheritedFont(m_fontCached);
			m_buttonConfirm->setInheritedFont(m_fontCached);
		}
		else
			ChildWindow::rendererChanged(property);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::unique_ptr<DataIO::Node> PackageFileDialog::save(SavingRenderersMap& renderers) const
	{
		auto node = ChildWindow::save(renderers);

		// Child widgets are saved indirectly by saving the child window.
		// The list view however contained items which shouldn't be saved, so we removed the nodes that were created for them.
		const auto listViewNodeIt = std::find_if(node->children.cbegin(), node->children.cend(), [](const std::unique_ptr<DataIO::Node>& child){
			return child->name == U"ListView.\"#TGUI_INTERNAL$ListView#\"";
		});
		TGUI_ASSERT(listViewNodeIt != node->children.end(), "FileDialog::save couldn't find its ListView");
		const auto& listViewNode = *listViewNodeIt;
		listViewNode->children.erase(std::remove_if(listViewNode->children.begin(), listViewNode->children.end(), [](const std::unique_ptr<DataIO::Node>& child){
			return child->name == U"Item";
		}), listViewNode->children.end());

		// We currently don't save the path, so the text in the path edit box shouldn't be saved either
		const auto pathEditBoxNodeIt = std::find_if(node->children.cbegin(), node->children.cend(), [](const std::unique_ptr<DataIO::Node>& child){
			return child->name == U"EditBox.\"#TGUI_INTERNAL$EditBoxPath#\"";
		});
		TGUI_ASSERT(pathEditBoxNodeIt != node->children.end(), "FileDialog::save couldn't find its path EditBox");
		const auto& pathEditBoxNode = *pathEditBoxNodeIt;
		pathEditBoxNode->propertyValuePairs.erase(U"Text");

		node->propertyValuePairs[U"FileMustExist"] = std::make_unique<DataIO::ValueNode>(Serializer::serialize(m_fileMustExist));
		node->propertyValuePairs[U"SelectingDirectory"] = std::make_unique<DataIO::ValueNode>(Serializer::serialize(m_selectingDirectory));
		node->propertyValuePairs[U"MultiSelect"] = std::make_unique<DataIO::ValueNode>(Serializer::serialize(m_multiSelect));

		if (!m_fileTypeFilters.empty())
		{
			for (const auto& filter : m_fileTypeFilters)
			{
				TGUI_EMPLACE_BACK(filterNode, node->children)
				filterNode = std::make_unique<DataIO::Node>();
				filterNode->name = "FileTypeFilter";

				const std::vector<String>& patternList = filter.second;
				String patternListStr = "[";
				for (std::size_t j = 0; j < patternList.size(); ++j)
				{
					if (j > 0)
						patternListStr += ", ";

					patternListStr += Serializer::serialize(patternList[j]);
				}
				patternListStr += "]";

				filterNode->propertyValuePairs[U"Description"] = std::make_unique<DataIO::ValueNode>(Serializer::serialize(filter.first));
				filterNode->propertyValuePairs[U"Pattern"] = std::make_unique<DataIO::ValueNode>(patternListStr);
			}

			if (m_fileTypeFilters.size() > 1)
				node->propertyValuePairs[U"SelectedFileTypeFilter"] = std::make_unique<DataIO::ValueNode>(String::fromNumber(m_selectedFileTypeFilter));
		}

		return node;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PackageFileDialog::load(const std::unique_ptr<DataIO::Node>& node, const LoadingRenderersMap& renderers)
	{
		std::vector<std::pair<String, std::vector<String>>> fileTypeFilters;
		for (const auto& childNode : node->children)
		{
			if (childNode->name != U"FileTypeFilter")
				continue;

			if (!childNode->propertyValuePairs[U"Description"])
				throw Exception{U"Failed to parse 'FileTypeFilter' property, no Description property found"};

			if (!childNode->propertyValuePairs[U"Pattern"])
				throw Exception{U"Failed to parse 'FileTypeFilter' property, no Pattern property found"};
			if (!childNode->propertyValuePairs[U"Pattern"]->listNode)
				throw Exception{U"Failed to parse 'Pattern' property inside the 'FileTypeFilter' property, expected a list as value"};

			const String description = Deserializer::deserialize(ObjectConverter::Type::String, childNode->propertyValuePairs[U"Description"]->value).getString();

			std::vector<String> patterns;
			for (const auto& item : childNode->propertyValuePairs[U"Pattern"]->valueList)
				patterns.push_back(Deserializer::deserialize(ObjectConverter::Type::String, item).getString());

			fileTypeFilters.emplace_back(description, patterns);
		}

		// We have to remove FileTypeFilter nodes before calling the load function on the base class, because Container will
		// assume that all child sections are widgets.
		node->children.erase(std::remove_if(node->children.begin(), node->children.end(), [](const std::unique_ptr<DataIO::Node>& child){
			return child->name == U"FileTypeFilter";
		}), node->children.end());

		// Remove the widgets that the constructor created because they will be created when loading the child window
		removeAllWidgets();

		ChildWindow::load(node, renderers);

		identifyChildWidgets();

		if (!m_buttonBack || !m_buttonForward || !m_buttonUp || !m_editBoxPath || !m_listView || !m_labelFilename
		 || !m_editBoxFilename || !m_comboBoxFileTypes || !m_buttonCancel || !m_buttonConfirm)
			throw Exception{U"Failed to find all internal child widgets while loading FileDialog"};

		connectSignals();

		if (node->propertyValuePairs[U"FileMustExist"])
			setFileMustExist(Deserializer::deserialize(ObjectConverter::Type::Bool, node->propertyValuePairs[U"FileMustExist"]->value).getBool());
		if (node->propertyValuePairs[U"SelectingDirectory"])
			setSelectingDirectory(Deserializer::deserialize(ObjectConverter::Type::Bool, node->propertyValuePairs[U"SelectingDirectory"]->value).getBool());
		if (node->propertyValuePairs[U"MultiSelect"])
			setMultiSelect(Deserializer::deserialize(ObjectConverter::Type::Bool, node->propertyValuePairs[U"MultiSelect"]->value).getBool());

		if (!fileTypeFilters.empty())
		{
			std::size_t filterIndex = 0;
			if (node->propertyValuePairs[U"SelectedFileTypeFilter"])
				filterIndex = static_cast<std::size_t>(Deserializer::deserialize(ObjectConverter::Type::Number, node->propertyValuePairs[U"SelectedFileTypeFilter"]->value).getNumber());

			setFileTypeFilters(fileTypeFilters, filterIndex);
		}

		sortFilesInListView();
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PackageFileDialog::updateTime(Duration elapsedTime)
	{
		bool b = ChildWindow::updateTime(elapsedTime);
		auto offset = (m_usesMusic ? this->m_music.getPlayingOffset() : this->m_sound.getPlayingOffset()).asSeconds();
		auto size = (m_usesMusic ? this->m_music.getDuration() : this->m_soundBuffer.getDuration()).asSeconds();
		std::string text;

		if (size >= 3600) {
			text += std::to_string(static_cast<int>(offset / 3600));
			text += ":";
		}

		int m = static_cast<int>(offset / 60) % 60;
		int s = static_cast<int>(offset) % 60;

		if (m < 10 && size >= 3600)
			text += "0";
		text += std::to_string(m);
		text += ":";
		if (s < 10)
			text += "0";
		text += std::to_string(s);

		text += " / ";
		if (size >= 3600) {
			text += std::to_string(static_cast<int>(offset / 3600));
			text += ":";
		}

		m = static_cast<int>(size / 60) % 60;
		s = static_cast<int>(size) % 60;
		if (m < 10 && size >= 3600)
			text += "0";
		text += std::to_string(m);
		text += ":";
		if (s < 10)
			text += "0";
		text += std::to_string(s);

		m_playLabel->setText(text);
		m_playSlider->onValueChange.setEnabled(false);
		m_playSlider->setValue(offset);
		m_playSlider->onValueChange.setEnabled(true);

		auto status = m_usesMusic ? m_music.getStatus() : m_sound.getStatus();

		if (m_lastStatus != status) {
			m_lastStatus = status;
			if (m_lastStatus == sf::Sound::Status::Playing)
				m_playPause->setImage("assets/gui/editor/pause.png");
			else
				m_playPause->setImage("assets/gui/editor/play.png");
		}
		return b;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Widget::Ptr PackageFileDialog::clone() const
	{
		return std::make_shared<PackageFileDialog>(*this);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	PackageStream::PackageStream(ShadyCore::BasePackageEntry *entry) :
		entry(entry),
		stream(entry->open()),
		size(entry->getSize())
	{
	}

	PackageStream::~PackageStream()
	{
		entry->close(stream);
	}

	std::optional<std::size_t> PackageStream::read(void *data, std::size_t s)
	{
		stream.read(static_cast<std::istream::char_type *>(data), s);
		if (!stream)
			return std::nullopt;
		return stream.gcount();
	}

	std::optional<std::size_t> PackageStream::seek(std::size_t position)
	{
		stream.seekg(position);
		if (!stream)
			return std::nullopt;
		return stream.tellg();
	}

	std::optional<std::size_t> PackageStream::tell()
	{
		if (!stream)
			return std::nullopt;
		return stream.tellg();
	}

	std::optional<std::size_t> PackageStream::getSize()
	{
		return size - stream.tellg();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

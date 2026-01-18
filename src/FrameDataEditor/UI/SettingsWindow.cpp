//
// Created by PinkySmile on 08/05/25.
//

#include "SettingsWindow.hpp"

SpiralOfFate::SettingsWindow::SettingsWindow(FrameDataEditor &editor) :
	LocalizedContainer<tgui::ChildWindow>(editor)
{
	this->loadLocalizedWidgetsFromFile("assets/gui/editor/settings.gui");
	Utils::setRenderer(*this);
	this->_oldSwr = game->settings.swr;
	this->_oldSoku = game->settings.soku;
	this->_oldSoku2 = game->settings.soku2;
	this->_oldExtra = game->settings.extra;
	this->_oldTheme = game->settings.theme;
	this->_oldLocale = editor.getLocale();
	this->_resize();

	auto locale = this->get<tgui::ComboBox>("Locale");
	auto theme = this->get<tgui::ComboBox>("Theme");
	auto soku2Path = this->get<tgui::EditBox>("Soku2Path");
	auto sokuPath = this->get<tgui::EditBox>("SokuPath");
	auto swrPath = this->get<tgui::EditBox>("SwrPath");
	auto soku2PathBrowse = this->get<tgui::Button>("Soku2PathBrowse");
	auto sokuPathBrowse = this->get<tgui::Button>("SokuPathBrowse");
	auto swrPathBrowse = this->get<tgui::Button>("SwrPathBrowse");

	locale->addItem(editor.localize("settings.locale."), "settings.locale.");
	for (auto &entry : std::filesystem::directory_iterator("assets/gui/editor/locale/")) {
		auto name = entry.path().stem().string();

		locale->addItem(editor.localize("settings.locale." + name), "settings.locale." + name);
	}
	locale->setSelectedItemById("settings.locale." + this->_oldLocale);

	for (auto &entry : std::filesystem::directory_iterator("assets/gui/themes/")) {
		auto ext = entry.path().extension().string();
		auto stem = entry.path().stem().string();

		if (ext != ".style")
			continue;
		theme->addItem(stem, stem);
	}
	if (game->settings.theme.rfind("assets/gui/themes/", 0) == 0) {
		auto path = game->settings.theme.substr(strlen("assets/gui/themes/"));

		if (path.find('/') == std::string::npos)
			theme->setSelectedItemById(path.substr(0, path.find('.')));
	}
	soku2Path->setText(game->settings.soku2.native());
	sokuPath->setText(game->settings.soku.native());
	swrPath->setText(game->settings.swr.native());
	this->checkPackages();

	// TODO: Extra assets


	locale->onItemSelect([&editor](const tgui::String &, const tgui::String &id){
		size_t pos = id.find_last_of('.');
		auto locale = id.substr(pos + 1);

		editor.setLocale(locale.toStdString());
	});
	theme->onItemSelect([this, &editor](const tgui::String &, const tgui::String &id){
		tgui::Theme::setDefault(tgui::Theme::create("assets/gui/themes/" + id + ".style"));
		Utils::setRenderer(game->gui);
		this->_resize();
		editor.refreshInterface();
		this->checkPackages();
	});
	soku2Path->onTextChange([this] (const tgui::String &s){
		game->settings.soku2 = static_cast<std::filesystem::path::string_type>(s);
		this->checkPackages();
	});
	sokuPath->onTextChange([this] (const std::weak_ptr<tgui::EditBox> &other, const tgui::String &s){
		game->settings.soku = static_cast<std::filesystem::path::string_type>(s);
		if (this->_checkSokuPackages() && !this->_checkSwrPackages()) {
			auto old = game->settings.swr;

			game->settings.swr = game->settings.soku;
			if (this->_checkSwrPackages()) {
				auto ptr = other.lock();

				ptr->onTextChange.setEnabled(false);
				ptr->setText(s);
				ptr->onTextChange.setEnabled(true);
			} else
				game->settings.swr = old;

		}
	}, std::weak_ptr(swrPath));
	swrPath->onTextChange([this] (const std::weak_ptr<tgui::EditBox> &other, const tgui::String &s){
		game->settings.swr = static_cast<std::filesystem::path::string_type>(s);
		if (this->_checkSwrPackages() && !this->_checkSokuPackages()) {
			auto old = game->settings.soku;

			game->settings.soku = game->settings.swr;
			if (this->_checkSokuPackages()) {
				auto ptr = other.lock();

				ptr->onTextChange.setEnabled(false);
				ptr->setText(s);
				ptr->onTextChange.setEnabled(true);
			} else
				game->settings.soku = old;

		}
	}, std::weak_ptr(sokuPath));
	soku2PathBrowse->onClick([soku2Path, &editor] {
		auto win = Utils::openFileDialog(game->gui, editor.localize("settings.browse.soku2"));

		win->setFileTypeFilters({
			{editor.localize("settings.type.soku2"), {"Soku2.dll"}}
		});
		win->onFileSelect([soku2Path] (const std::vector<tgui::Filesystem::Path> &file){
			soku2Path->setText(file[0].getParentPath().asString());
		});
	});
	sokuPathBrowse->onClick([sokuPath, &editor] {
		auto win = Utils::openFileDialog(game->gui, editor.localize("settings.browse.soku"));

		win->setFileTypeFilters({
			{editor.localize("settings.type.soku"), {"th123a.dat", "th123b.dat", "th123c.dat"}}
		});
		win->onFileSelect([sokuPath] (const std::vector<tgui::Filesystem::Path> &file){
			sokuPath->setText(file[0].getParentPath().asString());
		});
	});
	swrPathBrowse->onClick([swrPath, &editor] {
		auto win = Utils::openFileDialog(game->gui, editor.localize("settings.browse.swr"));

		win->setFileTypeFilters({
			{editor.localize("settings.type.swr"), {"th105a.dat", "th105b.dat"}}
		});
		win->onFileSelect([swrPath] (const std::vector<tgui::Filesystem::Path> &file){
			swrPath->setText(file[0].getParentPath().asString());
		});
	});


	auto cancel = this->get<tgui::Button>("Cancel");
	auto save = this->get<tgui::Button>("Save");

	this->onClosing.connect([this, &editor] (bool *abort){
		if (this->_saved)
			return;

		auto _swr = game->settings.swr;
		auto _soku = game->settings.soku;
		auto _soku2 = game->settings.soku2;
		auto _extra = game->settings.extra;

		game->settings.swr = this->_oldSwr;
		game->settings.soku = this->_oldSoku;
		game->settings.soku2 = this->_oldSoku2;
		game->settings.extra = this->_oldExtra;
		*abort = !this->checkPackages();
		if (!*abort) {
			editor.reloadGamePackages();
			return;
		}
		game->settings.swr = _swr;
		game->settings.soku = _soku;
		game->settings.soku2 = _soku2;
		game->settings.extra = _extra;
	});
	cancel->onClick([this]{
		this->close();
	});
	save->onClick([this, &editor]{
		if (!this->checkPackages())
			return;
		try {
			editor.reloadGamePackages();
		} catch (std::exception &e) {
			Utils::dispMsg(game->gui, "Failed to reload packages", e.what(), MB_ICONERROR);
		}
		game->settings.theme = tgui::Theme::getDefault()->getPrimary().toStdString();
		game->settings.save();
		editor.saveSettings();
		this->_saved = true;
		this->close();
	});
	this->onClose([this, &editor]{
		if (this->_saved)
			return;
		game->settings.swr = this->_oldSwr;
		game->settings.soku = this->_oldSoku;
		game->settings.soku2 = this->_oldSoku2;
		game->settings.extra = this->_oldExtra;
		editor.setLocale(this->_oldLocale);
		tgui::Theme::setDefault(tgui::Theme::create(this->_oldTheme));
		Utils::setRenderer(game->gui);
		editor.refreshInterface();
	});
}

bool SpiralOfFate::SettingsWindow::_checkSwrPackages()
{
	auto swrError = this->get<tgui::Label>("SwrPathError");
	std::vector<std::string> swrFiles{"th105a.dat", "th105b.dat"};

	swrError->getRenderer()->setTextColor(tgui::Color::Red);
	if (game->settings.swr.empty()) {
		swrError->setText(this->_editor.localize("settings.provide_path"));
		swrError->setSize({"&.w - x - 10", 17});
		return false;
	}
	for (size_t i = 0; i < swrFiles.size();) {
		if (std::filesystem::exists(game->settings.soku / swrFiles[i]))
			swrFiles.erase(swrFiles.begin() + i);
		else
			i++;
	}
	if (swrFiles.empty()) {
		swrError->setText("");
		swrError->setSize({0, 0});
		return true;
	}

	std::string files;

	for (auto &file : swrFiles) {
		if (!files.empty())
			files += ", ";
		files += file;
	}
	if (swrFiles.size() == 1)
		swrError->setText(this->_editor.localize("settings.missing_file", files));
	else
		swrError->setText(this->_editor.localize("settings.missing_files", files));
	swrError->setSize({"&.w - x - 10", 17});
	return false;
}

bool SpiralOfFate::SettingsWindow::_checkSokuPackages()
{
	auto sokuError = this->get<tgui::Label>("SokuPathError");
	std::vector<std::string> sokuFiles{"th123a.dat", "th123b.dat", "th123c.dat"};

	sokuError->getRenderer()->setTextColor(tgui::Color::Red);
	if (game->settings.soku.empty()) {
		sokuError->setText(this->_editor.localize("settings.provide_path"));
		sokuError->setSize({"&.w - x - 10", 17});
		return false;
	}
	for (size_t i = 0; i < sokuFiles.size();) {
		if (std::filesystem::exists(game->settings.soku / sokuFiles[i]))
			sokuFiles.erase(sokuFiles.begin() + i);
		else
			i++;
	}
	if (sokuFiles.empty()) {
		sokuError->setText("");
		sokuError->setSize({0, 0});
		return true;
	}

	std::string files;

	for (auto &file : sokuFiles) {
		if (!files.empty())
			files += ", ";
		files += file;
	}
	if (sokuFiles.size() == 1)
		sokuError->setText(this->_editor.localize("settings.missing_file", files));
	else
		sokuError->setText(this->_editor.localize("settings.missing_files", files));
	sokuError->setSize({"&.w - x - 10", 17});
	return false;
}

bool SpiralOfFate::SettingsWindow::_checkSoku2Packages()
{
	auto soku2Error = this->get<tgui::Label>("Soku2PathError");

	soku2Error->getRenderer()->setTextColor(tgui::Color::Red);
	if (!game->settings.soku2.empty()) {
		std::error_code err;
		size_t chars = 0;
		std::filesystem::directory_iterator it{game->settings.soku2 / "characters", err};

		if (err) {
			soku2Error->getRenderer()->setTextColor(tgui::Color::Red);
			soku2Error->setText(this->_editor.localize("settings.invalid_path"));
			soku2Error->setSize({"&.w - x - 10", 17});
			return false;
		}
		for (auto &entry : it) {
			if (!entry.is_directory())
				continue;
			for (auto &entry2 : std::filesystem::directory_iterator(entry.path()))
				if (entry2.path().extension().string() == ".dat")
					chars++;
		}
		if (chars == 0) {
			soku2Error->getRenderer()->setTextColor(tgui::Color{0xFF, 0x80, 0x00, 0xFF});
			soku2Error->setText(this->_editor.localize("settings.no_characters"));
			soku2Error->setSize({"&.w - x - 10", 17});
		} else {
			soku2Error->setText("");
			soku2Error->setSize({0, 0});
		}
	} else {
		soku2Error->setText("");
		soku2Error->setSize({0, 0});
	}
	return true;
}

bool SpiralOfFate::SettingsWindow::checkPackages()
{
	bool b = this->_checkSwrPackages();

	b &= this->_checkSokuPackages();
	b &= this->_checkSoku2Packages();
	return b;
}

SpiralOfFate::SettingsWindow::Ptr SpiralOfFate::SettingsWindow::create(SpiralOfFate::FrameDataEditor &editor)
{
	return std::make_shared<SettingsWindow>(editor);
}

void SpiralOfFate::SettingsWindow::_resize()
{
	auto size = this->getSize();
	auto panel = this->get<tgui::Panel>("Panel");
	auto innerSize = this->getInnerSize();
	auto expectedSize = panel->getSize();
	auto result = size + expectedSize - innerSize;

	this->setSize(result);
}

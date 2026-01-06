//
// Created by PinkySmile on 08/05/25.
//

#include "SettingsWindow.hpp"

SpiralOfFate::SettingsWindow::SettingsWindow(FrameDataEditor &editor) :
	LocalizedContainer<tgui::ChildWindow>(editor)
{
	this->loadLocalizedWidgetsFromFile("assets/gui/editor/settings.gui");
	Utils::setRenderer(*this);
	this->_oldTheme = game->settings.theme;
	this->_oldLocale = editor.getLocale();
	this->_resize();

	auto locale = this->get<tgui::ComboBox>("Locale");
	auto theme = this->get<tgui::ComboBox>("Theme");

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
	});

	auto cancel = this->get<tgui::Button>("Cancel");
	auto save = this->get<tgui::Button>("Save");

	cancel->onClick([this]{
		this->close();
	});
	save->onClick([this, &editor]{
		game->settings.theme = tgui::Theme::getDefault()->getPrimary().toStdString();
		game->settings.save();
		editor.saveSettings();
		this->_saved = true;
		this->close();
	});
	this->onClose([this, &editor]{
		if (this->_saved)
			return;
		editor.setLocale(this->_oldLocale);
		tgui::Theme::setDefault(tgui::Theme::create(this->_oldTheme));
		Utils::setRenderer(game->gui);
		editor.refreshInterface();
	});
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

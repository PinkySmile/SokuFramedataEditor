//
// Created by PinkySmile on 09/04/25.
//

#include <nlohmann/json.hpp>
#include "FrameDataEditor.hpp"
#include "UI/MainWindow.hpp"
#include "UI/LocalizedContainer.hpp"
#include "UI/SettingsWindow.hpp"

template<typename T>
void localizeGui(T &container)
{
	using namespace SpiralOfFate;

	for (const tgui::Widget::Ptr &w : container.getWidgets()) {
		if (auto loc1 = w->cast<LocalizedContainer<tgui::ChildWindow>>())
			loc1->reLocalize();
		else if (auto loc2 = w->cast<LocalizedContainer<MainWindow>>())
			loc2->reLocalize();
		else if (auto loc3 = w->cast<LocalizedContainer<SettingsWindow>>())
			loc3->reLocalize();
		else if (auto loc4 = w->cast<LocalizedContainer<tgui::ScrollablePanel>>())
			loc4->reLocalize();
		else if (auto cont = w->cast<tgui::Container>())
			localizeGui(*cont);
	}
}

SpiralOfFate::FrameDataEditor::FrameDataEditor()
{
	game->gui.loadWidgetsFromFile("assets/gui/editor/layout.gui");
	this->_clock.stop();
	this->_loadSettings();

	auto menu = game->gui.get<tgui::MenuBar>("MainBar");

	this->_shortcutsNames["menu_item.file.new"]           = { .code = sf::Keyboard::Key::N,      .alt = false, .control = true,  .shift = false, .meta = false };
	this->_shortcutsNames["menu_item.file.load"]          = { .code = sf::Keyboard::Key::O,      .alt = false, .control = true,  .shift = false, .meta = false };
	this->_shortcutsNames["menu_item.file.save"]          = { .code = sf::Keyboard::Key::S,      .alt = false, .control = true,  .shift = false, .meta = false };
	this->_shortcutsNames["menu_item.file.save_as"]       = { .code = sf::Keyboard::Key::S,      .alt = false, .control = true,  .shift = true,  .meta = false };
	// this->_shortcutsNames["menu_item.file.settings"]      = { .code = sf::Keyboard::Key::LAlt, .alt = false, .control = false, .shift = false, .meta = false };
	// this->_shortcutsNames["menu_item.file.shortcuts"]     = { .code = sf::Keyboard::Key::LAlt, .alt = false, .control = false, .shift = false, .meta = false };
	this->_shortcutsNames["menu_item.file.quit"]          = { .code = sf::Keyboard::Key::Q,      .alt = false, .control = true,  .shift = false, .meta = false };
	this->_shortcutsNames["menu_item.edit.undo"]          = { .code = sf::Keyboard::Key::Z,      .alt = false, .control = true,  .shift = false, .meta = false };
	this->_shortcutsNames["menu_item.edit.redo"]          = { .code = sf::Keyboard::Key::Y,      .alt = false, .control = true,  .shift = false, .meta = false };
	this->_shortcutsNames["menu_item.new.frame"]          = { .code = sf::Keyboard::Key::F,      .alt = false, .control = true,  .shift = false, .meta = false };
	this->_shortcutsNames["menu_item.new.frame_end"]      = { .code = sf::Keyboard::Key::F,      .alt = false, .control = true,  .shift = true,  .meta = false };
	this->_shortcutsNames["menu_item.new.block"]          = { .code = sf::Keyboard::Key::B,      .alt = false, .control = true,  .shift = false, .meta = false };
	this->_shortcutsNames["menu_item.new.action"]         = { .code = sf::Keyboard::Key::A,      .alt = false, .control = true,  .shift = false, .meta = false };
	this->_shortcutsNames["menu_item.new.hurt_box"]       = { .code = sf::Keyboard::Key::H,      .alt = false, .control = true,  .shift = false, .meta = false };
	this->_shortcutsNames["menu_item.new.hit_box"]        = { .code = sf::Keyboard::Key::H,      .alt = false, .control = true,  .shift = true,  .meta = false };
	this->_shortcutsNames["menu_item.remove.frame"]       = { .code = sf::Keyboard::Key::Delete, .alt = false, .control = true,  .shift = true,  .meta = false };
	this->_shortcutsNames["menu_item.remove.block"]       = { .code = sf::Keyboard::Key::Delete, .alt = false, .control = true,  .shift = false, .meta = false };
	this->_shortcutsNames["menu_item.remove.action"]      = { .code = sf::Keyboard::Key::Delete, .alt = false, .control = false, .shift = true, .meta = true };
	this->_shortcutsNames["menu_item.misc.copy_box_last"] = { .code = sf::Keyboard::Key::I,      .alt = false, .control = true, .shift = false, .meta = false };
	this->_shortcutsNames["menu_item.misc.copy_box_next"] = { .code = sf::Keyboard::Key::O,      .alt = false, .control = true, .shift = false, .meta = false };
	this->_shortcutsNames["menu_item.misc.flatten"]       = { .code = sf::Keyboard::Key::K,      .alt = false, .control = true, .shift = false, .meta = false };
	this->_shortcutsNames["menu_item.misc.reload"]        = { .code = sf::Keyboard::Key::R,      .alt = false, .control = true, .shift = true, .meta = false };
	this->_menuHierarchy = menu->getMenus();
	this->setLocale(this->_locale);
	Utils::setRenderer(game->gui);
}

SpiralOfFate::FrameDataEditor::~FrameDataEditor()
{
	this->saveSettings();
}

bool SpiralOfFate::FrameDataEditor::closeAll()
{
	auto vec = this->_openWindows;

	// TODO: Actually end the program if all popups are answered with no
	for (auto &widget : vec)
		if (!widget->isModified())
			widget->close();
	if (this->_openWindows.empty())
		return true;
	Utils::dispMsg(game->gui, this->localize("message_box.title.not_saved"), this->localize("message_box.not_saved"), MB_ICONINFORMATION);
	return false;
}

void SpiralOfFate::FrameDataEditor::_loadSettings()
{
	std::ifstream stream{"editorSettings.json"};
	nlohmann::json json;

	if (!stream.fail()) {
		stream >> json;
		if (json.contains("locale") && json["locale"].is_string())
			this->_locale = json["locale"];
	} else if (errno != ENOENT)
		throw std::runtime_error("Cannot open settings file: editorSettings.json: " + std::string(strerror(errno)));
	else {
		this->_locale = "";
	}
}

void SpiralOfFate::FrameDataEditor::saveSettings()
{
	std::ofstream stream{"editorSettings.json"};
	nlohmann::json json = {
		{ "locale", this->_locale }
	};

	if (!stream)
		throw std::runtime_error("Cannot open settings file: editorSettings.json: " + std::string(strerror(errno)));
	stream << json.dump(4);
}

bool SpiralOfFate::FrameDataEditor::hasLocalization(const std::string &s) const
{
	return this->_localization.count(s) != 0;
}

std::string SpiralOfFate::FrameDataEditor::localize(const std::string &s) const
{
	auto it = this->_localization.find(s);

	if (it == this->_localization.end())
		return s;
	return it->second;
}

std::string SpiralOfFate::FrameDataEditor::_shortcutToString(const SpiralOfFate::FrameDataEditor::Shortcut &s)
{
	std::string result;

	switch (s.code) {
	case sf::Keyboard::Key::A:
		result = "A";
		break;
	case sf::Keyboard::Key::B:
		result = "B";
		break;
	case sf::Keyboard::Key::C:
		result = "C";
		break;
	case sf::Keyboard::Key::D:
		result = "D";
		break;
	case sf::Keyboard::Key::E:
		result = "E";
		break;
	case sf::Keyboard::Key::F:
		result = "F";
		break;
	case sf::Keyboard::Key::G:
		result = "G";
		break;
	case sf::Keyboard::Key::H:
		result = "H";
		break;
	case sf::Keyboard::Key::I:
		result = "I";
		break;
	case sf::Keyboard::Key::J:
		result = "J";
		break;
	case sf::Keyboard::Key::K:
		result = "K";
		break;
	case sf::Keyboard::Key::L:
		result = "L";
		break;
	case sf::Keyboard::Key::M:
		result = "M";
		break;
	case sf::Keyboard::Key::N:
		result = "N";
		break;
	case sf::Keyboard::Key::O:
		result = "O";
		break;
	case sf::Keyboard::Key::P:
		result = "P";
		break;
	case sf::Keyboard::Key::Q:
		result = "Q";
		break;
	case sf::Keyboard::Key::R:
		result = "R";
		break;
	case sf::Keyboard::Key::S:
		result = "S";
		break;
	case sf::Keyboard::Key::T:
		result = "T";
		break;
	case sf::Keyboard::Key::U:
		result = "U";
		break;
	case sf::Keyboard::Key::V:
		result = "V";
		break;
	case sf::Keyboard::Key::W:
		result = "W";
		break;
	case sf::Keyboard::Key::X:
		result = "X";
		break;
	case sf::Keyboard::Key::Y:
		result = "Y";
		break;
	case sf::Keyboard::Key::Z:
		result = "Z";
		break;
	case sf::Keyboard::Key::Num0:
		result = "0";
		break;
	case sf::Keyboard::Key::Num1:
		result = "1";
		break;
	case sf::Keyboard::Key::Num2:
		result = "2";
		break;
	case sf::Keyboard::Key::Num3:
		result = "3";
		break;
	case sf::Keyboard::Key::Num4:
		result = "4";
		break;
	case sf::Keyboard::Key::Num5:
		result = "5";
		break;
	case sf::Keyboard::Key::Num6:
		result = "6";
		break;
	case sf::Keyboard::Key::Num7:
		result = "7";
		break;
	case sf::Keyboard::Key::Num8:
		result = "8";
		break;
	case sf::Keyboard::Key::Num9:
		result = "9";
		break;
	case sf::Keyboard::Key::Escape:
		result = "Esc";
		break;
	case sf::Keyboard::Key::LControl:
		result = "LCtrl";
		break;
	case sf::Keyboard::Key::LShift:
		result = "LShift";
		break;
	case sf::Keyboard::Key::LAlt:
		result = "LAlt";
		break;
	case sf::Keyboard::Key::LSystem:
		result = "LMeta";
		break;
	case sf::Keyboard::Key::RControl:
		result = "RCtrl";
		break;
	case sf::Keyboard::Key::RShift:
		result = "RShift";
		break;
	case sf::Keyboard::Key::RAlt:
		result = "RAlt";
		break;
	case sf::Keyboard::Key::RSystem:
		result = "RMeta";
		break;
	case sf::Keyboard::Key::Menu:
		result = "Menu";
		break;
	case sf::Keyboard::Key::LBracket:
		result = "[";
		break;
	case sf::Keyboard::Key::RBracket:
		result = "]";
		break;
	case sf::Keyboard::Key::Semicolon:
		result = ";";
		break;
	case sf::Keyboard::Key::Comma:
		result = ",";
		break;
	case sf::Keyboard::Key::Period:
		result = ".";
		break;
	case sf::Keyboard::Key::Apostrophe:
		result = "'";
		break;
	case sf::Keyboard::Key::Slash:
		result = "/";
		break;
	case sf::Keyboard::Key::Backslash:
		result = "\\";
		break;
	case sf::Keyboard::Key::Grave:
		result = "`";
		break;
	case sf::Keyboard::Key::Equal:
		result = "=";
		break;
	case sf::Keyboard::Key::Hyphen:
		result = "-";
		break;
	case sf::Keyboard::Key::Space:
		result = "Space";
		break;
	case sf::Keyboard::Key::Enter:
		result = "Enter";
		break;
	case sf::Keyboard::Key::Backspace:
		result = "BSpace";
		break;
	case sf::Keyboard::Key::Tab:
		result = "Tab";
		break;
	case sf::Keyboard::Key::PageUp:
		result = "PgUp";
		break;
	case sf::Keyboard::Key::PageDown:
		result = "PgDown";
		break;
	case sf::Keyboard::Key::End:
		result = "End";
		break;
	case sf::Keyboard::Key::Home:
		result = "Home";
		break;
	case sf::Keyboard::Key::Insert:
		result = "Insert";
		break;
	case sf::Keyboard::Key::Delete:
		result = "Del";
		break;
	case sf::Keyboard::Key::Add:
		result = "+";
		break;
	case sf::Keyboard::Key::Subtract:
		result = "Min";
		break;
	case sf::Keyboard::Key::Multiply:
		result = "Mul";
		break;
	case sf::Keyboard::Key::Divide:
		result = "Div";
		break;
	case sf::Keyboard::Key::Left:
		result = "Left";
		break;
	case sf::Keyboard::Key::Right:
		result = "Right";
		break;
	case sf::Keyboard::Key::Up:
		result = "Up";
		break;
	case sf::Keyboard::Key::Down:
		result = "Down";
		break;
	case sf::Keyboard::Key::Numpad0:
		result = "Num0";
		break;
	case sf::Keyboard::Key::Numpad1:
		result = "Num1";
		break;
	case sf::Keyboard::Key::Numpad2:
		result = "Num2";
		break;
	case sf::Keyboard::Key::Numpad3:
		result = "Num3";
		break;
	case sf::Keyboard::Key::Numpad4:
		result = "Num4";
		break;
	case sf::Keyboard::Key::Numpad5:
		result = "Num5";
		break;
	case sf::Keyboard::Key::Numpad6:
		result = "Num6";
		break;
	case sf::Keyboard::Key::Numpad7:
		result = "Num7";
		break;
	case sf::Keyboard::Key::Numpad8:
		result = "Num8";
		break;
	case sf::Keyboard::Key::Numpad9:
		result = "Num9";
		break;
	case sf::Keyboard::Key::F1:
		result = "F1";
		break;
	case sf::Keyboard::Key::F2:
		result = "F2";
		break;
	case sf::Keyboard::Key::F3:
		result = "F3";
		break;
	case sf::Keyboard::Key::F4:
		result = "F4";
		break;
	case sf::Keyboard::Key::F5:
		result = "F5";
		break;
	case sf::Keyboard::Key::F6:
		result = "F6";
		break;
	case sf::Keyboard::Key::F7:
		result = "F7";
		break;
	case sf::Keyboard::Key::F8:
		result = "F8";
		break;
	case sf::Keyboard::Key::F9:
		result = "F9";
		break;
	case sf::Keyboard::Key::F10:
		result = "F10";
		break;
	case sf::Keyboard::Key::F11:
		result = "F11";
		break;
	case sf::Keyboard::Key::F12:
		result = "F12";
		break;
	case sf::Keyboard::Key::F13:
		result = "F13";
		break;
	case sf::Keyboard::Key::F14:
		result = "F14";
		break;
	case sf::Keyboard::Key::F15:
		result = "F15";
		break;
	case sf::Keyboard::Key::Pause:
		result = "Pause";
		break;
	default:
		result = "Unknown";
		break;
	}
	if (s.control && s.code != sf::Keyboard::Key::LControl && s.code != sf::Keyboard::Key::RControl)
		result = "Ctrl+" + result;
	if (s.alt && s.code != sf::Keyboard::Key::LAlt && s.code != sf::Keyboard::Key::RAlt)
		result = "Alt+" + result;
	if (s.meta && s.code != sf::Keyboard::Key::LSystem && s.code != sf::Keyboard::Key::RSystem)
		result = "Meta+" + result;
	if (s.shift && s.code != sf::Keyboard::Key::LShift && s.code != sf::Keyboard::Key::RShift)
		result = "Shift+" + result;
	return result;
}

std::string SpiralOfFate::FrameDataEditor::localizeShortcut(const std::string &s) const
{
	std::string name = this->localize(s);
	auto it = this->_shortcutsNames.find(s);

	if (it != this->_shortcutsNames.end()) {
		std::string sname = FrameDataEditor::_shortcutToString(it->second);

		name += " (";
		name += sname;
		name.push_back(')');
	}
	return name;
}

void SpiralOfFate::FrameDataEditor::_connectShortcut(const tgui::MenuBar::Ptr &menu, const std::vector<std::string> &hierarchy, void (FrameDataEditor::*callback)())
{
	std::vector<tgui::String> tguiHierarchy;

	tguiHierarchy.reserve(hierarchy.size());
	for (size_t i = 0; i < hierarchy.size() - 1; i++)
		tguiHierarchy.emplace_back(this->localize(hierarchy[i]));

	auto it = this->_shortcutsNames.find(hierarchy.back());

	if (it != this->_shortcutsNames.end()) {
		game->logger.debug(std::string(hierarchy.back()) + " shortcut is " + FrameDataEditor::_shortcutToString(it->second));
		this->_shortcuts.emplace(it->second, callback);
	}
	tguiHierarchy.emplace_back(this->localizeShortcut(hierarchy.back()));
	menu->connectMenuItem(tguiHierarchy, callback, this);
}

void SpiralOfFate::FrameDataEditor::_placeMenuCallbacks(const tgui::MenuBar::Ptr &menu)
{
	this->_connectShortcut(menu, { "menu_item.file", "menu_item.file.new"       }, &FrameDataEditor::_newFramedata);
	this->_connectShortcut(menu, { "menu_item.file", "menu_item.file.load"      }, &FrameDataEditor::_loadFramedata);
	this->_connectShortcut(menu, { "menu_item.file", "menu_item.file.save"      }, &FrameDataEditor::_save);
	this->_connectShortcut(menu, { "menu_item.file", "menu_item.file.save_as"   }, &FrameDataEditor::_saveAs);
	this->_connectShortcut(menu, { "menu_item.file", "menu_item.file.settings"  }, &FrameDataEditor::_settings);
	this->_connectShortcut(menu, { "menu_item.file", "menu_item.file.shortcuts" }, &FrameDataEditor::_editShortcuts);
	this->_connectShortcut(menu, { "menu_item.file", "menu_item.file.quit"      }, &FrameDataEditor::_quit);

	this->_connectShortcut(menu, { "menu_item.edit", "menu_item.edit.undo"  }, &FrameDataEditor::_undo);
	this->_connectShortcut(menu, { "menu_item.edit", "menu_item.edit.redo"  }, &FrameDataEditor::_redo);

	this->_connectShortcut(menu, { "menu_item.new", "menu_item.new.frame"     }, &FrameDataEditor::_newFrame);
	this->_connectShortcut(menu, { "menu_item.new", "menu_item.new.frame_end" }, &FrameDataEditor::_newEndFrame);
	this->_connectShortcut(menu, { "menu_item.new", "menu_item.new.block"     }, &FrameDataEditor::_newAnimationBlock);
	this->_connectShortcut(menu, { "menu_item.new", "menu_item.new.action"    }, &FrameDataEditor::_newAction);
	this->_connectShortcut(menu, { "menu_item.new", "menu_item.new.hurt_box"  }, &FrameDataEditor::_newHurtBox);
	this->_connectShortcut(menu, { "menu_item.new", "menu_item.new.hit_box"   }, &FrameDataEditor::_newHitBox);

	this->_connectShortcut(menu, { "menu_item.remove", "menu_item.remove.frame"  }, &FrameDataEditor::_removeFrame);
	this->_connectShortcut(menu, { "menu_item.remove", "menu_item.remove.block"  }, &FrameDataEditor::_removeAnimationBlock);
	this->_connectShortcut(menu, { "menu_item.remove", "menu_item.remove.action" }, &FrameDataEditor::_removeAction);

	this->_connectShortcut(menu, { "menu_item.misc", "menu_item.misc.copy_box_last" }, &FrameDataEditor::_copyBoxesFromLastFrame);
	this->_connectShortcut(menu, { "menu_item.misc", "menu_item.misc.copy_box_next" }, &FrameDataEditor::_copyBoxesFromNextFrame);
	this->_connectShortcut(menu, { "menu_item.misc", "menu_item.misc.flatten"       }, &FrameDataEditor::_flattenThisMoveCollisionBoxes);
	this->_connectShortcut(menu, { "menu_item.misc", "menu_item.misc.reload"        }, &FrameDataEditor::_reloadTextures);
}

void SpiralOfFate::FrameDataEditor::_addMenu(const tgui::MenuBar::Ptr &menu, const tgui::MenuBar::GetMenusElement &element, std::vector<tgui::String> hierarchy)
{
	if (element.menuItems.empty()) {
		hierarchy.emplace_back(this->localizeShortcut(std::string(element.text)));
		menu->addMenuItem(hierarchy);
	} else {
		hierarchy.emplace_back(this->localize(std::string(element.text)));
		menu->addMenuItem(hierarchy);
		for (auto &d: element.menuItems)
			this->_addMenu(menu, d, hierarchy);
	}
}

void SpiralOfFate::FrameDataEditor::_buildMenu()
{
	auto menu = game->gui.get<tgui::MenuBar>("MainBar");

	this->_shortcuts.clear();
	game->gui.remove(menu);
	menu = tgui::MenuBar::create();
	game->gui.add(menu, "MainBar");
	game->gui.moveWidgetToBack(menu);
	for (auto &d : this->_menuHierarchy)
		this->_addMenu(menu, d, {});
	this->_placeMenuCallbacks(menu);
	this->_updateMenuBar();
}

void SpiralOfFate::FrameDataEditor::_updateMenuBar()
{
	auto menu = game->gui.get<tgui::MenuBar>("MainBar");

	if (this->_focusedWindow) {
		menu->setMenuEnabled(this->localize("menu_item.new"), true);
		menu->setMenuEnabled(this->localize("menu_item.edit"), true);
		menu->setMenuEnabled(this->localize("menu_item.misc"), true);
		menu->setMenuEnabled(this->localize("menu_item.remove"), true);
		menu->setMenuItemEnabled({ this->localize("menu_item.edit"), this->localize("menu_item.edit.undo") }, this->_focusedWindow->hasUndoData());
		menu->setMenuItemEnabled({ this->localize("menu_item.edit"), this->localize("menu_item.edit.redo") }, this->_focusedWindow->hasRedoData());
	} else {
		menu->setMenuEnabled(this->localize("menu_item.new"), false);
		menu->setMenuEnabled(this->localize("menu_item.edit"), false);
		menu->setMenuEnabled(this->localize("menu_item.misc"), false);
		menu->setMenuEnabled(this->localize("menu_item.remove"), false);
	}
}

void SpiralOfFate::FrameDataEditor::_tickAnimation()
{
	for (auto &window : this->_openWindows)
		window->tick();
}

void SpiralOfFate::FrameDataEditor::update()
{
	auto time = this->_clock.restart();

	this->_timer += std::min(200.f, time.asSeconds());
	while (this->_timer >= 1 / 60.f) {
		this->_timer -= 1 / 60.f;
		this->_tickAnimation();
	}
}

void SpiralOfFate::FrameDataEditor::render()
{
}

void SpiralOfFate::FrameDataEditor::setLocale(const std::string &name)
{
	std::string locale = name;
	nlohmann::json json = nlohmann::json::object();

	this->_locale = name;
	this->_localization.clear();
	if (locale.empty())
		locale = Utils::getLocale();

	std::ifstream stream{ "assets/gui/editor/locale/" + locale + ".json" };

	if (!stream && name.empty())
		stream.open("assets/gui/editor/locale/en.json");
	if (stream)
		stream >> json;
	if (json.is_object()) {
		for (auto &[key, value] : json.items()) {
			if (!value.is_string()) {
				this->_localization.clear();
				break;
			}
			this->_localization[key] = value;
		}
	}
	this->_buildMenu();
	localizeGui(game->gui);
	this->refreshInterface();
}

std::string SpiralOfFate::FrameDataEditor::getLocale() const
{
	return this->_locale;
}

void SpiralOfFate::FrameDataEditor::_newFramedata()
{
	// TODO: Not implemented
	Utils::dispMsg(game->gui, "Error", "Not implemented", MB_ICONERROR);
}

void SpiralOfFate::FrameDataEditor::_loadFramedata()
{
	// TODO: Hardcoded string
	auto file = Utils::openFileDialog(game->gui, "Open Character", "assets/characters");
	auto load = [this](const std::filesystem::path &path){
		try {
			this->_openWindows.emplace_back(new MainWindow(path, *this));
			this->_focusedWindow = this->_openWindows.back();
			this->_focusedWindow->onMousePress.connect([this]{
				this->_focusedWindow->setFocused(true);
			});
			this->_focusedWindow->onFocus.connect([this](const std::weak_ptr<MainWindow> &This){
				auto lock = This.lock();

				if (this->_focusedWindow != lock) {
					if (this->_focusedWindow)
						this->_focusedWindow->setFocused(false);
					this->_focusedWindow = lock;
					this->_updateMenuBar();
					this->setHasRedo(this->_focusedWindow->hasRedoData());
					this->setHasUndo(this->_focusedWindow->hasUndoData());
				}
			}, std::weak_ptr(this->_focusedWindow));
			this->_focusedWindow->onRealClose.connect([this](const std::weak_ptr<MainWindow> &This){
				this->_openWindows.erase(std::remove(this->_openWindows.begin(), this->_openWindows.end(), This.lock()), this->_openWindows.end());
			}, std::weak_ptr(this->_focusedWindow));
			game->gui.add(this->_focusedWindow);
			this->_focusedWindow->setFocused(true);
			this->_updateMenuBar();
			this->setHasRedo(false);
			this->setHasUndo(false);
			return true;
		} catch (_AssertionFailedException &e) { // TODO: Very dirty
			Utils::dispMsg(
				game->gui,
				this->localize("message_box.title.load_failed"),
				this->localize("message_box.invalid_file", e.what()),
				MB_ICONERROR
			);
		} catch (std::exception &e) {
			Utils::dispMsg(
				game->gui,
				this->localize("message_box.title.load_failed"),
				this->localize("message_box.internal_error", Utils::getLastExceptionName(), e.what()),
				MB_ICONERROR
			);
		}
		return false;
	};

	// TODO: Hardcoded string
	file->setFileTypeFilters({ {"Framedata file", {"*.json"}}, {"All files", {}} }, 0);
	file->setMultiSelect(true);
	file->onFileSelect.connect([load, this](const std::vector<tgui::Filesystem::Path> &arr) {
		for (auto &p : arr) {
			const std::filesystem::path &filePath = p;
			std::filesystem::path pathBackup = p;

			pathBackup += ".bak";
			if (!std::filesystem::exists(pathBackup)) {
				load(filePath);
				return;
			}

			auto dialog = Utils::dispMsg(
				game->gui,
				this->localize("message_box.title.backup_exists"),
				this->localize("message_box.backup_exists", filePath),
				MB_ICONINFORMATION
			);

			dialog->changeButtons({
				this->localize("message_box.button.yes"),
				this->localize("message_box.button.no")
			});
			dialog->onButtonPress.connect([this, load, filePath, pathBackup](const tgui::String &d){
				if (d == this->localize("message_box.button.no"))
					load(filePath);
				else if (load(pathBackup))
					this->_focusedWindow->setPath(filePath);
			});
		}
	});
}

void SpiralOfFate::FrameDataEditor::_save()
{
	this->_focusedWindow->save();
}

void SpiralOfFate::FrameDataEditor::_saveAs()
{
	// TODO: Hardcoded string
	auto file = Utils::saveFileDialog(game->gui, "Save Framedata", "assets/characters");

	// TODO: Hardcoded string
	file->setFileTypeFilters({ {"Framedata file", {"*.json"}}, {"All files", {}} }, 0);
	file->onFileSelect.connect([this](const std::vector<tgui::Filesystem::Path> &arr) {
		this->_focusedWindow->save(arr[0].asString().toStdString());
	});
}

void SpiralOfFate::FrameDataEditor::_settings()
{
	auto window = Utils::openWindowWithFocus<tgui::Gui, SettingsWindow>(game->gui, 0, 0, nullptr, false, std::ref(*this));

	window->setTitle(this->localize("settings.title"));
}

void SpiralOfFate::FrameDataEditor::_editShortcuts()
{
	// TODO: Not implemented
	Utils::dispMsg(game->gui, "Error", "Not implemented", MB_ICONERROR);
}

void SpiralOfFate::FrameDataEditor::_quit()
{
	if (this->closeAll())
		game->screen->close();
}

void SpiralOfFate::FrameDataEditor::_undo()
{
	this->_focusedWindow->undo();
}

void SpiralOfFate::FrameDataEditor::_redo()
{
	this->_focusedWindow->redo();
}

void SpiralOfFate::FrameDataEditor::_newFrame()
{
	this->_focusedWindow->newFrame();
}

void SpiralOfFate::FrameDataEditor::_newEndFrame()
{
	this->_focusedWindow->newEndFrame();
}

void SpiralOfFate::FrameDataEditor::_newAnimationBlock()
{
	this->_focusedWindow->newAnimationBlock();
}

void SpiralOfFate::FrameDataEditor::_newAction()
{
	this->_focusedWindow->newAction();
}

void SpiralOfFate::FrameDataEditor::_newHurtBox()
{
	this->_focusedWindow->newHurtBox();
}

void SpiralOfFate::FrameDataEditor::_newHitBox()
{
	this->_focusedWindow->newHitBox();
}

void SpiralOfFate::FrameDataEditor::_removeFrame()
{
	this->_focusedWindow->removeFrame();
}

void SpiralOfFate::FrameDataEditor::_removeAnimationBlock()
{
	this->_focusedWindow->removeAnimationBlock();
}

void SpiralOfFate::FrameDataEditor::_removeAction()
{
	this->_focusedWindow->removeAction();
}

void SpiralOfFate::FrameDataEditor::_copyBoxesFromLastFrame()
{
	this->_focusedWindow->copyBoxesFromLastFrame();
}

void SpiralOfFate::FrameDataEditor::_copyBoxesFromNextFrame()
{
	this->_focusedWindow->copyBoxesFromNextFrame();
}

void SpiralOfFate::FrameDataEditor::_flattenThisMoveCollisionBoxes()
{
	this->_focusedWindow->flattenThisMoveCollisionBoxes();
}

void SpiralOfFate::FrameDataEditor::_reloadTextures()
{
	this->_focusedWindow->reloadTextures();
}

void SpiralOfFate::FrameDataEditor::setHasRedo(bool hasRedo)
{
	game->gui.get<tgui::MenuBar>("MainBar")->setMenuItemEnabled(
		{ this->localize("menu_item.edit"), this->localizeShortcut("menu_item.edit.redo") },
		hasRedo
	);
}

void SpiralOfFate::FrameDataEditor::setHasUndo(bool hasUndo)
{
	game->gui.get<tgui::MenuBar>("MainBar")->setMenuItemEnabled(
		{ this->localize("menu_item.edit"), this->localizeShortcut("menu_item.edit.undo") },
		hasUndo
	);
}

bool SpiralOfFate::FrameDataEditor::canHandleKeyPress(const sf::Event::KeyPressed &event)
{
	Shortcut s{ .code = event.code, .alt = event.alt, .control = event.control, .shift = event.shift, .meta = event.system };

	game->logger.debug("Pressed keys: " + FrameDataEditor::_shortcutToString(s));
	return this->_shortcuts.contains(s);
}

void SpiralOfFate::FrameDataEditor::keyPressed(const sf::Event::KeyPressed &event)
{
	Shortcut s{ .code = event.code, .alt = event.alt, .control = event.control, .shift = event.shift, .meta = event.system };
	auto it = this->_shortcuts.find(s);

	assert_exp(it != this->_shortcuts.end());
	game->logger.debug("Execute shortcut: " + FrameDataEditor::_shortcutToString(s));
	(this->*(it->second))();
}

const std::map<std::string, std::string> &SpiralOfFate::FrameDataEditor::getLocalizationData() const
{
	return this->_localization;
}

void SpiralOfFate::FrameDataEditor::refreshInterface()
{
	for (auto &win : this->_openWindows)
		win->refreshInterface();
}

bool SpiralOfFate::FrameDataEditor::Shortcut::operator<(const SpiralOfFate::FrameDataEditor::Shortcut &other) const
{
	if (this->meta != other.meta)
		return other.meta;
	if (this->alt != other.alt)
		return other.alt;
	if (this->shift != other.shift)
		return other.shift;
	if (this->control != other.control)
		return other.control;
	return this->code < other.code;
}

//
// Created by PinkySmile on 09/04/25.
//

#include <nlohmann/json.hpp>
#include "FrameDataEditor.hpp"
#include "UI/MainWindow.hpp"
#include "UI/LocalizedContainer.hpp"
#include "UI/SettingsWindow.hpp"
#include "UI/ShortcutsWindow.hpp"

const char *SpiralOfFate::keyNames[] = {
	"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z",
	"num0", "num1", "num2", "num3", "num4", "num5", "num6", "num7", "num8", "num9",
	"escape",
	"lcontrol", "lshift", "lalt", "lsystem",
	"rcontrol", "rshift", "ralt", "rsystem",
	"menu",
	"lbracket", "rbracket",
	"semicolon", "comma", "period", "apostrophe", "slash", "backslash", "grave", "equal", "hyphen",
	"space", "enter",
	"backspace", "tab",
	"pageup", "pagedown",
	"end", "home",
	"insert", "delete",
	"add", "subtract", "multiply", "divide",
	"left", "right", "up", "down",
	"numpad0", "numpad1", "numpad2", "numpad3", "numpad4", "numpad5", "numpad6", "numpad7", "numpad8", "numpad9",
	"f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", "f9", "f10", "f11", "f12", "f13", "f14", "f15",
	"pause"
};

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
	this->_shortcutsNames["menu_item.file.load"]                = { .code = sf::Keyboard::Key::O,      .alt = false, .control = true,  .shift = false, .meta = false };
	this->_shortcutsNames["menu_item.file.save"]                = { .code = sf::Keyboard::Key::S,      .alt = false, .control = true,  .shift = false, .meta = false };
	this->_shortcutsNames["menu_item.file.save_as"]             = { .code = sf::Keyboard::Key::S,      .alt = false, .control = true,  .shift = true,  .meta = false };
	// this->_shortcutsNames["menu_item.file.settings"]      = { .code = sf::Keyboard::Key::LAlt, .alt = false, .control = false, .shift = false, .meta = false };
	// this->_shortcutsNames["menu_item.file.shortcuts"]     = { .code = sf::Keyboard::Key::LAlt, .alt = false, .control = false, .shift = false, .meta = false };
	this->_shortcutsNames["menu_item.file.quit"]                = { .code = sf::Keyboard::Key::Q,      .alt = false, .control = true,  .shift = false, .meta = false };
	this->_shortcutsNames["menu_item.edit.undo"]                = { .code = sf::Keyboard::Key::Z,      .alt = false, .control = true,  .shift = false, .meta = false };
	this->_shortcutsNames["menu_item.edit.redo"]                = { .code = sf::Keyboard::Key::Y,      .alt = false, .control = true,  .shift = false, .meta = false };
	this->_shortcutsNames["menu_item.edit.copy"]                = { .code = sf::Keyboard::Key::C,      .alt = false, .control = true,  .shift = false, .meta = false };
	this->_shortcutsNames["menu_item.edit.paste"]               = { .code = sf::Keyboard::Key::V,      .alt = false, .control = true,  .shift = false, .meta = false };
	this->_shortcutsNames["menu_item.edit.paste_boxdata"]       = { .code = sf::Keyboard::Key::V,      .alt = false, .control = true,  .shift = true,  .meta = false };
	this->_shortcutsNames["menu_item.edit.paste_animdata"]      = { .code = sf::Keyboard::Key::V,      .alt = true,  .control = true,  .shift = false, .meta = false };

	this->_shortcutsNames["menu_item.navigate.next_frame"]      = { .code = sf::Keyboard::Key::Right,  .alt = false, .control = false, .shift = false, .meta = false };
	this->_shortcutsNames["menu_item.navigate.previous_frame"]  = { .code = sf::Keyboard::Key::Left,   .alt = false, .control = false, .shift = false, .meta = false };
	this->_shortcutsNames["menu_item.navigate.next_block"]      = { .code = sf::Keyboard::Key::Right,  .alt = false, .control = false, .shift = true,  .meta = false };
	this->_shortcutsNames["menu_item.navigate.previous_block"]  = { .code = sf::Keyboard::Key::Left,   .alt = false, .control = false, .shift = true,  .meta = false };
	this->_shortcutsNames["menu_item.navigate.next_action"]     = { .code = sf::Keyboard::Key::Right,  .alt = false, .control = true,  .shift = false, .meta = false };
	this->_shortcutsNames["menu_item.navigate.previous_action"] = { .code = sf::Keyboard::Key::Left,   .alt = false, .control = true,  .shift = false, .meta = false };

	this->_shortcutsNames["menu_item.new.frame"]                = { .code = sf::Keyboard::Key::F,      .alt = false, .control = true,  .shift = false, .meta = false };
	this->_shortcutsNames["menu_item.new.frame_end"]            = { .code = sf::Keyboard::Key::F,      .alt = false, .control = true,  .shift = true,  .meta = false };
	this->_shortcutsNames["menu_item.new.block"]                = { .code = sf::Keyboard::Key::B,      .alt = false, .control = true,  .shift = false, .meta = false };
	this->_shortcutsNames["menu_item.new.action"]               = { .code = sf::Keyboard::Key::A,      .alt = false, .control = true,  .shift = false, .meta = false };
	this->_shortcutsNames["menu_item.new.hurt_box"]             = { .code = sf::Keyboard::Key::H,      .alt = false, .control = true,  .shift = false, .meta = false };
	this->_shortcutsNames["menu_item.new.hit_box"]              = { .code = sf::Keyboard::Key::H,      .alt = false, .control = true,  .shift = true,  .meta = false };
	this->_shortcutsNames["menu_item.remove.frame"]             = { .code = sf::Keyboard::Key::Delete, .alt = false, .control = true,  .shift = true,  .meta = false };
	this->_shortcutsNames["menu_item.remove.block"]             = { .code = sf::Keyboard::Key::Delete, .alt = false, .control = true,  .shift = false, .meta = false };
	this->_shortcutsNames["menu_item.remove.action"]            = { .code = sf::Keyboard::Key::Delete, .alt = false, .control = false, .shift = true,  .meta = true  };
	this->_shortcutsNames["menu_item.remove.box"]               = { .code = sf::Keyboard::Key::Delete, .alt = false, .control = false, .shift = false, .meta = false };
	this->_shortcutsNames["menu_item.misc.copy_box_last"]       = { .code = sf::Keyboard::Key::I,      .alt = false, .control = true,  .shift = false, .meta = false };
	this->_shortcutsNames["menu_item.misc.copy_box_next"]       = { .code = sf::Keyboard::Key::I,      .alt = false, .control = true,  .shift = true,  .meta = false };
	this->_shortcutsNames["menu_item.misc.flatten"]             = { .code = sf::Keyboard::Key::K,      .alt = false, .control = true,  .shift = false, .meta = false };
	this->_shortcutsNames["menu_item.misc.reload"]              = { .code = sf::Keyboard::Key::R,      .alt = false, .control = true,  .shift = true,  .meta = false };
	this->_loadSettings();

	auto menu = game->gui.get<tgui::MenuBar>("MainBar");

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
		if (json.contains("shortcuts") && json["shortcuts"].is_object()) {
			this->_shortcutsNames.clear();
			for (const auto &[name, shortcut] : json["shortcuts"].items())
				from_json(shortcut, this->_shortcutsNames[name]);
		}
	} else if (errno != ENOENT)
		throw std::runtime_error("Cannot open settings file: editorSettings.json: " + std::string(strerror(errno)));
}

void SpiralOfFate::FrameDataEditor::saveSettings()
{
	std::ofstream stream{"editorSettings.json"};
	nlohmann::json json = {
		{ "locale", this->_locale },
		{ "shortcuts", nlohmann::json::object() }
	};

	for (const auto &[name, shortcut] : this->_shortcutsNames)
		to_json(json["shortcuts"][name], shortcut);

	if (!stream)
		throw std::runtime_error("Cannot open settings file: editorSettings.json: " + std::string(strerror(errno)));
	stream << json.dump(4);
}

void SpiralOfFate::FrameDataEditor::setShortcuts(const std::map<std::string, Shortcut> &shortcuts)
{
	this->_shortcutsNames = shortcuts;
	this->_buildMenu();
}

std::map<std::string, SpiralOfFate::FrameDataEditor::Shortcut> SpiralOfFate::FrameDataEditor::getShortcuts() const
{
	return this->_shortcutsNames;
}

std::vector<std::pair<std::string, std::string>> SpiralOfFate::FrameDataEditor::getShortcutsNames() const
{
	return this->_shortcutData;
}

bool SpiralOfFate::FrameDataEditor::hasLocalization(const std::string &s) const
{
	return this->_localization.contains(s);
}

std::string SpiralOfFate::FrameDataEditor::localize(const std::string &s) const
{
	auto it = this->_localization.find(s);

	if (it != this->_localization.end())
		return it->second;
	game->logger.warn("Missing translation for " + s);
	return s;
}

std::string SpiralOfFate::FrameDataEditor::shortcutToString(const Shortcut &s) const
{
	std::string result;
	auto code = static_cast<int>(s.code);

	if (code < 0 || static_cast<std::size_t>(code) >= std::size(keyNames))
		result = this->localize("key.unknown");
	else
		result = this->localize(std::string("key.") + keyNames[code]);

	if (s.alt && s.code != sf::Keyboard::Key::LAlt && s.code != sf::Keyboard::Key::RAlt)
		result = this->localize("key.alt") + "+" + result;
	if (s.shift && s.code != sf::Keyboard::Key::LShift && s.code != sf::Keyboard::Key::RShift)
		result = this->localize("key.shift") + "+" + result;
	if (s.meta && s.code != sf::Keyboard::Key::LSystem && s.code != sf::Keyboard::Key::RSystem)
		result = this->localize("key.meta") + "+" + result;
	if (s.control && s.code != sf::Keyboard::Key::LControl && s.code != sf::Keyboard::Key::RControl)
		result = this->localize("key.control") + "+" + result;
	return result;
}

std::string SpiralOfFate::FrameDataEditor::localizeShortcut(const std::string &s) const
{
	std::string name = this->localize(s);
	auto it = this->_shortcutsNames.find(s);

	if (it != this->_shortcutsNames.end()) {
		std::string sname = this->shortcutToString(it->second);

		name += " (";
		name += sname;
		name.push_back(')');
	}
	return name;
}

void SpiralOfFate::FrameDataEditor::_connectShortcut(const tgui::MenuBar::Ptr &menu, const std::vector<std::string> &hierarchy, void (FrameDataEditor::*callback)())
{
	std::vector<tgui::String> tguiHierarchy;
	std::string tmp;

	tguiHierarchy.reserve(hierarchy.size());
	for (size_t i = 0; i < hierarchy.size() - 1; i++) {
		auto l = this->localize(hierarchy[i]);

		tguiHierarchy.emplace_back(l);
		if (!tmp.empty())
			tmp += ", ";
		tmp += l;
	}

	auto it = this->_shortcutsNames.find(hierarchy.back());

	if (hierarchy.size() >= 2)
		tmp += " -> ";
	tmp += this->localize(hierarchy.back());
	this->_shortcutData.emplace_back(hierarchy.back(), tmp);
	tguiHierarchy.emplace_back(this->localizeShortcut(hierarchy.back()));
	if (it != this->_shortcutsNames.end()) {
		game->logger.debug(std::string(hierarchy.back()) + " shortcut is " + this->shortcutToString(it->second));
		this->_shortcuts.emplace(it->second, std::pair{tguiHierarchy, callback});
	}
	menu->connectMenuItem(tguiHierarchy, callback, this);
}

void SpiralOfFate::FrameDataEditor::_placeMenuCallbacks(const tgui::MenuBar::Ptr &menu)
{
	this->_connectShortcut(menu, { "menu_item.file", "menu_item.file.load"      }, &FrameDataEditor::_loadFramedata);
	this->_connectShortcut(menu, { "menu_item.file", "menu_item.file.save"      }, &FrameDataEditor::_save);
	this->_connectShortcut(menu, { "menu_item.file", "menu_item.file.save_as"   }, &FrameDataEditor::_saveAs);
	this->_connectShortcut(menu, { "menu_item.file", "menu_item.file.settings"  }, &FrameDataEditor::_settings);
	this->_connectShortcut(menu, { "menu_item.file", "menu_item.file.shortcuts" }, &FrameDataEditor::_editShortcuts);
	this->_connectShortcut(menu, { "menu_item.file", "menu_item.file.quit"      }, &FrameDataEditor::_quit);

	this->_connectShortcut(menu, { "menu_item.edit", "menu_item.edit.undo"           }, &FrameDataEditor::_undo);
	this->_connectShortcut(menu, { "menu_item.edit", "menu_item.edit.redo"           }, &FrameDataEditor::_redo);
	this->_connectShortcut(menu, { "menu_item.edit", "menu_item.edit.copy"           }, &FrameDataEditor::_copy);
	this->_connectShortcut(menu, { "menu_item.edit", "menu_item.edit.paste"          }, &FrameDataEditor::_paste);
	this->_connectShortcut(menu, { "menu_item.edit", "menu_item.edit.paste_boxdata"  }, &FrameDataEditor::_pasteBoxData);
	this->_connectShortcut(menu, { "menu_item.edit", "menu_item.edit.paste_animdata" }, &FrameDataEditor::_pasteAnimData);

	this->_connectShortcut(menu, { "menu_item.navigate", "menu_item.navigate.next_frame"      }, &FrameDataEditor::_navToNextFrame);
	this->_connectShortcut(menu, { "menu_item.navigate", "menu_item.navigate.previous_frame"  }, &FrameDataEditor::_navToPrevFrame);
	this->_connectShortcut(menu, { "menu_item.navigate", "menu_item.navigate.next_block"      }, &FrameDataEditor::_navToNextBlock);
	this->_connectShortcut(menu, { "menu_item.navigate", "menu_item.navigate.previous_block"  }, &FrameDataEditor::_navToPrevBlock);
	this->_connectShortcut(menu, { "menu_item.navigate", "menu_item.navigate.next_action"     }, &FrameDataEditor::_navToNextAction);
	this->_connectShortcut(menu, { "menu_item.navigate", "menu_item.navigate.previous_action" }, &FrameDataEditor::_navToPrevAction);

	this->_connectShortcut(menu, { "menu_item.new", "menu_item.new.frame"     }, &FrameDataEditor::_newFrame);
	this->_connectShortcut(menu, { "menu_item.new", "menu_item.new.frame_end" }, &FrameDataEditor::_newEndFrame);
	this->_connectShortcut(menu, { "menu_item.new", "menu_item.new.block"     }, &FrameDataEditor::_newAnimationBlock);
	this->_connectShortcut(menu, { "menu_item.new", "menu_item.new.action"    }, &FrameDataEditor::_newAction);
	this->_connectShortcut(menu, { "menu_item.new", "menu_item.new.hurt_box"  }, &FrameDataEditor::_newHurtBox);
	this->_connectShortcut(menu, { "menu_item.new", "menu_item.new.hit_box"   }, &FrameDataEditor::_newHitBox);

	this->_connectShortcut(menu, { "menu_item.remove", "menu_item.remove.frame"  }, &FrameDataEditor::_removeFrame);
	this->_connectShortcut(menu, { "menu_item.remove", "menu_item.remove.block"  }, &FrameDataEditor::_removeAnimationBlock);
	this->_connectShortcut(menu, { "menu_item.remove", "menu_item.remove.action" }, &FrameDataEditor::_removeAction);
	this->_connectShortcut(menu, { "menu_item.remove", "menu_item.remove.box"    }, &FrameDataEditor::_removeBox);

	this->_connectShortcut(menu, { "menu_item.misc", "menu_item.misc.export_image", "menu_item.misc.export_image.this_frame"     }, &FrameDataEditor::_exportThisFrameImage);
	this->_connectShortcut(menu, { "menu_item.misc", "menu_item.misc.export_image", "menu_item.misc.export_image.this_action"    }, &FrameDataEditor::_exportThisActionImage);
	this->_connectShortcut(menu, { "menu_item.misc", "menu_item.misc.export_image", "menu_item.misc.export_image.this_character" }, &FrameDataEditor::_exportThisCharacterImage);

	this->_connectShortcut(menu, { "menu_item.misc", "menu_item.misc.copy_box_last"      }, &FrameDataEditor::_copyBoxesFromLastFrame);
	this->_connectShortcut(menu, { "menu_item.misc", "menu_item.misc.copy_box_next"      }, &FrameDataEditor::_copyBoxesFromNextFrame);
	this->_connectShortcut(menu, { "menu_item.misc", "menu_item.misc.flatten"            }, &FrameDataEditor::_flattenThisMoveCollisionBoxes);
	this->_connectShortcut(menu, { "menu_item.misc", "menu_item.misc.flatten_properties" }, &FrameDataEditor::_flattenThisMoveProperties);
	this->_connectShortcut(menu, { "menu_item.misc", "menu_item.misc.reload"             }, &FrameDataEditor::_reloadTextures);
	this->_connectShortcut(menu, { "menu_item.misc", "menu_item.misc.invert_colors"      }, &FrameDataEditor::_invertColors);
	this->_connectShortcut(menu, { "menu_item.misc", "menu_item.misc.reverse_palette"    }, &FrameDataEditor::_reversePalette);

	this->_connectShortcut(menu, {"menu_item.help", "menu_item.help.about"}, &FrameDataEditor::_about);
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
	this->_shortcutData.clear();
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
		menu->setMenuEnabled(this->localize("menu_item.navigate"), true);
		menu->setMenuEnabled(this->localize("menu_item.misc"), true);
		menu->setMenuEnabled(this->localize("menu_item.remove"), true);
		menu->setMenuItemEnabled({ this->localize("menu_item.file"), this->localizeShortcut("menu_item.file.save") }, true);
		menu->setMenuItemEnabled({ this->localize("menu_item.file"), this->localizeShortcut("menu_item.file.save_as") }, true);
		menu->setMenuItemEnabled({ this->localize("menu_item.edit"), this->localizeShortcut("menu_item.edit.undo") }, this->_focusedWindow->hasUndoData());
		menu->setMenuItemEnabled({ this->localize("menu_item.edit"), this->localizeShortcut("menu_item.edit.redo") }, this->_focusedWindow->hasRedoData());
	} else {
		menu->setMenuEnabled(this->localize("menu_item.new"), false);
		menu->setMenuEnabled(this->localize("menu_item.edit"), false);
		menu->setMenuEnabled(this->localize("menu_item.navigate"), false);
		menu->setMenuEnabled(this->localize("menu_item.misc"), false);
		menu->setMenuEnabled(this->localize("menu_item.remove"), false);
		menu->setMenuItemEnabled({ this->localize("menu_item.file"), this->localizeShortcut("menu_item.file.save") }, false);
		menu->setMenuItemEnabled({ this->localize("menu_item.file"), this->localizeShortcut("menu_item.file.save_as") }, false);
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

void SpiralOfFate::FrameDataEditor::_loadFramedata()
{
	auto file = Utils::openFileDialog(game->gui, this->localize("message_box.title.open_framedata"), "assets/characters");
	auto load = [this](const std::filesystem::path &path){
		try {
			this->_openWindows.emplace_back(new MainWindow(path, *this));
			this->_focusedWindow = this->_openWindows.back();
			this->_focusedWindow->onFocus.connect([this](const std::weak_ptr<MainWindow> &This){
				auto lock = This.lock();

				if (this->_focusedWindow != lock) {
					if (this->_focusedWindow)
						this->_focusedWindow->setFocused(false);
					this->_focusedWindow = lock;
					this->_updateMenuBar();
					this->_focusedWindow->refreshMenuItems();
				}
			}, std::weak_ptr(this->_focusedWindow));
			this->_focusedWindow->onRealClose.connect([this](const std::weak_ptr<MainWindow> &This){
				this->_openWindows.erase(std::remove(this->_openWindows.begin(), this->_openWindows.end(), This.lock()), this->_openWindows.end());
				if (this->_focusedWindow == This.lock())
					this->_focusedWindow = nullptr;
				this->_updateMenuBar();
			}, std::weak_ptr(this->_focusedWindow));
			game->gui.add(this->_focusedWindow);
			this->_focusedWindow->setFocused(true);
			this->_updateMenuBar();
			this->_focusedWindow->refreshMenuItems();
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

	file->setFileTypeFilters({
		{this->localize("file_type.framedata"), {"*.json"}},
		{this->localize("file_type.all"), {}}
	}, 0);
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
	auto file = Utils::saveFileDialog(game->gui, this->localize("message_box.title.save_framedata"), "assets/characters");

	file->setFileTypeFilters({
		{this->localize("file_type.framedata"), {"*.json"}},
		{this->localize("file_type.all"), {}}
	}, 0);
	file->onFileSelect.connect([this](const std::vector<tgui::Filesystem::Path> &arr) {
		this->_focusedWindow->save(arr[0].asString().toStdString());
	});
}

void SpiralOfFate::FrameDataEditor::_settings()
{
	auto window = Utils::openWindowWithFocus<tgui::Gui, SettingsWindow>(
		game->gui,
		0, 0,
		nullptr,
		false,
		std::ref(*this)
	);

	window->setTitle(this->localize("settings.title"));
}

void SpiralOfFate::FrameDataEditor::_editShortcuts()
{
	this->_shortcutWindow = Utils::openWindowWithFocus<tgui::Gui, ShortcutsWindow>(
		game->gui,
		"600", "&.h - 300",
		nullptr,
		false,
		std::ref(*this)
	);
	this->_shortcutWindow->setTitle(this->localize("settings.shortcuts"));
	this->_shortcutWindow->onClose.connect([this] {
		this->_shortcutWindow.reset();
	});
}

void SpiralOfFate::FrameDataEditor::_quit()
{
	if (this->closeAll())
		game->screen->close();
}

void SpiralOfFate::FrameDataEditor::_navToNextFrame()
{
	this->_focusedWindow->navToNextFrame();
}

void SpiralOfFate::FrameDataEditor::_navToPrevFrame()
{
	this->_focusedWindow->navToPrevFrame();
}

void SpiralOfFate::FrameDataEditor::_navToNextBlock()
{
	this->_focusedWindow->navToNextBlock();
}

void SpiralOfFate::FrameDataEditor::_navToPrevBlock()
{
	this->_focusedWindow->navToPrevBlock();
}

void SpiralOfFate::FrameDataEditor::_navToNextAction()
{
	this->_focusedWindow->navToNextAction();
}

void SpiralOfFate::FrameDataEditor::_navToPrevAction()
{
	this->_focusedWindow->navToPrevAction();
}

void SpiralOfFate::FrameDataEditor::_undo()
{
	this->_focusedWindow->undo();
}

void SpiralOfFate::FrameDataEditor::_redo()
{
	this->_focusedWindow->redo();
}

void SpiralOfFate::FrameDataEditor::_copy()
{
	this->_focusedWindow->copyFrame();
}

void SpiralOfFate::FrameDataEditor::_paste()
{
	this->_focusedWindow->pasteFrame();
}

void SpiralOfFate::FrameDataEditor::_pasteBoxData()
{
	this->_focusedWindow->pasteBoxData();
}

void SpiralOfFate::FrameDataEditor::_pasteAnimData()
{
	this->_focusedWindow->pasteAnimData();
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

void SpiralOfFate::FrameDataEditor::_removeBox()
{
	this->_focusedWindow->removeBox();
}

void SpiralOfFate::FrameDataEditor::_exportThisFrameImage()
{
	// TODO: Not implemented
	Utils::dispMsg(game->gui, "Error", "Not implemented", MB_ICONERROR);
}

void SpiralOfFate::FrameDataEditor::_exportThisActionImage()
{
	// TODO: Not implemented
	Utils::dispMsg(game->gui, "Error", "Not implemented", MB_ICONERROR);
}

void SpiralOfFate::FrameDataEditor::_exportThisCharacterImage()
{
	// TODO: Not implemented
	Utils::dispMsg(game->gui, "Error", "Not implemented", MB_ICONERROR);
}

void SpiralOfFate::FrameDataEditor::_flattenThisMoveProperties()
{
	// TODO: Not implemented
	Utils::dispMsg(game->gui, "Error", "Not implemented", MB_ICONERROR);
}

void SpiralOfFate::FrameDataEditor::_invertColors()
{
	// TODO: Not implemented
	Utils::dispMsg(game->gui, "Error", "Not implemented", MB_ICONERROR);
	this->_focusedWindow->invertColors();
}

void SpiralOfFate::FrameDataEditor::_reversePalette()
{
	// TODO: Not implemented
	Utils::dispMsg(game->gui, "Error", "Not implemented", MB_ICONERROR);
	this->_focusedWindow->reversePalette();
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

void SpiralOfFate::FrameDataEditor::_about()
{
	auto window = Utils::openWindowWithFocus(game->gui, 470, 150);

	window->loadWidgetsFromFile("assets/gui/editor/about.gui");
	window->setTitle("About FrameDataEditor");
	window->get<tgui::Label>("Version")->setText(VERSION_STR);
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

void SpiralOfFate::FrameDataEditor::setCanDelBoxes(bool canDel)
{
	game->gui.get<tgui::MenuBar>("MainBar")->setMenuItemEnabled(
		{ this->localize("menu_item.remove"), this->localizeShortcut("menu_item.remove.box") },
		canDel
	);
}

void SpiralOfFate::FrameDataEditor::setCanDelFrame(bool canDel)
{
	game->gui.get<tgui::MenuBar>("MainBar")->setMenuItemEnabled(
		{ this->localize("menu_item.remove"), this->localizeShortcut("menu_item.remove.frame") },
		canDel
	);
}

void SpiralOfFate::FrameDataEditor::setCanDelBlock(bool canDel)
{
	game->gui.get<tgui::MenuBar>("MainBar")->setMenuItemEnabled(
		{ this->localize("menu_item.remove"), this->localizeShortcut("menu_item.remove.block") },
		canDel
	);
}

void SpiralOfFate::FrameDataEditor::setCanDelAction(bool canDel)
{
	game->gui.get<tgui::MenuBar>("MainBar")->setMenuItemEnabled(
		{ this->localize("menu_item.remove"), this->localizeShortcut("menu_item.remove.action") },
		canDel
	);
}

void SpiralOfFate::FrameDataEditor::setCanCopyLast(bool canCopy)
{
	game->gui.get<tgui::MenuBar>("MainBar")->setMenuItemEnabled(
		{ this->localize("menu_item.misc"), this->localizeShortcut("menu_item.misc.copy_box_last") },
		canCopy
	);
}

void SpiralOfFate::FrameDataEditor::setCanCopyNext(bool canCopy)
{
	game->gui.get<tgui::MenuBar>("MainBar")->setMenuItemEnabled(
		{ this->localize("menu_item.misc"), this->localizeShortcut("menu_item.misc.copy_box_next") },
		canCopy
	);
}

void SpiralOfFate::FrameDataEditor::setHasLastFrame(bool hasIt)
{
	game->gui.get<tgui::MenuBar>("MainBar")->setMenuItemEnabled(
		{ this->localize("menu_item.navigate"), this->localizeShortcut("menu_item.navigate.previous_frame") },
		hasIt
	);
}

void SpiralOfFate::FrameDataEditor::setHasNextFrame(bool hasIt)
{
	game->gui.get<tgui::MenuBar>("MainBar")->setMenuItemEnabled(
		{ this->localize("menu_item.navigate"), this->localizeShortcut("menu_item.navigate.next_frame") },
		hasIt
	);
}

void SpiralOfFate::FrameDataEditor::setHasLastBlock(bool hasIt)
{
	game->gui.get<tgui::MenuBar>("MainBar")->setMenuItemEnabled(
		{ this->localize("menu_item.navigate"), this->localizeShortcut("menu_item.navigate.previous_block") },
		hasIt
	);
}

void SpiralOfFate::FrameDataEditor::setHasNextBlock(bool hasIt)
{
	game->gui.get<tgui::MenuBar>("MainBar")->setMenuItemEnabled(
		{ this->localize("menu_item.navigate"), this->localizeShortcut("menu_item.navigate.next_block") },
		hasIt
	);
}

void SpiralOfFate::FrameDataEditor::setHasLastAction(bool hasIt)
{
	game->gui.get<tgui::MenuBar>("MainBar")->setMenuItemEnabled(
		{ this->localize("menu_item.navigate"), this->localizeShortcut("menu_item.navigate.previous_action") },
		hasIt
	);
}

void SpiralOfFate::FrameDataEditor::setHasNextAction(bool hasIt)
{
	game->gui.get<tgui::MenuBar>("MainBar")->setMenuItemEnabled(
		{ this->localize("menu_item.navigate"), this->localizeShortcut("menu_item.navigate.next_action") },
		hasIt
	);
}

bool SpiralOfFate::FrameDataEditor::canHandleKeyPress(const sf::Event::KeyPressed &event)
{
	if (this->_shortcutWindow)
		return false;
	if (this->_focusedWindow && this->_isEditBoxSelected(*this->_focusedWindow))
		return false;

	Shortcut s{ .code = event.code, .alt = event.alt, .control = event.control, .shift = event.shift, .meta = event.system };

	game->logger.debug("Pressed keys: " + this->shortcutToString(s));
	game->logger.debug("Has it? " + std::string(this->_shortcuts.contains(s) ? "true" : "false"));
	return this->_shortcuts.contains(s);
}

void SpiralOfFate::FrameDataEditor::keyPressed(const sf::Event::KeyPressed &event)
{
	Shortcut s{ .code = event.code, .alt = event.alt, .control = event.control, .shift = event.shift, .meta = event.system };
	auto it = this->_shortcuts.find(s);
	auto menu = game->gui.get<tgui::MenuBar>("MainBar");
	std::string tmp = "Check ";
	std::vector<tgui::String> list;

	assert_exp(it != this->_shortcuts.end());
	for (size_t i = 0; i < it->second.first.size(); i++) {
		list.push_back(it->second.first[i]);
		if (i)
			tmp += "->";
		tmp += "\"";
		tmp += it->second.first[i].toStdString();
		tmp += "\"";

		bool ok = i == 0 ? menu->getMenuEnabled(it->second.first[i]) : menu->getMenuItemEnabled(list);

		game->logger.debug(tmp + ": is " + (ok ? "enabled" : "disabled"));
		if (!ok)
			return;
	}
	game->logger.debug("Execute shortcut: " + this->shortcutToString(s));
	(this->*(it->second.second))();
}

void SpiralOfFate::FrameDataEditor::keyReleased(const sf::Event::KeyReleased &event)
{
	if (this->_shortcutWindow)
		this->_shortcutWindow->keyReleased(event);
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

void SpiralOfFate::FrameDataEditor::mouseMovedAbsolute(tgui::Vector2f pos)
{
	if (this->_focusedWindow)
		this->_focusedWindow->mouseMovedAbsolute(pos);
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

bool SpiralOfFate::FrameDataEditor::_isEditBoxSelected(const tgui::Container &container)
{
	return std::ranges::any_of(container.getWidgets(), [this](const auto &widget){
		if (auto cont = widget->template cast<tgui::Container>()) {
			if (this->_isEditBoxSelected(*cont))
				return true;
		} else if (auto box = widget->template cast<tgui::EditBox>()) {
			if (box->isFocused())
				return true;
		}
		return false;
	});
}

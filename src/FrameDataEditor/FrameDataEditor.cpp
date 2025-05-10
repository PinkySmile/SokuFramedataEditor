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

	this->_menuHierarchy = menu->getMenus();
	this->setLocale(this->_locale);
	Utils::setRenderer(game->gui);
}

SpiralOfFate::FrameDataEditor::~FrameDataEditor()
{
	this->saveSettings();
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

void SpiralOfFate::FrameDataEditor::_placeMenuCallbacks(const tgui::MenuBar::Ptr &menu)
{
	menu->connectMenuItem({ this->localize("menu_item.file"), this->localize("menu_item.file.new")      }, &FrameDataEditor::_newFramedata, this);
	menu->connectMenuItem({ this->localize("menu_item.file"), this->localize("menu_item.file.load")     }, &FrameDataEditor::_loadFramedata, this);
	menu->connectMenuItem({ this->localize("menu_item.file"), this->localize("menu_item.file.save")     }, &FrameDataEditor::_save, this);
	menu->connectMenuItem({ this->localize("menu_item.file"), this->localize("menu_item.file.save_as")  }, &FrameDataEditor::_saveAs, this);
	menu->connectMenuItem({ this->localize("menu_item.file"), this->localize("menu_item.file.settings") }, &FrameDataEditor::_settings, this);
	menu->connectMenuItem({ this->localize("menu_item.file"), this->localize("menu_item.file.quit")     }, &FrameDataEditor::_quit, this);

	menu->connectMenuItem({ this->localize("menu_item.edit"), this->localize("menu_item.edit.undo")  }, &FrameDataEditor::_undo, this);
	menu->connectMenuItem({ this->localize("menu_item.edit"), this->localize("menu_item.edit.redo")  }, &FrameDataEditor::_redo, this);

	menu->connectMenuItem({ this->localize("menu_item.new"), this->localize("menu_item.new.frame")     }, &FrameDataEditor::_newFrame, this);
	menu->connectMenuItem({ this->localize("menu_item.new"), this->localize("menu_item.new.frame_end") }, &FrameDataEditor::_newEndFrame, this);
	menu->connectMenuItem({ this->localize("menu_item.new"), this->localize("menu_item.new.block")     }, &FrameDataEditor::_newAnimationBlock, this);
	menu->connectMenuItem({ this->localize("menu_item.new"), this->localize("menu_item.new.hurt_box")  }, &FrameDataEditor::_newHurtBox, this);
	menu->connectMenuItem({ this->localize("menu_item.new"), this->localize("menu_item.new.hit_box")   }, &FrameDataEditor::_newHitBox, this);

	menu->connectMenuItem({ this->localize("menu_item.remove"), this->localize("menu_item.remove.frame")  }, &FrameDataEditor::_removeFrame, this);
	menu->connectMenuItem({ this->localize("menu_item.remove"), this->localize("menu_item.remove.block")  }, &FrameDataEditor::_removeAnimationBlock, this);
	menu->connectMenuItem({ this->localize("menu_item.remove"), this->localize("menu_item.remove.action") }, &FrameDataEditor::_removeAction, this);

	menu->connectMenuItem({ this->localize("menu_item.misc"), this->localize("menu_item.misc.copy_box_last") }, &FrameDataEditor::_copyBoxesFromLastFrame, this);
	menu->connectMenuItem({ this->localize("menu_item.misc"), this->localize("menu_item.misc.copy_box_next") }, &FrameDataEditor::_copyBoxesFromNextFrame, this);
	menu->connectMenuItem({ this->localize("menu_item.misc"), this->localize("menu_item.misc.flatten")       }, &FrameDataEditor::_flattenThisMoveCollisionBoxes, this);
	menu->connectMenuItem({ this->localize("menu_item.misc"), this->localize("menu_item.misc.reload")        }, &FrameDataEditor::_reloadTextures, this);
}

void SpiralOfFate::FrameDataEditor::_addMenu(const tgui::MenuBar::Ptr &menu, const tgui::MenuBar::GetMenusElement &element, std::vector<tgui::String> hierarchy)
{
	hierarchy.emplace_back(this->localize(std::string(element.text)));
	menu->addMenuItem(hierarchy);
	for (auto &d : element.menuItems)
		this->_addMenu(menu, d, hierarchy);
}

void SpiralOfFate::FrameDataEditor::_buildMenu()
{
	auto menu = game->gui.get<tgui::MenuBar>("MainBar");

	game->gui.remove(menu);
	menu = tgui::MenuBar::create();
	game->gui.add(menu, "MainBar");
	game->gui.moveWidgetToBack(menu);
	for (auto &d : this->_menuHierarchy)
		this->_addMenu(menu, d, {});
	this->_placeMenuCallbacks(menu);
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
}

std::string SpiralOfFate::FrameDataEditor::getLocale() const
{
	return this->_locale;
}

void SpiralOfFate::FrameDataEditor::_newFramedata()
{
}

void SpiralOfFate::FrameDataEditor::_loadFramedata()
{
	auto file = Utils::openFileDialog(game->gui, "Open Framedata", "assets/characters");

	file->setFileTypeFilters({ {"Framedata file", {"*.json"}}, {"All files", {}} }, 0);
	file->onFileSelect.connect([this](const std::vector<tgui::Filesystem::Path> &arr) {
		try {
			this->_openWindows.emplace_back(new MainWindow(arr[0].asNativeString(), *this));
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
				}
			}, std::weak_ptr(this->_focusedWindow));
			this->_focusedWindow->onClose.connect([this](const std::weak_ptr<MainWindow> &This){
				this->_openWindows.erase(std::remove(this->_openWindows.begin(), this->_openWindows.end(), This.lock()), this->_openWindows.end());
			}, std::weak_ptr(this->_focusedWindow));
			game->gui.add(this->_focusedWindow);
			this->_focusedWindow->setFocused(true);
		} catch (_AssertionFailedException &e) {
			Utils::dispMsg(game->gui, "Framedata loading failed", "Invalid framedata file: " + std::string(e.what()), MB_ICONERROR);
		} catch (std::exception &e) {
			Utils::dispMsg(game->gui, "Framedata loading failed", "Internal error: " + Utils::getLastExceptionName() + ": " + std::string(e.what()), MB_ICONERROR);
		}
	});
}

void SpiralOfFate::FrameDataEditor::_save()
{
	this->_focusedWindow->save();
}

void SpiralOfFate::FrameDataEditor::_saveAs()
{
	auto file = Utils::saveFileDialog(game->gui, "Save Framedata", "assets/characters");

	file->setFileTypeFilters({ {"Framedata file", {"*.json"}}, {"All files", {}} }, 0);
	file->onFileSelect.connect([this](const std::vector<tgui::Filesystem::Path> &arr) {
		this->_focusedWindow->save(arr[0].asNativeString());
	});
}

void SpiralOfFate::FrameDataEditor::_settings()
{
	auto window = Utils::openWindowWithFocus<SettingsWindow>(game->gui, 0, 0, nullptr, false, std::ref(*this));

	window->setTitle(this->localize("settings.title"));
}

void SpiralOfFate::FrameDataEditor::_quit()
{
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
}

void SpiralOfFate::FrameDataEditor::_newEndFrame()
{
}

void SpiralOfFate::FrameDataEditor::_newAnimationBlock()
{
}

void SpiralOfFate::FrameDataEditor::_newHurtBox()
{
}

void SpiralOfFate::FrameDataEditor::_newHitBox()
{
}

void SpiralOfFate::FrameDataEditor::_removeFrame()
{
}

void SpiralOfFate::FrameDataEditor::_removeAnimationBlock()
{
}

void SpiralOfFate::FrameDataEditor::_removeAction()
{
}

void SpiralOfFate::FrameDataEditor::_copyBoxesFromLastFrame()
{
}

void SpiralOfFate::FrameDataEditor::_copyBoxesFromNextFrame()
{
}

void SpiralOfFate::FrameDataEditor::_flattenThisMoveCollisionBoxes()
{
}

void SpiralOfFate::FrameDataEditor::_reloadTextures()
{
}

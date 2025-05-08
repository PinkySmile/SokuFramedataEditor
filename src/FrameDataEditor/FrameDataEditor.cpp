//
// Created by PinkySmile on 09/04/25.
//

#include <nlohmann/json.hpp>
#include "FrameDataEditor.hpp"
#include "UI/MainWindow.hpp"

SpiralOfFate::FrameDataEditor *editor = nullptr;

SpiralOfFate::FrameDataEditor::FrameDataEditor()
{
	editor = this;
	game->gui.loadWidgetsFromFile("assets/gui/editor/layout.gui");
	this->_clock.stop();
	this->_loadSettings();

	std::ifstream stream{ "assets/gui/editor/locale/" + this->_locale + ".json" };
	auto menu = game->gui.get<tgui::MenuBar>("main_bar");
	nlohmann::json json = nlohmann::json::object();

	Utils::setRenderer(game->gui);
	if (stream)
		stream >> json;
	this->_localization = json.get<std::map<std::string, std::string>>();
	this->_menuHierarchy = menu->getMenus();
	this->_buildMenu(menu);
}

void SpiralOfFate::FrameDataEditor::_loadSettings()
{
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

void SpiralOfFate::FrameDataEditor::_buildMenu(const tgui::MenuBar::Ptr &menu)
{
	menu->removeAllMenus();
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
}

void SpiralOfFate::FrameDataEditor::_saveAs()
{

}

void SpiralOfFate::FrameDataEditor::_settings()
{
}

void SpiralOfFate::FrameDataEditor::_quit()
{
	game->screen->close();
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

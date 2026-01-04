//
// Created by PinkySmile on 09/04/25.
//

#ifndef SOFGV_FRAMEDATAEDITOR_HPP
#define SOFGV_FRAMEDATAEDITOR_HPP


#include <SFML/System.hpp>
#include "EditableObject.hpp"

namespace SpiralOfFate
{
	class MainWindow;

	extern const char *keyNames[101];

	class FrameDataEditor {
	public:
		struct Shortcut {
			sf::Keyboard::Key code;
			bool alt;
			bool control;
			bool shift;
			bool meta;

			bool operator<(const Shortcut &other) const;
		};

	private:
		std::string _locale;

		static void to_json(nlohmann::json &j, const Shortcut &s) {
			j = {
				{ "code",    s.code },
				{ "alt",     s.alt },
				{ "control", s.control },
				{ "shift",   s.shift },
				{ "meta",    s.meta },
			};
		}

		static void from_json(const nlohmann::json &j, Shortcut &s) {
			j.at("code").get_to(s.code);
			j.at("alt").get_to(s.alt);
			j.at("control").get_to(s.control);
			j.at("shift").get_to(s.shift);
			j.at("meta").get_to(s.meta);
		}

		float _timer = 0;
		sf::Clock _clock;
		std::shared_ptr<class ShortcutsWindow> _shortcutWindow;
		std::map<std::string, Shortcut> _shortcutsNames;
		std::vector<std::pair<std::string, std::string>> _shortcutData;
		std::map<Shortcut, std::pair<std::vector<tgui::String>, void (FrameDataEditor::*)()>> _shortcuts;
		std::vector<tgui::MenuBar::GetMenusElement> _menuHierarchy;
		std::map<std::string, std::string> _localization;
		std::vector<std::shared_ptr<MainWindow>> _openWindows;
		std::shared_ptr<MainWindow> _focusedWindow;

		void _connectShortcut(const tgui::MenuBar::Ptr &menu, const std::vector<std::string> &hierarchy, void (FrameDataEditor::*callback)());
		void _placeMenuCallbacks(const tgui::MenuBar::Ptr &menu);
		void _addMenu(const tgui::MenuBar::Ptr &menu, const tgui::MenuBar::GetMenusElement &element, std::vector<tgui::String> hierarchy);
		void _buildMenu();
		void _updateMenuBar();

		void _loadSettings();
		void _tickAnimation();

		void _loadFramedata();
		void _save();
		void _saveAs();
		void _settings();
		void _editShortcuts();
		void _quit();

		void _navToNextFrame();
		void _navToPrevFrame();
		void _navToNextBlock();
		void _navToPrevBlock();
		void _navToNextAction();
		void _navToPrevAction();

		void _undo();
		void _redo();
		void _copy();
		void _paste();
		void _pasteBoxData();
		void _pasteAnimData();

		void _newFrame();
		void _newEndFrame();
		void _newAnimationBlock();
		void _newAction();
		void _newHurtBox();
		void _newHitBox();

		void _removeFrame();
		void _removeAnimationBlock();
		void _removeAction();
		void _removeBox();

		void _exportThisFrameImage();
		void _exportThisActionImage();
		void _exportThisCharacterImage();
		void _copyBoxesFromLastFrame();
		void _copyBoxesFromNextFrame();
		void _flattenThisMoveCollisionBoxes();
		void _flattenThisMoveProperties();
		void _reloadTextures();
		void _invertColors();
		void _reversePalette();

		void _about();

		bool _isEditBoxSelected(const tgui::Container &container);

	public:
		FrameDataEditor();
		~FrameDataEditor();

		std::string shortcutToString(const Shortcut &s) const;
		void mouseMovedAbsolute(tgui::Vector2f pos);
		void setLocale(const std::string &name);
		std::string getLocale() const;
		void saveSettings();
		void setShortcuts(const std::map<std::string, Shortcut> &shortcuts);
		std::map<std::string, Shortcut> getShortcuts() const;
		std::vector<std::pair<std::string, std::string>> getShortcutsNames() const;

		const std::map<std::string, std::string> &getLocalizationData() const;
		bool hasLocalization(const std::string &s) const;
		std::string localizeShortcut(const std::string &s) const;
		std::string localize(const std::string &s) const;
		template<typename ...Args>
		std::string localize(const std::string &s, const Args... args) const
		{
			std::vector<std::string> vec{args...};
			std::string localize = this->localize(s);

			for (size_t i = 0; i < vec.size(); i++) {
				std::string v = "%" + std::to_string(i + 1);
				size_t pos = localize.find(v);

				if (pos != std::string::npos)
					localize.replace(pos, v.size(), vec[i]);
				else
					localize += " " + v;
			}
			return localize;
		}

		void refreshInterface();
		void update();
		void render();
		bool closeAll();

		void setHasRedo(bool hasRedo);
		void setHasUndo(bool hasUndo);
		void setCanDelBoxes(bool canDel);
		void setCanDelFrame(bool canDel);
		void setCanDelBlock(bool canDel);
		void setCanDelAction(bool canDel);
		void setCanCopyLast(bool canCopy);
		void setCanCopyNext(bool canCopy);
		void setHasLastFrame(bool hasIt);
		void setHasNextFrame(bool hasIt);
		void setHasLastBlock(bool hasIt);
		void setHasNextBlock(bool hasIt);
		void setHasLastAction(bool hasIt);
		void setHasNextAction(bool hasIt);

		void keyPressed(const sf::Event::KeyPressed &event);
		void keyReleased(const sf::Event::KeyReleased &event);
		bool canHandleKeyPress(const sf::Event::KeyPressed &event);
	};
}


#endif //SOFGV_FRAMEDATAEDITOR_HPP

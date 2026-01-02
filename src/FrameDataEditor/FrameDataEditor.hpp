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

	class FrameDataEditor {
	private:
		std::string _locale;

		struct Shortcut {
			sf::Keyboard::Key code;
			bool alt;
			bool control;
			bool shift;
			bool meta;

			bool operator<(const Shortcut &other) const;
		};

		float _timer = 0;
		sf::Clock _clock;
		std::map<std::string, Shortcut> _shortcutsNames;
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

		void _copyBoxesFromLastFrame();
		void _copyBoxesFromNextFrame();
		void _flattenThisMoveCollisionBoxes();
		void _reloadTextures();

		bool _isEditBoxSelected(const tgui::Container &container);
		std::string _shortcutToString(const Shortcut &s) const;

	public:
		FrameDataEditor();
		~FrameDataEditor();

		void mouseMovedAbsolute(tgui::Vector2f pos);
		void setLocale(const std::string &name);
		std::string getLocale() const;
		void saveSettings();

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
		bool canHandleKeyPress(const sf::Event::KeyPressed &event);
	};
}


#endif //SOFGV_FRAMEDATAEDITOR_HPP

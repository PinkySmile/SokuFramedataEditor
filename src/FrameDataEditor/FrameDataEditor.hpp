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

		bool _force = false;
		float _timer = 0;
		sf::Clock _clock;
		std::vector<tgui::MenuBar::GetMenusElement> _menuHierarchy;
		std::map<std::string, std::string> _localization;
		std::vector<std::shared_ptr<MainWindow>> _openWindows;
		std::shared_ptr<MainWindow> _focusedWindow;

		void _placeMenuCallbacks(const tgui::MenuBar::Ptr &menu);
		void _addMenu(const tgui::MenuBar::Ptr &menu, const tgui::MenuBar::GetMenusElement &element, std::vector<tgui::String> hierarchy);
		void _buildMenu();

		void _loadSettings();
		void _tickAnimation();

		void _newFramedata();
		void _loadFramedata();
		void _save();
		void _saveAs();
		void _settings();
		void _quit();
		void _newFrame();
		void _newEndFrame();
		void _newAnimationBlock();
		void _newHurtBox();
		void _newHitBox();
		void _removeFrame();
		void _removeAnimationBlock();
		void _removeAction();
		void _copyBoxesFromLastFrame();
		void _copyBoxesFromNextFrame();
		void _flattenThisMoveCollisionBoxes();
		void _reloadTextures();

	public:
		FrameDataEditor();
		~FrameDataEditor();

		void setLocale(const std::string &name);
		std::string getLocale() const;
		void saveSettings();

		std::string localize(const std::string &s) const;
		void update();
		void render();
	};
}


#endif //SOFGV_FRAMEDATAEDITOR_HPP

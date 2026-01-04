//
// Created by PinkySmile on 05/05/25.
//

#ifndef SOFGV_LOCALIZEDCONTAINER_HPP
#define SOFGV_LOCALIZEDCONTAINER_HPP

#include <TGUI/Container.hpp>
#include "../FrameDataEditor.hpp"

namespace SpiralOfFate
{
	template<typename BaseContainer>
	class LocalizedContainer : public BaseContainer {
	protected:
		void _localizeWidgets(tgui::Container &container, bool first)
		{
			// TODO: Create struct LocalizationData and use that instead
			for (auto &w : container.getWidgets()) {
				try { if (!w->getUserData<bool>()) continue; } catch (std::bad_any_cast &) {}
				try { if (w->getUserData<tgui::String>() == "NO_LOCALE") continue; } catch (std::bad_any_cast &) {}
				try { if (w->getUserData<std::string>() == "NO_LOCALE") continue; } catch (std::bad_any_cast &) {}
				try { if (strcmp(w->getUserData<const char *>(), "NO_LOCALE") == 0) continue; } catch (std::bad_any_cast &) {}
				try {
					if (auto label = w->cast<tgui::Label>()) {
						if (first)
							label->setUserData(std::string(label->getText()));
						label->setText(this->_editor.localize(std::string(label->getUserData<std::string>())));
					} else if (auto button = w->cast<tgui::ButtonBase>()) {
						if (first)
							button->setUserData(std::string(button->getText()));
						button->setText(this->_editor.localize(std::string(button->getUserData<std::string>())));
					} else if (auto check = w->cast<tgui::CheckBox>()) {
						if (first)
							check->setUserData(std::string(check->getText()));
						check->setText(this->_editor.localize(std::string(check->getUserData<std::string>())));
					} else if (auto combo = w->cast<tgui::ComboBox>()) {
						for (const auto &item : combo->getItemIds())
							combo->changeItemById(item, this->_editor.localize(item.toStdString()));
					} else if (auto cont = w->cast<tgui::Container>())
						this->_localizeWidgets(*cont, first);
				} catch (std::bad_any_cast &) {}
			}
		}

	protected:
		const FrameDataEditor &_editor;

	public:
		using Ptr = std::shared_ptr<LocalizedContainer>; //!< Shared widget pointer
		using ConstPtr = std::shared_ptr<const LocalizedContainer>; //!< Shared constant widget pointer

		LocalizedContainer(const FrameDataEditor &editor, const char *name = BaseContainer::StaticWidgetType, bool initRenderer = true) :
			BaseContainer(name, initRenderer),
			_editor(editor)
		{
		}

		void loadLocalizedWidgetsFromFile(const tgui::String &filename, bool replaceExisting = true)
		{
			this->loadWidgetsFromFile(filename, replaceExisting);
			this->_localizeWidgets(*this, true);
		}

		void reLocalize()
		{
			this->_localizeWidgets(*this, false);
		}
	};
}


#endif //SOFGV_LOCALIZEDCONTAINER_HPP

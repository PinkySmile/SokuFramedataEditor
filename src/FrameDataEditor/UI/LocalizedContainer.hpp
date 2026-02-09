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
		void _localizeWidgets(const tgui::Container &container, bool first)
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
						label->setText(this->localize(std::string(label->getUserData<std::string>())));
					} else if (auto button = w->cast<tgui::ButtonBase>()) {
						if (first)
							button->setUserData(std::string(button->getText()));
						button->setText(this->localize(std::string(button->getUserData<std::string>())));
					} else if (auto check = w->cast<tgui::CheckBox>()) {
						if (first)
							check->setUserData(std::string(check->getText()));
						check->setText(this->localize(std::string(check->getUserData<std::string>())));
					} else if (auto combo = w->cast<tgui::ComboBox>()) {
						if (first) {
							auto items = combo->getItems();

							combo->removeAllItems();
							for (const auto &item : items)
								combo->addItem(this->localize(item.toStdString()), item);
						} else {
							for (const auto &item : combo->getItemIds())
								combo->changeItemById(item, this->localize(item.toStdString()));
						}
					} else if (auto cont = w->cast<tgui::Container>())
						this->_localizeWidgets(*cont, first);
				} catch (std::bad_any_cast &) {}
			}
		}

	protected:
		const FrameDataEditor &_editor;

	public:
		std::map<std::string, std::string> localizationOverride;

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

		template<typename ...Args>
		std::string localize(const std::string &s, const Args... args) const
		{
			for (auto &[key, value] : this->localizationOverride)
				if (s.starts_with(key))
					return this->_editor.localize(value + s.substr(key.size()), args...);
			return this->_editor.localize(s, args...);
		}

		std::string localize(const std::string &s)
		{
			for (auto &[key, value] : this->localizationOverride)
				if (s.starts_with(key))
					return this->_editor.localize(value + s.substr(key.size()));
			return this->_editor.localize(s);
		}
	};
}


#endif //SOFGV_LOCALIZEDCONTAINER_HPP

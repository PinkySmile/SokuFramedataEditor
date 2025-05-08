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
	private:
		void _localizeWidgets(tgui::Container &container, bool first)
		{
			for (auto &w : container.getWidgets()) {
				if (auto label = w->cast<tgui::Label>()) {
					if (first)
						label->setUserData(std::string(label->getText()));
					label->setText(this->_editor.localize(std::string(label->getUserData<std::string>())));
				} else if (auto button = w->cast<tgui::ButtonBase>()) {
					if (first)
						button->setUserData(std::string(button->getText()));
					button->setText(this->_editor.localize(std::string(button->getUserData<std::string>())));
				} else if (auto combo = w->cast<tgui::ComboBox>()) {
					auto items = combo->getItemIds();

					for (size_t i = 0; i < items.size(); i++)
						combo->changeItemById(items[i], this->_editor.localize(items[i].toStdString()));
				} else if (auto cont = w->cast<tgui::Container>())
					this->_localizeWidgets(*cont, first);
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

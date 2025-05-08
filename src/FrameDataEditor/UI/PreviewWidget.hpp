//
// Created by PinkySmile on 07/05/25.
//

#ifndef SOFGV_PREVIEWWIDGET_HPP
#define SOFGV_PREVIEWWIDGET_HPP


#include <TGUI/Widgets/ClickableWidget.hpp>
#include "../EditableObject.hpp"

namespace SpiralOfFate
{
	class PreviewWidget : public tgui::ClickableWidget {
	private:
		sf::Texture _stageTexture;
		mutable sf::Sprite _stageSprite;
		const EditableObject &_object;

	public:
		typedef std::shared_ptr<PreviewWidget> Ptr; //!< Shared widget pointer
		typedef std::shared_ptr<const PreviewWidget> ConstPtr; //!< Shared constant widget pointer

		PreviewWidget(const EditableObject &object);
		~PreviewWidget() override = default;

		void draw(tgui::BackendRenderTarget &target, tgui::RenderStates states) const override;
	};
}


#endif //SOFGV_PREVIEWWIDGET_HPP

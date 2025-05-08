//
// Created by PinkySmile on 07/05/25.
//

#include "PreviewWidget.hpp"
#include <TGUI/Backend/Renderer/SFML-Graphics/BackendRenderTargetSFML.hpp>

SpiralOfFate::PreviewWidget::PreviewWidget(const EditableObject &object) :
	_stageSprite(this->_stageTexture),
	_object(object)
{
	assert_exp(this->_stageTexture.loadFromFile("assets/stages/editor.png"));
}

void SpiralOfFate::PreviewWidget::draw(tgui::BackendRenderTarget &target, tgui::RenderStates states) const
{
	auto &sfmlTarget = dynamic_cast<tgui::BackendRenderTargetSFML &>(target);
	const std::array<float, 16>& transformMatrix = states.transform.getMatrix();
	sf::RenderStates statesSFML;
	auto realTarget = sfmlTarget.getTarget();

	this->_stageSprite.setPosition({-50, 50 - this->getSize().y});
	this->_stageSprite.setTextureRect(sf::IntRect{
		{0, (int)this->_stageTexture.getSize().y - (int)this->getSize().y - 50},
		{(int)this->getSize().x, (int)this->getSize().y}
	});

	states.transform.translate({50, this->getSize().y - 50});
	statesSFML.transform = sf::Transform(
		transformMatrix[0], transformMatrix[4], transformMatrix[12],
		transformMatrix[1], transformMatrix[5], transformMatrix[13],
		transformMatrix[3], transformMatrix[7], transformMatrix[15]
	);
	statesSFML.coordinateType = sf::CoordinateType::Normalized;
	realTarget->draw(this->_stageSprite, statesSFML);
	this->_object.render(*realTarget, statesSFML);
}

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

void SpiralOfFate::PreviewWidget::_drawBox(const SpiralOfFate::Rectangle &box, const SpiralOfFate::Color &color, sf::RenderStates &states) const
{
	sf::VertexArray arr{ sf::PrimitiveType::TriangleFan, 4 };
	sf::VertexArray arr2{ sf::PrimitiveType::LineStrip, 5 };

	for (int i = 0; i < 4; i++) {
		arr[i].color = color;
		arr[i].color.a *= 0x60 / 255.f;
		arr[i].position = (&box.pt1)[i];
	}
	game->screen->draw(arr, states);

	for (unsigned i = 0; i < 5; i++) {
		arr2[i].color = color;
		arr2[i].position = (&box.pt1)[i % 4];
	}
	game->screen->draw(arr2, states);
}

void SpiralOfFate::PreviewWidget::draw(tgui::BackendRenderTarget &target, tgui::RenderStates states) const
{
	auto &sfmlTarget = dynamic_cast<tgui::BackendRenderTargetSFML &>(target);
	const std::array<float, 16>& transformMatrix = states.transform.getMatrix();
	sf::RenderStates statesSFML;
	auto realTarget = sfmlTarget.getTarget();

	this->_stageSprite.setPosition({-150, 50 - this->getSize().y});
	this->_stageSprite.setTextureRect(sf::IntRect{
		{-100, (int)this->_stageTexture.getSize().y - (int)this->getSize().y - 50},
		{(int)this->getSize().x + 100, (int)this->getSize().y + 100}
	});

	states.transform.translate({100, this->getSize().y - 150});
	statesSFML.transform = sf::Transform(
		transformMatrix[0], transformMatrix[4], transformMatrix[12],
		transformMatrix[1], transformMatrix[5], transformMatrix[13],
		transformMatrix[3], transformMatrix[7], transformMatrix[15]
	);
	statesSFML.coordinateType = sf::CoordinateType::Normalized;
	realTarget->draw(this->_stageSprite, statesSFML);
	this->_object.render(*realTarget, statesSFML);

	for (auto &hurtBox : this->_object._getModifiedHurtBoxes())
		this->_drawBox(hurtBox, Color::Green, statesSFML);
	for (auto &hitBox : this->_object._getModifiedHitBoxes())
		this->_drawBox(hitBox, Color::Red, statesSFML);

	auto &data = this->_object.getFrameData();

	if (data.collisionBox) {
		auto &box = *data.collisionBox;
		Vector2f realPos = {
			this->_object._position.x,
			-this->_object._position.y
		};

		this->_drawBox({
			realPos + box.pos,
			realPos + Vector2f{
				static_cast<float>(box.pos.x),
				static_cast<float>(box.pos.y) + box.size.y
			},
			realPos + box.pos + box.size,
			realPos + Vector2f{
				static_cast<float>(box.pos.x) + box.size.x,
				static_cast<float>(box.pos.y)
			}
		}, Color::Yellow, statesSFML);
	}

;}

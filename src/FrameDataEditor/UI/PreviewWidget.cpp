//
// Created by PinkySmile on 07/05/25.
//

#include "PreviewWidget.hpp"
#include <TGUI/Backend/Renderer/SFML-Graphics/BackendRenderTargetSFML.hpp>

#define HOVER_ALPHA 0x80
#define NORMAL_ALPHA 0x40

SpiralOfFate::PreviewWidget::PreviewWidget(const EditableObject &object) :
	_stageSprite(this->_stageTexture),
	_object(object)
{
	assert_exp(this->_stageTexture.loadFromFile("assets/stages/editor.png"));
}

void SpiralOfFate::PreviewWidget::_drawBox(const SpiralOfFate::Rectangle &box, const SpiralOfFate::Color &color, sf::RenderStates &states, bool hovered, bool selected, bool rotate) const
{
	sf::VertexArray arr{ sf::PrimitiveType::TriangleFan, 4 };
	sf::VertexArray arr2{ sf::PrimitiveType::LineStrip, 5 };

	for (int i = 0; i < 4; i++) {
		arr[i].color = color;
		if (hovered || selected)
			arr[i].color.a *= HOVER_ALPHA / 255.f;
		else
			arr[i].color.a *= NORMAL_ALPHA / 255.f;
		arr[i].position = (&box.pt1)[i];
	}
	game->screen->draw(arr, states);

	for (unsigned i = 0; i < 5; i++) {
		arr2[i].color = color;
		arr2[i].position = (&box.pt1)[i % 4];
	}
	game->screen->draw(arr2, states);

	if (!selected)
		return;

	auto rotation = this->displaceObject ? this->_object._rotation : 0;
	sf::RectangleShape rect;

	rect.setFillColor(sf::Color{0x80, 0x80, 0x80});
	rect.setOutlineColor(sf::Color::Red);
	rect.setOutlineThickness(1);
	rect.setSize({8, 8});
	rect.setOrigin({4, 4});
	if (rotate)
		rect.setRotation(sf::radians(rotation));

	rect.setPosition(box.pt1);
	game->screen->draw(rect, states);
	rect.setPosition(box.pt2);
	game->screen->draw(rect, states);
	rect.setPosition(box.pt3);
	game->screen->draw(rect, states);
	rect.setPosition(box.pt4);
	game->screen->draw(rect, states);
	rect.setPosition(box.pt1 + (box.pt2 - box.pt1) / 2);
	game->screen->draw(rect, states);
	rect.setPosition(box.pt2 + (box.pt3 - box.pt2) / 2);
	game->screen->draw(rect, states);
	rect.setPosition(box.pt3 + (box.pt4 - box.pt3) / 2);
	game->screen->draw(rect, states);
	rect.setPosition(box.pt4 + (box.pt1 - box.pt4) / 2);
	game->screen->draw(rect, states);
}

void SpiralOfFate::PreviewWidget::draw(tgui::BackendRenderTarget &target, tgui::RenderStates states) const
{
	auto &sfmlTarget = dynamic_cast<tgui::BackendRenderTargetSFML &>(target);
	const std::array<float, 16> &transformMatrix = states.transform.getMatrix();
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
	this->_object.render(*realTarget, statesSFML, this->displaceObject);

	if (this->displayBoxes) {
		size_t index = 1;

		for (auto &hurtBox : this->_object._getModifiedHurtBoxes(this->displaceObject)) {
			this->_drawBox(hurtBox, Color::Green, statesSFML, this->_boxHovered == index, this->_boxSelected == index, true);
			index++;
		}
		for (auto &hitBox : this->_object._getModifiedHitBoxes(this->displaceObject)) {
			this->_drawBox(hitBox, Color::Red, statesSFML, this->_boxHovered == index, this->_boxSelected == index, true);
			index++;
		}

		auto &data = this->_object.getFrameData();

		if (data.collisionBox) {
			auto &box = *data.collisionBox;
			Vector2f realPos = this->displaceObject ? Vector2f{
				this->_object._position.x,
				-this->_object._position.y
			} : Vector2f{0, 0};

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
			}, Color::Yellow, statesSFML, this->_boxHovered == index, this->_boxSelected == index, false);
		}
	}
}

void SpiralOfFate::PreviewWidget::_updateHover(const tgui::Vector2f &pos)
{
	size_t index = 1;
	std::vector<size_t> old;
	size_t highest = 0;

	this->_hoveredBoxes.swap(old);
	for (auto &hurtBox : this->_object._getModifiedHurtBoxes(this->displaceObject)) {
		if (hurtBox.contains(pos))
			this->_hoveredBoxes.push_back(index);
		index++;
	}
	for (auto &hitBox : this->_object._getModifiedHitBoxes(this->displaceObject)) {
		if (hitBox.contains(pos))
			this->_hoveredBoxes.push_back(index);
		index++;
	}

	auto &data = this->_object.getFrameData();

	if (data.collisionBox) {
		auto &box = *data.collisionBox;
		Vector2f realPos = this->displaceObject ? Vector2f{
			this->_object._position.x,
			-this->_object._position.y
		} : Vector2f{0, 0};
		Rectangle rect{
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
		};

		if (rect.contains(pos))
			this->_hoveredBoxes.push_back(index);
	}
	if (this->_hoveredBoxes.empty())
		this->_boxHovered = 0;
	else if (highest < this->_hoveredBoxes.back()) {
		this->_boxCounter = this->_hoveredBoxes.size() - 1;
		this->_boxHovered = this->_hoveredBoxes.back();
	} else if (this->_boxHovered != 0) {
		auto it = std::find(this->_hoveredBoxes.begin(), this->_hoveredBoxes.end(), this->_boxHovered);

		if (it == this->_hoveredBoxes.end())
			this->_boxCounter = this->_hoveredBoxes.size() - 1;
		else
			this->_boxCounter = it - this->_hoveredBoxes.begin();
		this->_boxHovered = this->_hoveredBoxes[this->_boxCounter];
	}
}

void SpiralOfFate::PreviewWidget::mouseMoved(tgui::Vector2f pos)
{
	auto translatedPos = pos;

	translatedPos.x -= 100;
	translatedPos.y -= this->getSize().y - 150;
	if (this->isMouseDown() && this->_dragStarted) {

	} else
		this->_updateHover(translatedPos);
	Widget::mouseMoved(pos);
}

bool SpiralOfFate::PreviewWidget::scrolled(float delta, tgui::Vector2f pos, bool touch)
{
	if (this->_hoveredBoxes.empty())
		this->_boxHovered = 0;
	else if (delta < 0) {
		this->_boxCounter++;
		if (this->_boxCounter == this->_hoveredBoxes.size())
			this->_boxCounter = 0;
		this->_boxHovered = this->_hoveredBoxes[this->_boxCounter];
	} else {
		if (this->_boxCounter != 0)
			this->_boxCounter--;
		else
			this->_boxCounter = this->_hoveredBoxes.size() - 1;
		this->_boxHovered = this->_hoveredBoxes[this->_boxCounter];
	}
	return Widget::scrolled(delta, pos, touch);
}

void SpiralOfFate::PreviewWidget::frameChanged()
{
	this->_boxCounter = 0;
	this->_boxHovered = 0;
	this->_boxSelected = 0;
}

bool SpiralOfFate::PreviewWidget::leftMousePressed(tgui::Vector2f pos)
{
	if (this->_boxHovered == 0) {
		auto translatedPos = pos;

		translatedPos.x -= 100;
		translatedPos.y -= this->getSize().y - 150;
		this->_updateHover(translatedPos);
	}
	this->_boxSelected = this->_boxHovered;
	return ClickableWidget::leftMousePressed(pos);
}

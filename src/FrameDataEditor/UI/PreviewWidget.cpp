//
// Created by PinkySmile on 07/05/25.
//

#include <TGUI/Backend/Renderer/SFML-Graphics/BackendRenderTargetSFML.hpp>
#include "MainWindow.hpp"
#include "PreviewWidget.hpp"
#include "../Operations/DummyOperation.hpp"
#include "../Operations/BoxModificationOperation.hpp"
#include "../Operations/RemoveBoxOperation.hpp"

#define HOVER_ALPHA 0x80
#define NORMAL_ALPHA 0x40
#define DRAG_CORNER_SIZE 8
#define BOX_SIZE_MIN 20

SpiralOfFate::PreviewWidget::PreviewWidget(const FrameDataEditor &editor, MainWindow &main, EditableObject &object) :
	_stageSprite(this->_stageTexture),
	_object(object),
	_editor(editor),
	_main(main)
{
	assert_exp(this->_stageTexture.loadFromFile("assets/stages/editor.png"));
}

std::pair<SpiralOfFate::BoxType, unsigned int> SpiralOfFate::PreviewWidget::getSelectedBox()
{
	if (this->_boxSelected == 0)
		return {BOXTYPE_NONE, 0};

	auto &data = this->_object.getFrameData();

	if (this->_boxSelected <= data.hurtBoxes.size())
		return {BOXTYPE_HURTBOX, this->_boxSelected - 1};
	if (this->_boxSelected <= data.hurtBoxes.size() + data.hitBoxes.size())
		return {BOXTYPE_HITBOX, this->_boxSelected - 1 - data.hurtBoxes.size()};
	return {BOXTYPE_COLLISIONBOX, 0};
}

SpiralOfFate::Box *SpiralOfFate::PreviewWidget::getSelectedBoxRef()
{
	if (this->_boxSelected == 0)
		return nullptr;

	auto &data = this->_object.getFrameData();

	if (this->_boxSelected <= data.hurtBoxes.size())
		return &data.hurtBoxes[this->_boxSelected - 1];
	if (this->_boxSelected <= data.hurtBoxes.size() + data.hitBoxes.size())
		return &data.hitBoxes[this->_boxSelected - 1 - data.hurtBoxes.size()];
	return data.collisionBox;
}

void SpiralOfFate::PreviewWidget::_drawBox(const SpiralOfFate::Rectangle &box, const SpiralOfFate::Color &color, sf::RenderStates &states, bool hovered, bool selected) const
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
}

void SpiralOfFate::PreviewWidget::_drawBoxBorder(const SpiralOfFate::Rectangle &box, sf::RenderStates &states, bool rotate) const
{
	sf::RectangleShape rect;

	rect.setOutlineColor(sf::Color::Red);
	rect.setOutlineThickness(1);
	rect.setSize({DRAG_CORNER_SIZE, DRAG_CORNER_SIZE});
	rect.setOrigin({DRAG_CORNER_SIZE / 2.f, DRAG_CORNER_SIZE / 2.f});
	if (rotate && this->displaceObject)
		rect.setRotation(sf::radians(this->_object._rotation));

	rect.setFillColor(this->_cornerSelected == 1 || this->_cornerHovered == 1 ? sf::Color{0xA0, 0xA0, 0xA0} : sf::Color{0x80, 0x80, 0x80});
	rect.setPosition(box.pt1);
	game->screen->draw(rect, states);
	rect.setFillColor(this->_cornerSelected == 2 || this->_cornerHovered == 2 ? sf::Color{0xA0, 0xA0, 0xA0} : sf::Color{0x80, 0x80, 0x80});
	rect.setPosition(box.pt2);
	game->screen->draw(rect, states);
	rect.setFillColor(this->_cornerSelected == 3 || this->_cornerHovered == 3 ? sf::Color{0xA0, 0xA0, 0xA0} : sf::Color{0x80, 0x80, 0x80});
	rect.setPosition(box.pt3);
	game->screen->draw(rect, states);
	rect.setFillColor(this->_cornerSelected == 4 || this->_cornerHovered == 4 ? sf::Color{0xA0, 0xA0, 0xA0} : sf::Color{0x80, 0x80, 0x80});
	rect.setPosition(box.pt4);
	game->screen->draw(rect, states);
	rect.setFillColor(this->_cornerSelected == 5 || this->_cornerHovered == 5 ? sf::Color{0xA0, 0xA0, 0xA0} : sf::Color{0x80, 0x80, 0x80});
	rect.setPosition(box.pt1 + (box.pt2 - box.pt1) / 2);
	game->screen->draw(rect, states);
	rect.setFillColor(this->_cornerSelected == 6 || this->_cornerHovered == 6 ? sf::Color{0xA0, 0xA0, 0xA0} : sf::Color{0x80, 0x80, 0x80});
	rect.setPosition(box.pt2 + (box.pt3 - box.pt2) / 2);
	game->screen->draw(rect, states);
	rect.setFillColor(this->_cornerSelected == 7 || this->_cornerHovered == 7 ? sf::Color{0xA0, 0xA0, 0xA0} : sf::Color{0x80, 0x80, 0x80});
	rect.setPosition(box.pt3 + (box.pt4 - box.pt3) / 2);
	game->screen->draw(rect, states);
	rect.setFillColor(this->_cornerSelected == 8 || this->_cornerHovered == 8 ? sf::Color{0xA0, 0xA0, 0xA0} : sf::Color{0x80, 0x80, 0x80});
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

	if (this->displayBoxes && !this->showingPalette) {
		size_t index = 1;
		auto modifiedHurtBoxes = this->_object._getModifiedHurtBoxes(this->displaceObject);
		auto modifiedHitBoxes = this->_object._getModifiedHitBoxes(this->displaceObject);

		for (auto &hurtBox : modifiedHurtBoxes) {
			this->_drawBox(hurtBox, Color::Green, statesSFML, this->_boxHovered == index, this->_boxSelected == index);
			index++;
		}
		for (auto &hitBox : modifiedHitBoxes) {
			this->_drawBox(hitBox, Color::Red, statesSFML, this->_boxHovered == index, this->_boxSelected == index);
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

			this->_drawBox(rect, Color::Yellow, statesSFML, this->_boxHovered == index, this->_boxSelected == index);
			if (this->_boxSelected == data.hurtBoxes.size() + data.hitBoxes.size() + 1)
				this->_drawBoxBorder(rect, statesSFML, false);
		}

		if (this->_boxSelected == 0);
		else if (this->_boxSelected <= data.hurtBoxes.size())
			this->_drawBoxBorder(modifiedHurtBoxes[this->_boxSelected - 1], statesSFML, true);
		else if (this->_boxSelected <= data.hurtBoxes.size() + data.hitBoxes.size())
			this->_drawBoxBorder(modifiedHitBoxes[this->_boxSelected - 1 - data.hurtBoxes.size()], statesSFML, true);
	}
}

void SpiralOfFate::PreviewWidget::_updateBoxSliderHover(const SpiralOfFate::Rectangle &box, const tgui::Vector2f &pos, bool rotate, tgui::Cursor::Type &cursor)
{
	Vector2f centers[] = {
		box.pt1,
		box.pt2,
		box.pt3,
		box.pt4,
		box.pt1 + (box.pt2 - box.pt1) / 2,
		box.pt2 + (box.pt3 - box.pt2) / 2,
		box.pt3 + (box.pt4 - box.pt3) / 2,
		box.pt4 + (box.pt1 - box.pt4) / 2,
	};
	float angle = this->displaceObject && rotate ? this->_object._rotation : 0;

	this->_cornerHovered = 0;
	for (size_t i = 0; i < std::size(centers); i++) {
		Rectangle rect{
			(centers[i] + Vector2f{-DRAG_CORNER_SIZE / 2.f, -DRAG_CORNER_SIZE / 2.f}).rotation(angle, centers[i]),
			(centers[i] + Vector2f{ DRAG_CORNER_SIZE / 2.f, -DRAG_CORNER_SIZE / 2.f}).rotation(angle, centers[i]),
			(centers[i] + Vector2f{ DRAG_CORNER_SIZE / 2.f,  DRAG_CORNER_SIZE / 2.f}).rotation(angle, centers[i]),
			(centers[i] + Vector2f{-DRAG_CORNER_SIZE / 2.f,  DRAG_CORNER_SIZE / 2.f}).rotation(angle, centers[i]),
		};

		if (rect.contains(pos)) {
			size_t cursorIndex[] = {0, 6, 4, 2, 7, 5, 3, 1};
			tgui::Cursor::Type cursors[] = {
				tgui::Cursor::Type::SizeTopLeft,
				tgui::Cursor::Type::SizeTop,
				tgui::Cursor::Type::SizeTopRight,
				tgui::Cursor::Type::SizeRight,
				tgui::Cursor::Type::SizeBottomRight,
				tgui::Cursor::Type::SizeBottom,
				tgui::Cursor::Type::SizeBottomLeft,
				tgui::Cursor::Type::SizeLeft
			};

			this->_cornerHovered = i + 1;
			i = cursorIndex[i];
			i += angle / M_PI_4;
			i %= 8;
			cursor = cursors[i];
			return;
		}
	}
}

void SpiralOfFate::PreviewWidget::_updateBoxSliderHover(const tgui::Vector2f &pos, tgui::Cursor::Type &cursor)
{
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

		if (this->_boxSelected == data.hurtBoxes.size() + data.hitBoxes.size() + 1)
			this->_updateBoxSliderHover(rect, pos, false, cursor);
	}

	if (this->_boxSelected <= data.hurtBoxes.size())
		this->_updateBoxSliderHover(this->_object._getModifiedHurtBoxes(this->displaceObject)[this->_boxSelected - 1], pos, true, cursor);
	else if (this->_boxSelected <= data.hurtBoxes.size() + data.hitBoxes.size())
		this->_updateBoxSliderHover(this->_object._getModifiedHitBoxes(this->displaceObject)[this->_boxSelected - 1 - data.hurtBoxes.size()], pos, true, cursor);
}

void SpiralOfFate::PreviewWidget::_updateHover(const tgui::Vector2f &pos)
{
	if (this->showingPalette || !this->displayBoxes)
		return;

	size_t index = 1;
	std::vector<size_t> old;
	size_t highest = 0;
	tgui::Cursor::Type cursor = tgui::Cursor::Type::Arrow;

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
	if (this->_boxSelected)
		this->_updateBoxSliderHover(pos, cursor);
	this->setMouseCursor(cursor);
}

void SpiralOfFate::PreviewWidget::_handleBoxResize(const tgui::Vector2f &pos)
{
	Vector2f diff = pos - this->_startMousePos;

	if (!this->_commited && diff.magnitude2() < 100)
		return;

	// TODO: Handle rotation
	auto &data = this->_object.getFrameData();
	float angle = this->displaceObject && this->_boxSelected <= data.hurtBoxes.size() + data.hitBoxes.size() ? this->_object._rotation : 0;
	Vector2f bpos = this->_boxSaved.pos;
	Vector2f bsize = this->_boxSaved.size;

	diff.rotate(-angle, {0, 0});
	switch (this->_cornerSelected) {
	case 1:
		// TopLeft
		bsize -= diff;
		bpos += diff;
		if (bsize.x < BOX_SIZE_MIN) {
			bpos.x -= BOX_SIZE_MIN - bsize.x;
			bsize.x = BOX_SIZE_MIN;
		}
		if (bsize.y < BOX_SIZE_MIN) {
			bpos.y -= BOX_SIZE_MIN - bsize.y;
			bsize.y = BOX_SIZE_MIN;
		}
		break;
	case 2:
		// BottomLeft
		bsize.y += diff.y;
		if (bsize.y < BOX_SIZE_MIN)
			bsize.y = BOX_SIZE_MIN;
		bsize.x -= diff.x;
		bpos.x += diff.x;
		if (bsize.x < BOX_SIZE_MIN) {
			bpos.x -= BOX_SIZE_MIN - bsize.x;
			bsize.x = BOX_SIZE_MIN;
		}
		break;
	case 3:
		// BottomRight
		bsize += diff;
		if (bsize.x < BOX_SIZE_MIN)
			bsize.x = BOX_SIZE_MIN;
		if (bsize.y < BOX_SIZE_MIN)
			bsize.y = BOX_SIZE_MIN;
		break;
	case 4:
		// TopRight
		bsize.y -= diff.y;
		bpos.y += diff.y;
		if (bsize.y < BOX_SIZE_MIN) {
			bpos.y -= BOX_SIZE_MIN - bsize.y;
			bsize.y = BOX_SIZE_MIN;
		}
		bsize.x += diff.x;
		if (bsize.x < BOX_SIZE_MIN)
			bsize.x = BOX_SIZE_MIN;
		break;
	case 5:
		// Left
		bsize.x -= diff.x;
		bpos.x += diff.x;
		if (bsize.x < BOX_SIZE_MIN) {
			bpos.x -= BOX_SIZE_MIN - bsize.x;
			bsize.x = BOX_SIZE_MIN;
		}
		break;
	case 6:
		// Bottom
		bsize.y += diff.y;
		if (bsize.y < BOX_SIZE_MIN)
			bsize.y = BOX_SIZE_MIN;
		break;
	case 7:
		// Right
		bsize.x += diff.x;
		if (bsize.x < BOX_SIZE_MIN)
			bsize.x = BOX_SIZE_MIN;
		break;
	case 8:
		// Top
		bsize.y -= diff.y;
		bpos.y += diff.y;
		if (bsize.y < BOX_SIZE_MIN) {
			bpos.y -= BOX_SIZE_MIN - bsize.y;
			bsize.y = BOX_SIZE_MIN;
		}
		break;
	}

	this->_main.updateTransaction([this, &bpos, &bsize]{
		auto box = this->getSelectedBox();

		return new BoxModificationOperation(
			this->_object,
			this->_editor.localize("operation.resize_box"),
			box.first, box.second, { bpos, bsize }
		);
	});
	this->_commited = true;
	this->_lastMousePos = pos;
}

void SpiralOfFate::PreviewWidget::_handleBoxMove(const tgui::Vector2f &pos)
{
	Vector2f diff = pos - this->_startMousePos;

	if (!this->_commited && diff.magnitude2() < 100)
		return;

	// TODO: Handle rotation
	auto &data = this->_object.getFrameData();
	float angle = this->displaceObject && this->_boxSelected <= data.hurtBoxes.size() + data.hitBoxes.size() ? this->_object._rotation : 0;
	auto box = this->_boxSaved;

	diff.rotate(-angle, {0, 0});
	box.pos += diff;
	this->_main.updateTransaction([this, &box]{
		auto b = this->getSelectedBox();

		return new BoxModificationOperation(
			this->_object,
			this->_editor.localize("operation.move_box"),
			b.first, b.second, box
		);
	});
	this->_commited = true;
	this->_lastMousePos = pos;
}

void SpiralOfFate::PreviewWidget::mouseMoved(tgui::Vector2f pos)
{
	auto translatedPos = pos;

	translatedPos.x -= 100;
	translatedPos.y -= this->getSize().y - 150;
	if (!this->_dragStarted)
		this->_updateHover(translatedPos);
	else if (this->_cornerSelected)
		this->_handleBoxResize(translatedPos);
	else if (this->_boxSelected)
		this->_handleBoxMove(translatedPos);
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
	this->_cornerHovered = 0;
	this->_cornerSelected = 0;
	this->_dragStarted = false;
	this->_commited = false;
}

bool SpiralOfFate::PreviewWidget::leftMousePressed(tgui::Vector2f pos)
{
	auto translatedPos = pos;

	translatedPos.x -= 100;
	translatedPos.y -= this->getSize().y - 150;
	if (this->_boxHovered == 0)
		this->_updateHover(translatedPos);
	if (this->_cornerHovered)
		this->_cornerSelected = this->_cornerHovered;
	else {
		this->_boxSelected = this->_boxHovered;
		if (this->_boxSelected == 0)
			this->onBoxUnselect.emit(this);
		else {
			auto b = this->getSelectedBox();
			this->onBoxSelect.emit(this, b.first, b.second);
		}
	}
	if (this->_boxSelected || this->_cornerSelected) {
		auto &data = this->_object.getFrameData();

		this->_boxSaved =
			this->_boxSelected == data.hurtBoxes.size() + data.hitBoxes.size() + 1 ? *data.collisionBox :
			this->_boxSelected <= data.hurtBoxes.size() ? data.hurtBoxes[this->_boxSelected - 1] :
			data.hitBoxes[this->_boxSelected - 1 - data.hurtBoxes.size()];
		this->_dragStarted = true;
		this->_commited = false;
		this->_main.startTransaction();
		this->_lastMousePos = translatedPos;
		this->_startMousePos = translatedPos;
		return true;
	}
	return ClickableWidget::leftMousePressed(pos);
}

void SpiralOfFate::PreviewWidget::leftMouseReleased(tgui::Vector2f pos)
{
	this->_cornerSelected = 0;
	if (this->_dragStarted)
		this->_main.commitTransaction();
	this->_dragStarted = false;
	ClickableWidget::leftMouseReleased(pos);
}

void SpiralOfFate::PreviewWidget::mouseNoLongerOnWidget()
{
	this->_cornerSelected = 0;
	if (this->_dragStarted)
		this->_main.cancelTransaction();
	this->_dragStarted = false;
	Widget::mouseNoLongerOnWidget();
}

void SpiralOfFate::PreviewWidget::keyPressed(const tgui::Event::KeyEvent &event)
{
	if (!event.alt && !event.control && !event.shift && !event.system && event.code == tgui::Event::KeyboardKey::Delete && this->_boxSelected)
		this->_main.applyOperation(new RemoveBoxOperation(
			this->_object,
			this->_editor.localize("operation.resize_box"),
			this->_boxSelected, this->_boxSelected
		));
	Widget::keyPressed(event);
}

bool SpiralOfFate::PreviewWidget::canHandleKeyPress(const tgui::Event::KeyEvent &event)
{
	if (!event.alt && !event.control && !event.shift && !event.system && event.code == tgui::Event::KeyboardKey::Delete && this->_boxSelected)
		return true;
	return Widget::canHandleKeyPress(event);
}

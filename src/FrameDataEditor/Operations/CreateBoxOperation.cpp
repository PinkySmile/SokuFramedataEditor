//
// Created by PinkySmile on 25/05/25.
//

#include "CreateBoxOperation.hpp"
#include "../UI/PreviewWidget.hpp"

SpiralOfFate::CreateBoxOperation::CreateBoxOperation(EditableObject &obj, const std::string &&name, BoxType type, PreviewWidget &preview) :
	_obj(obj),
	_preview(preview),
	_action(obj._action),
	_actionBlock(obj._actionBlock),
	_animation(obj._animation),
	_type(type),
	_fieldName(name)
{
}

void SpiralOfFate::CreateBoxOperation::apply()
{
	this->_obj._action = this->_action;
	this->_obj._actionBlock = this->_actionBlock;
	this->_obj._animation = this->_animation;

	auto &data = this->_obj.getFrameData();

	if (this->_type == BOXTYPE_COLLISIONBOX) {
		data.cBoxes.clear();
		data.cBoxes.push_back({-10, -10, 10, 10});
	} else if (this->_type == BOXTYPE_HURTBOX)
		data.hBoxes.push_back({-10, -10, 10, 10});
	else
		data.aBoxes.push_back({-10, -10, 10, 10});
}

void SpiralOfFate::CreateBoxOperation::undo()
{
	this->_obj._action = this->_action;
	this->_obj._actionBlock = this->_actionBlock;
	this->_obj._animation = this->_animation;

	auto &data = this->_obj.getFrameData();

	if (this->_type == BOXTYPE_COLLISIONBOX) {
		data.cBoxes.clear();
		this->_preview.boxDeleted(BOXTYPE_COLLISIONBOX, 0);
	} else if (this->_type == BOXTYPE_HURTBOX) {
		data.hBoxes.pop_back();
		this->_preview.boxDeleted(BOXTYPE_HURTBOX, data.hBoxes.size());
	} else {
		data.aBoxes.pop_back();
		this->_preview.boxDeleted(BOXTYPE_HITBOX, data.aBoxes.size());
	}
}

std::string SpiralOfFate::CreateBoxOperation::getName() const noexcept
{
	return this->_fieldName;
}

bool SpiralOfFate::CreateBoxOperation::hasModification() const
{
	return true;
}

bool SpiralOfFate::CreateBoxOperation::hasFramedataModification() const
{
	return true;
}

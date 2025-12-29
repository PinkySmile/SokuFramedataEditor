//
// Created by PinkySmile on 04/07/25.
//

#include "RemoveBoxOperation.hpp"

#include <utility>

SpiralOfFate::Box &SpiralOfFate::RemoveBoxOperation::_getBox()
{
	auto &data = this->_obj.getFrameData();

	if (this->_type == BOXTYPE_COLLISIONBOX)
		return *data.collisionBox;
	if (this->_type == BOXTYPE_HITBOX)
		return data.hitBoxes[this->_boxIndex];
	return data.hurtBoxes[this->_boxIndex];
}

SpiralOfFate::RemoveBoxOperation::RemoveBoxOperation(EditableObject &obj, const std::string &&name, BoxType type, unsigned boxIndex, RemoveBoxApply onApply) :
	_obj(obj),
	_onApply(std::move(onApply)),
	_action(obj._action),
	_actionBlock(obj._actionBlock),
	_animation(obj._animation),
	_boxIndex(boxIndex),
	_type(type),
	_oldValue(this->_getBox()),
	_fieldName(name)
{
}

void SpiralOfFate::RemoveBoxOperation::apply()
{
	this->_obj._action = this->_action;
	this->_obj._actionBlock = this->_actionBlock;
	this->_obj._animation = this->_animation;

	auto &data = this->_obj.getFrameData();

	if (this->_type == BOXTYPE_COLLISIONBOX) {
		delete data.collisionBox;
		data.collisionBox = nullptr;
	} else if (this->_type == BOXTYPE_HURTBOX)
		data.hurtBoxes.erase(data.hurtBoxes.begin() + this->_boxIndex);
	else
		data.hitBoxes.erase(data.hitBoxes.begin() + this->_boxIndex);
	this->_onApply(BOXTYPE_NONE, 0);
}

void SpiralOfFate::RemoveBoxOperation::undo()
{
	this->_obj._action = this->_action;
	this->_obj._actionBlock = this->_actionBlock;
	this->_obj._animation = this->_animation;

	auto &data = this->_obj.getFrameData();

	if (this->_type == BOXTYPE_COLLISIONBOX)
		data.collisionBox = new Box{this->_oldValue};
	else if (this->_type == BOXTYPE_HURTBOX)
		data.hurtBoxes.insert(data.hurtBoxes.begin() + this->_boxIndex , this->_oldValue);
	else
		data.hitBoxes.insert(data.hitBoxes.begin() + this->_boxIndex, this->_oldValue);
	this->_onApply(this->_type, this->_boxIndex);
}

std::string SpiralOfFate::RemoveBoxOperation::getName() const noexcept
{
	return this->_fieldName;
}

bool SpiralOfFate::RemoveBoxOperation::hasModification() const
{
	return true;
}

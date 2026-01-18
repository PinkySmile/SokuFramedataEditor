//
// Created by PinkySmile on 04/07/25.
//

#include "RemoveBoxOperation.hpp"

#include <utility>

ShadyCore::Schema::Sequence::BBox &SpiralOfFate::RemoveBoxOperation::_getBox()
{
	auto &data = this->_obj.getFrameData();

	if (this->_type == BOXTYPE_COLLISIONBOX)
		return data.cBoxes.front();
	if (this->_type == BOXTYPE_HITBOX)
		return data.aBoxes[this->_boxIndex];
	return data.hBoxes[this->_boxIndex];
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

	if (this->_type == BOXTYPE_COLLISIONBOX)
		data.cBoxes.clear();
	else if (this->_type == BOXTYPE_HURTBOX)
		data.hBoxes.erase(data.hBoxes.begin() + this->_boxIndex);
	else
		data.aBoxes.erase(data.aBoxes.begin() + this->_boxIndex);
	this->_onApply(BOXTYPE_NONE, 0);
}

void SpiralOfFate::RemoveBoxOperation::undo()
{
	this->_obj._action = this->_action;
	this->_obj._actionBlock = this->_actionBlock;
	this->_obj._animation = this->_animation;

	auto &data = this->_obj.getFrameData();

	if (this->_type == BOXTYPE_COLLISIONBOX) {
		data.cBoxes.resize(1);
		data.cBoxes.front() = this->_oldValue;
	} else if (this->_type == BOXTYPE_HURTBOX)
		data.hBoxes.insert(data.hBoxes.begin() + this->_boxIndex , this->_oldValue);
	else
		data.aBoxes.insert(data.aBoxes.begin() + this->_boxIndex, this->_oldValue);
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

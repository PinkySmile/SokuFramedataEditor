//
// Created by PinkySmile on 23/05/25.
//

#include "BoxModificationOperation.hpp"

SpiralOfFate::Box &SpiralOfFate::BoxModificationOperation::_getBox()
{
	auto &data = this->_obj.getFrameData();

	return this->_boxIndex == data.hurtBoxes.size() + data.hitBoxes.size() + 1 ? *data.collisionBox :
	       this->_boxIndex <= data.hurtBoxes.size() ? data.hurtBoxes[this->_boxIndex - 1] :
	       data.hitBoxes[this->_boxIndex - 1 - data.hurtBoxes.size()];
}

SpiralOfFate::BoxModificationOperation::BoxModificationOperation(
	EditableObject &obj,
	const std::string &&name,
	unsigned int boxIndex,
	SpiralOfFate::Box newValue
) :
	_obj(obj),
	_action(obj._action),
	_actionBlock(obj._actionBlock),
	_animation(obj._animation),
	_boxIndex(boxIndex),
	_oldValue(this->_getBox()),
	_newValue(newValue),
	_fieldName(name)
{
}

void SpiralOfFate::BoxModificationOperation::apply()
{
	bool shouldReset =
		this->_obj._action != this->_action ||
		this->_obj._actionBlock != this->_actionBlock ||
		this->_obj._animation != this->_animation;

	this->_obj._action = this->_action;
	this->_obj._actionBlock = this->_actionBlock;
	this->_obj._animation = this->_animation;
	this->_getBox() = this->_newValue;
	if (shouldReset) {
		this->_obj._animationCtr = 0;
		this->_obj.resetState();
	}
}

void SpiralOfFate::BoxModificationOperation::undo()
{
	bool shouldReset =
		this->_obj._action != this->_action ||
		this->_obj._actionBlock != this->_actionBlock ||
		this->_obj._animation != this->_animation;

	this->_obj._action = this->_action;
	this->_obj._actionBlock = this->_actionBlock;
	this->_obj._animation = this->_animation;
	this->_getBox() = this->_oldValue;
	if (shouldReset) {
		this->_obj._animationCtr = 0;
		this->_obj.resetState();
	}
}

std::string SpiralOfFate::BoxModificationOperation::getName() const noexcept
{
	return this->_fieldName;
}

bool SpiralOfFate::BoxModificationOperation::hasModification() const
{
	return this->_oldValue != this->_newValue;
}

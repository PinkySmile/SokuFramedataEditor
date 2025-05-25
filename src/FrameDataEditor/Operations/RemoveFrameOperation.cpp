//
// Created by PinkySmile on 25/05/25.
//

#include "RemoveFrameOperation.hpp"

SpiralOfFate::RemoveFrameOperation::RemoveFrameOperation(EditableObject &obj, const std::string &&name) :
	_obj(obj),
	_action(obj._action),
	_actionBlock(obj._actionBlock),
	_animation(obj._animation),
	_oldValue(this->_obj._moves[this->_action][this->_actionBlock][this->_animation]),
	_fieldName(name)
{
}

void SpiralOfFate::RemoveFrameOperation::apply()
{
	this->_obj._action = this->_action;
	this->_obj._actionBlock = this->_actionBlock;
	this->_obj._animation = this->_animation;

	auto &arr = this->_obj._moves[this->_action][this->_actionBlock];

	arr.erase(arr.begin() + this->_animation);
	if (arr.size() == this->_animation)
		this->_obj._animation--;
}

void SpiralOfFate::RemoveFrameOperation::undo()
{
	this->_obj._action = this->_action;
	this->_obj._actionBlock = this->_actionBlock;
	this->_obj._animation = this->_animation;

	auto &arr = this->_obj._moves[this->_action][this->_actionBlock];

	arr.insert(arr.begin() + this->_animation, this->_oldValue);
	if (arr.size() == this->_animation)
		this->_obj._animation--;
}

std::string SpiralOfFate::RemoveFrameOperation::getName() const noexcept
{
	return this->_fieldName;
}

bool SpiralOfFate::RemoveFrameOperation::hasModification() const
{
	return true;
}

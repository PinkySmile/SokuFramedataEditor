//
// Created by PinkySmile on 25/05/25.
//

#include "RemoveFrameOperation.hpp"

SpiralOfFate::RemoveFrameOperation::RemoveFrameOperation(EditableObject &obj, const std::string &&name) :
	_obj(obj),
	_action(obj._action),
	_actionBlock(obj._actionBlock),
	_animation(obj._animation),
	_oldValue(this->_obj._schema.framedata[this->_action][this->_actionBlock][this->_animation]),
	_fieldName(name)
{
}

void SpiralOfFate::RemoveFrameOperation::apply()
{
	this->_obj._action = this->_action;
	this->_obj._actionBlock = this->_actionBlock;
	this->_obj._animation = this->_animation;

	auto &arr = this->_obj._schema.framedata[this->_action][this->_actionBlock];

	arr.data.erase(arr.data.begin() + this->_animation);
	if (arr.data.size() == this->_obj._animation)
		this->_obj._animation = arr.data.size() - 1;
}

void SpiralOfFate::RemoveFrameOperation::undo()
{
	this->_obj._action = this->_action;
	this->_obj._actionBlock = this->_actionBlock;
	this->_obj._animation = this->_animation;

	auto &arr = this->_obj._schema.framedata[this->_action][this->_actionBlock];

	arr.data.insert(arr.begin() + this->_animation, this->_oldValue);
}

std::string SpiralOfFate::RemoveFrameOperation::getName() const noexcept
{
	return this->_fieldName;
}

bool SpiralOfFate::RemoveFrameOperation::hasModification() const
{
	return true;
}

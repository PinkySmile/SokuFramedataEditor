//
// Created by PinkySmile on 30/12/2025.
//

#include "RemoveBlockOperation.hpp"

SpiralOfFate::RemoveBlockOperation::RemoveBlockOperation(
	EditableObject &obj,
	const std::string &&name,
	unsigned id
) :
	_obj(obj),
	_name(name),
	_action(obj._action),
	_id(id),
	_oldData(obj._schema.framedata[obj._action][id])
{
}

void SpiralOfFate::RemoveBlockOperation::apply()
{
	this->_obj._action = this->_action;
	this->_obj._actionBlock = this->_id;
	this->_obj._animation = 0;

	auto &arr = this->_obj._schema.framedata[this->_action];

	if (arr.size() <= this->_obj._actionBlock)
		this->_obj._actionBlock = arr.size() - 1;
	arr.sequences().erase(arr.begin() + this->_id);
}

void SpiralOfFate::RemoveBlockOperation::undo()
{
	this->_obj._action = this->_action;
	this->_obj._actionBlock = this->_id;
	this->_obj._animation = 0;

	auto &arr = this->_obj._schema.framedata[this->_action];

	arr.sequences().insert(arr.begin() + this->_id, this->_oldData);
}

bool SpiralOfFate::RemoveBlockOperation::hasModification() const
{
	return true;
}

std::string SpiralOfFate::RemoveBlockOperation::getName() const noexcept
{
	return this->_name;
}
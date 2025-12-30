//
// Created by PinkySmile on 24/05/25.
//

#include "CreateBlockOperation.hpp"

SpiralOfFate::CreateBlockOperation::CreateBlockOperation(
	EditableObject &obj,
	const std::string &&name,
	unsigned id,
	const std::vector<FrameData> &newData
) :
	_obj(obj),
	_name(name),
	_action(obj._action),
	_id(id),
	_newData(newData)
{
}

void SpiralOfFate::CreateBlockOperation::apply()
{
	this->_obj._action = this->_action;
	this->_obj._actionBlock = this->_id;
	this->_obj._animation = 0;

	auto &arr = this->_obj._moves[this->_action];

	arr.insert(arr.begin() + this->_id, this->_newData);
}

void SpiralOfFate::CreateBlockOperation::undo()
{
	this->_obj._action = this->_action;
	this->_obj._actionBlock = this->_id - 1;
	this->_obj._animation = 0;

	auto &arr = this->_obj._moves[this->_action];

	if (arr.size() == this->_obj._actionBlock)
		this->_obj._actionBlock = arr.size() - 1;
	arr.erase(arr.begin() + this->_id);
}

bool SpiralOfFate::CreateBlockOperation::hasModification() const
{
	return true;
}

std::string SpiralOfFate::CreateBlockOperation::getName() const noexcept
{
	return this->_name;
}
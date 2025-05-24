//
// Created by PinkySmile on 24/05/25.
//

#include "CreateFrameOperation.hpp"

SpiralOfFate::CreateFrameOperation::CreateFrameOperation(
	EditableObject &obj,
	const std::string &&name,
	unsigned action,
	unsigned block,
	unsigned id,
	const FrameData &newData
) :
	_obj(obj),
	_name(name),
	_action(action),
	_block(block),
	_id(id),
	_newData(newData)
{
}

void SpiralOfFate::CreateFrameOperation::apply()
{
	this->_obj._action = this->_action;
	this->_obj._actionBlock = this->_block;
	this->_obj._animation = this->_id;

	auto &arr = this->_obj._moves[this->_action][this->_block];

	arr.insert(arr.begin() + this->_id, this->_newData);
}

void SpiralOfFate::CreateFrameOperation::undo()
{
	this->_obj._action = this->_action;
	this->_obj._actionBlock = this->_block;
	this->_obj._animation = this->_id;

	auto &arr = this->_obj._moves[this->_action][this->_block];

	if (arr.size() == this->_obj._animation)
		this->_obj._animation = arr.size() - 1;
	arr.erase(arr.begin() + this->_id);
}

bool SpiralOfFate::CreateFrameOperation::hasModification() const
{
	return true;
}

std::string SpiralOfFate::CreateFrameOperation::getName() const noexcept
{
	return this->_name;
}
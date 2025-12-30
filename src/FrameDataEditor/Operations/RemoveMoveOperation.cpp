//
// Created by PinkySmile on 30/12/2025.
//

#include "RemoveMoveOperation.hpp"

SpiralOfFate::RemoveMoveOperation::RemoveMoveOperation(
	EditableObject &obj,
	const std::string &&name,
	unsigned id
) :
	_obj(obj),
	_name(name),
	_id(id),
	_oldData(obj._moves[id])
{
}

void SpiralOfFate::RemoveMoveOperation::apply()
{
	this->_obj._moves.erase(this->_id);
	if (this->_obj._action == this->_id) {
		this->_obj._action = this->_obj._moves.begin()->first;
		this->_obj._actionBlock = 0;
		this->_obj._animation = 0;
	}
}

void SpiralOfFate::RemoveMoveOperation::undo()
{
	this->_obj._action = this->_id;
	this->_obj._actionBlock = 0;
	this->_obj._animation = 0;
	this->_obj._moves[this->_id] = this->_oldData;
}

bool SpiralOfFate::RemoveMoveOperation::hasModification() const
{
	return true;
}

std::string SpiralOfFate::RemoveMoveOperation::getName() const noexcept
{
	return this->_name;
}
//
// Created by PinkySmile on 24/05/25.
//

#include "CreateMoveOperation.hpp"

SpiralOfFate::CreateMoveOperation::CreateMoveOperation(
	EditableObject &obj,
	const std::string &&name,
	unsigned int id,
	const FrameData::Action &newData
) :
	_obj(obj),
	_name(name),
	_id(id),
	_newData(newData)
{
}

void SpiralOfFate::CreateMoveOperation::apply()
{
	this->_obj._action = this->_id;
	this->_obj._actionBlock = 0;
	this->_obj._animation = 0;
	this->_obj._schema.framedata[this->_id] = this->_newData;
}

void SpiralOfFate::CreateMoveOperation::undo()
{
	this->_obj._schema.framedata.erase(this->_id);
	if (this->_obj._action == this->_id) {
		this->_obj._action = this->_obj._schema.framedata.begin()->first;
		this->_obj._actionBlock = 0;
		this->_obj._animation = 0;
	}
}

bool SpiralOfFate::CreateMoveOperation::hasModification() const
{
	return true;
}

std::string SpiralOfFate::CreateMoveOperation::getName() const noexcept
{
	return this->_name;
}

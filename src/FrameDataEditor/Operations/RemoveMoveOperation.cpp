//
// Created by PinkySmile on 30/12/2025.
//

#include "RemoveMoveOperation.hpp"

#include <ranges>

SpiralOfFate::RemoveMoveOperation::RemoveMoveOperation(
	EditableObject &obj,
	const std::string &&name,
	unsigned id
) :
	_obj(obj),
	_name(name),
	_id(id),
	_oldData(obj._schema.framedata[id])
{
	for (auto &[k, a] : obj._schema.framedata)
		if (a.cloned && a.clonedId == id)
			this->_clones.push_back(k);
}

void SpiralOfFate::RemoveMoveOperation::apply()
{
	this->_obj._schema.framedata.erase(this->_id);
	for (auto clone : this->_clones)
		this->_obj._schema.framedata.erase(clone);
	if (this->_obj._action == this->_id) {
		this->_obj._action = this->_obj._schema.framedata.begin()->first;
		this->_obj._actionBlock = 0;
		this->_obj._animation = 0;
	}
}

void SpiralOfFate::RemoveMoveOperation::undo()
{
	this->_obj._action = this->_id;
	this->_obj._actionBlock = 0;
	this->_obj._animation = 0;
	this->_obj._schema.framedata[this->_id] = this->_oldData;
	for (auto clone : this->_clones)
		this->_obj._schema.framedata[clone] = {
			._sequences = {},
			.cloned = &this->_obj._schema.framedata[this->_id],
			.clonedId = this->_id
		};
}

bool SpiralOfFate::RemoveMoveOperation::hasModification() const
{
	return true;
}

std::string SpiralOfFate::RemoveMoveOperation::getName() const noexcept
{
	return this->_name;
}

bool SpiralOfFate::RemoveMoveOperation::hasFramedataModification() const
{
	return true;
}

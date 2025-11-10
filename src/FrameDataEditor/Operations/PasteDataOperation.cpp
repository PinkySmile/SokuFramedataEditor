//
// Created by PinkySmile on 10/11/2025.
//

#include "PasteDataOperation.hpp"

namespace SpiralOfFate
{
	PasteDataOperation::PasteDataOperation(
		EditableObject &obj,
		const std::string &&name,
		FrameData newValue
	) :
		_obj(obj),
		_action(obj._action),
		_blockId(obj._actionBlock),
		_id(obj._animation),
		_oldValue(obj.getFrameData()),
		_newValue(newValue),
		_fieldName(name)
	{
	}

	void PasteDataOperation::apply()
	{
		this->_obj._action = this->_action;
		this->_obj._actionBlock = this->_blockId;
		this->_obj._animation = this->_id;
		this->_obj._moves[this->_action][this->_blockId][this->_id] = this->_newValue;
	}

	void PasteDataOperation::undo()
	{
		this->_obj._action = this->_action;
		this->_obj._actionBlock = this->_blockId;
		this->_obj._animation = this->_id;
		this->_obj._moves[this->_action][this->_blockId][this->_id] = this->_oldValue;
	}

	std::string PasteDataOperation::getName() const noexcept
	{
		return this->_fieldName;
	}

	bool PasteDataOperation::hasModification() const
	{
		return true;
	}
}
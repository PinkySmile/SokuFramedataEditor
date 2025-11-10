//
// Created by PinkySmile on 10/11/2025.
//

#include "PasteBoxDataOperation.hpp"

namespace SpiralOfFate
{
	PasteBoxDataOperation::PasteBoxDataOperation(
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

	void PasteBoxDataOperation::apply()
	{
		this->_obj._action = this->_action;
		this->_obj._actionBlock = this->_blockId;
		this->_obj._animation = this->_id;

		auto &data = this->_obj._moves[this->_action][this->_blockId][this->_id];

		delete data.collisionBox;
		if (this->_newValue.collisionBox)
			data.collisionBox = new Box{*this->_newValue.collisionBox};
		else
			data.collisionBox = nullptr;
		data.hurtBoxes = this->_newValue.hurtBoxes;
		data.hitBoxes = this->_newValue.hitBoxes;
	}

	void PasteBoxDataOperation::undo()
	{
		this->_obj._action = this->_action;
		this->_obj._actionBlock = this->_blockId;
		this->_obj._animation = this->_id;
		this->_obj._moves[this->_action][this->_blockId][this->_id] = this->_oldValue;
	}

	std::string PasteBoxDataOperation::getName() const noexcept
	{
		return this->_fieldName;
	}

	bool PasteBoxDataOperation::hasModification() const
	{
		return true;
	}
}
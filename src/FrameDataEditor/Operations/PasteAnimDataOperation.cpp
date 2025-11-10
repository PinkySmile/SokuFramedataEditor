//
// Created by PinkySmile on 10/11/2025.
//

#include "PasteAnimDataOperation.hpp"

namespace SpiralOfFate
{
	PasteAnimDataOperation::PasteAnimDataOperation(
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

	void PasteAnimDataOperation::apply()
	{
		this->_obj._action = this->_action;
		this->_obj._actionBlock = this->_blockId;
		this->_obj._animation = this->_id;

		auto &data = this->_obj._moves[this->_action][this->_blockId][this->_id];

		game->textureMgr.remove(data.textureHandle);
		game->textureMgr.addRef(this->_newValue.textureHandle);
		data.textureHandle = this->_newValue.textureHandle;
		data.spritePath = this->_newValue.spritePath;
		data.textureBounds = this->_newValue.textureBounds;
		data.scale = this->_newValue.scale;
		data.offset = this->_newValue.offset;
	}

	void PasteAnimDataOperation::undo()
	{
		this->_obj._action = this->_action;
		this->_obj._actionBlock = this->_blockId;
		this->_obj._animation = this->_id;
		this->_obj._moves[this->_action][this->_blockId][this->_id] = this->_oldValue;
	}

	std::string PasteAnimDataOperation::getName() const noexcept
	{
		return this->_fieldName;
	}

	bool PasteAnimDataOperation::hasModification() const
	{
		return true;
	}
}
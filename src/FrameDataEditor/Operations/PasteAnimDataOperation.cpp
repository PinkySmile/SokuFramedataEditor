//
// Created by PinkySmile on 10/11/2025.
//

#include "PasteAnimDataOperation.hpp"

namespace SpiralOfFate
{
	PasteAnimDataOperation::PasteAnimDataOperation(
		EditableObject &obj,
		const std::string &&name,
		const FrameData &newValue
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

		auto &data = this->_obj._schema.framedata[this->_action][this->_blockId][this->_id];

		game->textureMgr.remove(data.textureHandle);
		game->textureMgr.addRef(this->_newValue.textureHandle);
		data.textureHandle = this->_newValue.textureHandle;
		data.imageIndex = this->_newValue.imageIndex;
		data.renderGroup = this->_newValue.renderGroup;
		data.blendOptions = this->_newValue.blendOptions;
	}

	void PasteAnimDataOperation::undo()
	{
		this->_obj._action = this->_action;
		this->_obj._actionBlock = this->_blockId;
		this->_obj._animation = this->_id;
		this->_obj._schema.framedata[this->_action][this->_blockId][this->_id] = this->_oldValue;
	}

	std::string PasteAnimDataOperation::getName() const noexcept
	{
		return this->_fieldName;
	}

	bool PasteAnimDataOperation::hasModification() const
	{
		return true;
	}

	bool PasteAnimDataOperation::hasFramedataModification() const
	{
		return true;
	}
}

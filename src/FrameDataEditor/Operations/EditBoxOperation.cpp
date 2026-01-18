//
// Created by PinkySmile on 23/05/25.
//

#include "EditBoxOperation.hpp"

ShadyCore::Schema::Sequence::BBox &SpiralOfFate::EditBoxOperation::_getBox()
{
	auto &data = this->_obj.getFrameData();

	if (this->_type == BOXTYPE_COLLISIONBOX)
		return data.cBoxes.front();
	if (this->_type == BOXTYPE_HITBOX)
		return data.aBoxes[this->_boxIndex];
	return data.hBoxes[this->_boxIndex];
}

SpiralOfFate::EditBoxOperation::EditBoxOperation(
	EditableObject &obj,
	const std::string &&name,
	BoxType type,
	unsigned int boxIndex,
	ShadyCore::Schema::Sequence::BBox newValue
) :
	_obj(obj),
	_action(obj._action),
	_actionBlock(obj._actionBlock),
	_animation(obj._animation),
	_boxIndex(boxIndex),
	_type(type),
	_oldValue(this->_getBox()),
	_newValue(newValue),
	_fieldName(name)
{
}

void SpiralOfFate::EditBoxOperation::apply()
{
	bool shouldReset =
		this->_obj._action != this->_action ||
		this->_obj._actionBlock != this->_actionBlock ||
		this->_obj._animation != this->_animation;

	this->_obj._action = this->_action;
	this->_obj._actionBlock = this->_actionBlock;
	this->_obj._animation = this->_animation;
	this->_getBox() = this->_newValue;
	if (shouldReset) {
		this->_obj._animationCtr = 0;
		this->_obj.resetState();
	}
}

void SpiralOfFate::EditBoxOperation::undo()
{
	bool shouldReset =
		this->_obj._action != this->_action ||
		this->_obj._actionBlock != this->_actionBlock ||
		this->_obj._animation != this->_animation;

	this->_obj._action = this->_action;
	this->_obj._actionBlock = this->_actionBlock;
	this->_obj._animation = this->_animation;
	this->_getBox() = this->_oldValue;
	if (shouldReset) {
		this->_obj._animationCtr = 0;
		this->_obj.resetState();
	}
}

std::string SpiralOfFate::EditBoxOperation::getName() const noexcept
{
	return this->_fieldName;
}

bool SpiralOfFate::EditBoxOperation::hasModification() const
{
	return
		this->_oldValue.down != this->_newValue.down ||
		this->_oldValue.up != this->_newValue.up ||
		this->_oldValue.left != this->_newValue.left ||
		this->_oldValue.right != this->_newValue.right;
}

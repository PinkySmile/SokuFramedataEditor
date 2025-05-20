//
// Created by PinkySmile on 11/05/25.
//

#include "ClearBlockOperation.hpp"

SpiralOfFate::ClearBlockOperation::ClearBlockOperation(EditableObject &obj, const SpiralOfFate::FrameDataEditor &editor) :
	_obj(obj),
	_action(obj._action),
	_actionBlock(obj._actionBlock),
	_animation(obj._animation),
	_name(editor.localize("operation.clearblock"))
{
	auto &data = obj.getFrameData();

	this->_oldBlockPlayerHitStop = data.blockPlayerHitStop;
	this->_oldBlockOpponentHitStop = data.blockOpponentHitStop;
	this->_oldPushBack = data.pushBack;
	this->_oldPushBlock = data.pushBlock;
	this->_oldGuardDmg = data.guardDmg;
	this->_oldChipDamage = data.chipDamage;
	this->_oldBlockStun = data.blockStun;
	this->_oldWrongBlockStun = data.wrongBlockStun;
}

void SpiralOfFate::ClearBlockOperation::apply()
{
	this->_obj._action = this->_action;
	this->_obj._actionBlock = this->_actionBlock;
	this->_obj._animation = this->_animation;

	auto &data = this->_obj.getFrameData();

	data.blockPlayerHitStop = 0;
	data.blockOpponentHitStop = 0;
	data.pushBack = 0;
	data.pushBlock = 0;
	data.guardDmg = 0;
	data.chipDamage = 0;
	data.blockStun = 0;
	data.wrongBlockStun = 0;
}

void SpiralOfFate::ClearBlockOperation::undo()
{
	this->_obj._action = this->_action;
	this->_obj._actionBlock = this->_actionBlock;
	this->_obj._animation = this->_animation;

	auto &data = this->_obj.getFrameData();

	data.blockPlayerHitStop = this->_oldBlockPlayerHitStop;
	data.blockOpponentHitStop = this->_oldBlockOpponentHitStop;
	data.pushBack = this->_oldPushBack;
	data.pushBlock = this->_oldPushBlock;
	data.guardDmg = this->_oldGuardDmg;
	data.chipDamage = this->_oldChipDamage;
	data.blockStun = this->_oldBlockStun;
	data.wrongBlockStun = this->_oldWrongBlockStun;
}

std::string SpiralOfFate::ClearBlockOperation::getName() const noexcept
{
	return this->_name;
}

bool SpiralOfFate::ClearBlockOperation::hasModification() const
{
	return
		this->_oldBlockPlayerHitStop != 0 ||
		this->_oldBlockOpponentHitStop != 0 ||
		this->_oldPushBack != 0 ||
		this->_oldPushBlock != 0 ||
		this->_oldGuardDmg != 0 ||
		this->_oldChipDamage != 0 ||
		this->_oldBlockStun != 0 ||
		this->_oldWrongBlockStun != 0;
}

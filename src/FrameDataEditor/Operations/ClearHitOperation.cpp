//
// Created by PinkySmile on 11/05/25.
//

#include "ClearHitOperation.hpp"

SpiralOfFate::ClearHitOperation::ClearHitOperation(EditableObject &obj, const SpiralOfFate::FrameDataEditor &editor) :
	_obj(obj),
	_action(obj._action),
	_actionBlock(obj._actionBlock),
	_animation(obj._animation),
	_name(editor.localize("operation.clearhit"))
{
	auto &data = obj.getFrameData();

	this->_oldHitPlayerHitStop = data.hitPlayerHitStop;
	this->_oldHitOpponentHitStop = data.hitOpponentHitStop;
	this->_oldHitSpeed = data.hitSpeed;
	this->_oldCounterHitSpeed = data.counterHitSpeed;
	this->_oldNeutralLimit = data.neutralLimit;
	this->_oldSpiritLimit = data.spiritLimit;
	this->_oldMatterLimit = data.matterLimit;
	this->_oldVoidLimit = data.voidLimit;
	this->_oldProrate = data.prorate;
	this->_oldMinProrate = data.minProrate;
	this->_oldUntech = data.untech;
	this->_oldDamage = data.damage;
	this->_oldHitStun = data.hitStun;
	this->_oldHitSoundPath = data.hitSoundPath;
	this->_oldSnap = data.snap;
}

void SpiralOfFate::ClearHitOperation::apply()
{
	this->_obj._action = this->_action;
	this->_obj._actionBlock = this->_actionBlock;
	this->_obj._animation = this->_animation;

	auto &data = this->_obj.getFrameData();

	data.hitPlayerHitStop = 0;
	data.hitOpponentHitStop = 0;
	data.hitSpeed = {0, 0};
	data.counterHitSpeed = {0, 0};
	data.neutralLimit = 0;
	data.spiritLimit = 0;
	data.matterLimit = 0;
	data.voidLimit = 0;
	data.prorate = 0;
	data.minProrate = 0;
	data.untech = 0;
	data.damage = 0;
	data.hitStun = 0;
	data.snap.reset();
	data.hitSoundPath.clear();
	data.reloadSound();
}

void SpiralOfFate::ClearHitOperation::undo()
{
	this->_obj._action = this->_action;
	this->_obj._actionBlock = this->_actionBlock;
	this->_obj._animation = this->_animation;

	auto &data = this->_obj.getFrameData();

	data.hitPlayerHitStop = this->_oldHitPlayerHitStop;
	data.hitOpponentHitStop = this->_oldHitOpponentHitStop;
	data.hitSpeed = this->_oldHitSpeed;
	data.counterHitSpeed = this->_oldCounterHitSpeed;
	data.neutralLimit = this->_oldNeutralLimit;
	data.spiritLimit = this->_oldSpiritLimit;
	data.matterLimit = this->_oldMatterLimit;
	data.voidLimit = this->_oldVoidLimit;
	data.prorate = this->_oldProrate;
	data.minProrate = this->_oldMinProrate;
	data.untech = this->_oldUntech;
	data.damage = this->_oldDamage;
	data.hitStun = this->_oldHitStun;
	data.hitSoundPath = this->_oldHitSoundPath;
	data.snap = this->_oldSnap;
	data.reloadSound();
}

std::string SpiralOfFate::ClearHitOperation::getName() const noexcept
{
	return this->_name;
}

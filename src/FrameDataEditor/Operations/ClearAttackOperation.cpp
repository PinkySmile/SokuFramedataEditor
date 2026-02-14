//
// Created by PinkySmile on 11/05/25.
//

#include "ClearAttackOperation.hpp"

SpiralOfFate::ClearAttackOperation::ClearAttackOperation(EditableObject &obj, const SpiralOfFate::FrameDataEditor &editor) :
	_obj(obj),
	_action(obj._action),
	_actionBlock(obj._actionBlock),
	_animation(obj._animation),
	_name(editor.localize("operation.clearhit"))
{
	auto &data = obj.getFrameData();

	this->oldTraits = data.traits;
}

void SpiralOfFate::ClearAttackOperation::apply()
{
	this->_obj._action = this->_action;
	this->_obj._actionBlock = this->_actionBlock;
	this->_obj._animation = this->_animation;

	auto &data = this->_obj.getFrameData();

	// On block
	data.traits.chipDamage = 0;
	data.traits.spiritDamage = 0;
	data.traits.onBlockPlayerStun = 0;
	data.traits.onBlockEnemyStun = 0;
	data.traits.onBlockCardGain = 0;

	// On hit
	data.traits.speedX = 0;
	data.traits.speedY = 0;
	data.traits.damage = 0;
	data.traits.proration = 0;
	data.traits.untech = 0;
	data.traits.power = 0;
	data.traits.limit = 0;
	data.traits.onHitPlayerStun = 0;
	data.traits.onHitEnemyStun = 0;
	data.traits.onHitCardGain = 0;
	data.traits.onAirHitSetSequence = 0;
	data.traits.onGroundHitSetSequence = 0;
	data.traits.onHitSfx = 0;
	data.traits.onHitEffect = 0;
	data.traits.attackLevel = 0;
	data.traits.comboModifier = 0;
}

void SpiralOfFate::ClearAttackOperation::undo()
{
	this->_obj._action = this->_action;
	this->_obj._actionBlock = this->_actionBlock;
	this->_obj._animation = this->_animation;

	auto &data = this->_obj.getFrameData();

	data.traits = this->oldTraits;
}

std::string SpiralOfFate::ClearAttackOperation::getName() const noexcept
{
	return this->_name;
}

bool SpiralOfFate::ClearAttackOperation::hasModification() const
{
	return true;
}

bool SpiralOfFate::ClearAttackOperation::hasFramedataModification() const
{
	return true;
}

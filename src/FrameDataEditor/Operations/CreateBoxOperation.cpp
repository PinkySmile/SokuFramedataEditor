//
// Created by PinkySmile on 25/05/25.
//

#include "CreateBoxOperation.hpp"

SpiralOfFate::CreateBoxOperation::CreateBoxOperation(EditableObject &obj, const std::string &&name, bool hurtbox) :
	_obj(obj),
	_action(obj._action),
	_actionBlock(obj._actionBlock),
	_animation(obj._animation),
	_hurtbox(hurtbox),
	_fieldName(name)
{
}

void SpiralOfFate::CreateBoxOperation::apply()
{
	this->_obj._action = this->_action;
	this->_obj._actionBlock = this->_actionBlock;
	this->_obj._animation = this->_animation;

	auto &data = this->_obj.getFrameData();

	if (this->_hurtbox)
		data.hurtBoxes.push_back({{-20, -20}, {40, 40}});
	else
		data.hitBoxes.push_back({{-20, -20}, {40, 40}});
}

void SpiralOfFate::CreateBoxOperation::undo()
{
	this->_obj._action = this->_action;
	this->_obj._actionBlock = this->_actionBlock;
	this->_obj._animation = this->_animation;

	auto &data = this->_obj.getFrameData();

	if (this->_hurtbox)
		data.hurtBoxes.pop_back();
	else
		data.hitBoxes.pop_back();
}

std::string SpiralOfFate::CreateBoxOperation::getName() const noexcept
{
	return this->_fieldName;
}

bool SpiralOfFate::CreateBoxOperation::hasModification() const
{
	return true;
}

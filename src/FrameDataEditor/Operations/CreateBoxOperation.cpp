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
		data.hBoxes.push_back({-10, -10, 10, 10});
	else
		data.aBoxes.push_back({-10, -10, 10, 10});
}

void SpiralOfFate::CreateBoxOperation::undo()
{
	this->_obj._action = this->_action;
	this->_obj._actionBlock = this->_actionBlock;
	this->_obj._animation = this->_animation;

	auto &data = this->_obj.getFrameData();

	if (this->_hurtbox)
		data.hBoxes.pop_back();
	else
		data.hBoxes.pop_back();
}

std::string SpiralOfFate::CreateBoxOperation::getName() const noexcept
{
	return this->_fieldName;
}

bool SpiralOfFate::CreateBoxOperation::hasModification() const
{
	return true;
}

bool SpiralOfFate::CreateBoxOperation::hasFramedataModification() const
{
	return true;
}

//
// Created by PinkySmile on 04/07/25.
//

#include "RemoveBoxOperation.hpp"

SpiralOfFate::Box &SpiralOfFate::RemoveBoxOperation::_getBox()
{
	auto &data = this->_obj.getFrameData();

	return this->_boxIndex == data.hurtBoxes.size() + data.hitBoxes.size() + 1 ? *data.collisionBox :
	       this->_boxIndex <= data.hurtBoxes.size() ? data.hurtBoxes[this->_boxIndex - 1] :
	       data.hitBoxes[this->_boxIndex - 1 - data.hurtBoxes.size()];
}

SpiralOfFate::RemoveBoxOperation::RemoveBoxOperation(EditableObject &obj, const std::string &&name, unsigned boxIndex, unsigned &boxIndexPtr) :
	_obj(obj),
	_action(obj._action),
	_actionBlock(obj._actionBlock),
	_animation(obj._animation),
	_boxIndex(boxIndex),
	_boxIndexPtr(boxIndexPtr),
	_oldValue(this->_getBox()),
	_fieldName(name)
{
	auto &data = this->_obj.getFrameData();

	this->_isCollisionBox = this->_boxIndex == data.hurtBoxes.size() + data.hitBoxes.size() + 1;
	this->_isHurtBox = !this->_isCollisionBox && this->_boxIndex <= data.hurtBoxes.size();
	this->_boxIndex--;
	if (!this->_isHurtBox && !this->_isCollisionBox)
		this->_boxIndex -= data.hurtBoxes.size();
}

void SpiralOfFate::RemoveBoxOperation::apply()
{
	this->_obj._action = this->_action;
	this->_obj._actionBlock = this->_actionBlock;
	this->_obj._animation = this->_animation;

	auto &data = this->_obj.getFrameData();

	if (this->_isCollisionBox) {
		delete data.collisionBox;
		data.collisionBox = nullptr;
	} else if (this->_isHurtBox)
		data.hurtBoxes.erase(data.hurtBoxes.begin() + this->_boxIndex );
	else
		data.hitBoxes.erase(data.hitBoxes.begin() + this->_boxIndex);
	this->_boxIndexPtr = 0;
}

void SpiralOfFate::RemoveBoxOperation::undo()
{
	this->_obj._action = this->_action;
	this->_obj._actionBlock = this->_actionBlock;
	this->_obj._animation = this->_animation;

	auto &data = this->_obj.getFrameData();

	if (this->_isCollisionBox)
		data.collisionBox = new Box{this->_oldValue};
	else if (this->_isHurtBox)
		data.hurtBoxes.insert(data.hurtBoxes.begin() + this->_boxIndex , this->_oldValue);
	else
		data.hitBoxes.insert(data.hitBoxes.begin() + this->_boxIndex, this->_oldValue);
	this->_boxIndexPtr = 0;
}

std::string SpiralOfFate::RemoveBoxOperation::getName() const noexcept
{
	return this->_fieldName;
}

bool SpiralOfFate::RemoveBoxOperation::hasModification() const
{
	return true;
}

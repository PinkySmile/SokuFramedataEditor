//
// Created by PinkySmile on 11/05/25.
//

#include "EditSoundOperation.hpp"

SpiralOfFate::EditSoundOperation::EditSoundOperation(
	EditableObject &obj,
	const std::string &&name,
	std::string FrameData::*field,
	const std::string &newValue,
	bool reset
) :
	BasicDataOperation<std::string>(obj, std::move(name), field, newValue, reset)
{
}

void SpiralOfFate::EditSoundOperation::apply()
{
	BasicDataOperation::apply();

	auto &data = this->_obj.getFrameData();

	data.reloadSound();
}

void SpiralOfFate::EditSoundOperation::undo()
{
	BasicDataOperation::undo();

	auto &data = this->_obj.getFrameData();

	data.reloadSound();
}
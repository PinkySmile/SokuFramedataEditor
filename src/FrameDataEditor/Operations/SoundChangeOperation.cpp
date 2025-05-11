//
// Created by PinkySmile on 11/05/25.
//

#include "SoundChangeOperation.hpp"

SpiralOfFate::SoundChangeOperation::SoundChangeOperation(
	EditableObject &obj, const std::string &&name, std::string FrameData::*field, const std::string &newValue
) :
	BasicDataOperation<std::string>(obj, std::move(name), field, newValue)
{
}

void SpiralOfFate::SoundChangeOperation::apply()
{
	BasicDataOperation::apply();

	auto &data = this->_obj.getFrameData();

	data.reloadSound();
}

void SpiralOfFate::SoundChangeOperation::undo()
{
	BasicDataOperation::undo();

	auto &data = this->_obj.getFrameData();

	data.reloadSound();
}
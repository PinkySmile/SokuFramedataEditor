//
// Created by PinkySmile on 10/05/25.
//

#include "EditSpriteOperation.hpp"

SpiralOfFate::EditSpriteOperation::EditSpriteOperation(
	EditableObject &obj,
	const std::string &&name,
	std::string FrameData::*field,
	const std::string &newValue,
	bool reset
) :
	BasicDataOperation<std::string, FrameData>(obj, std::move(name), field, newValue, reset)
{
}

void SpiralOfFate::EditSpriteOperation::apply()
{
	BasicDataOperation::apply();

	auto &data = this->_obj.getFrameData();

	data.reloadTexture();
}

void SpiralOfFate::EditSpriteOperation::undo()
{
	BasicDataOperation::undo();

	auto &data = this->_obj.getFrameData();

	data.reloadTexture();
}
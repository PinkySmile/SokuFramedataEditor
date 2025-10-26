//
// Created by PinkySmile on 10/05/25.
//

#include "SpriteChangeOperation.hpp"

SpiralOfFate::SpriteChangeOperation::SpriteChangeOperation(
	EditableObject &obj,
	const std::string &&name,
	std::string FrameData::*field,
	const std::string &newValue,
	bool reset
) :
	BasicDataOperation<std::string>(obj, std::move(name), field, newValue, reset)
{
}

void SpiralOfFate::SpriteChangeOperation::apply()
{
	BasicDataOperation::apply();

	auto &data = this->_obj.getFrameData();

	data.reloadTexture();
}

void SpiralOfFate::SpriteChangeOperation::undo()
{
	BasicDataOperation::undo();

	auto &data = this->_obj.getFrameData();

	data.reloadTexture();
}
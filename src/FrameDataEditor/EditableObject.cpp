//
// Created by PinkySmile on 18/09/2021
//

#include "EditableObject.hpp"

EditableObject::EditableObject() :
	OverlayObject(this->_moves)
{
}

EditableObject::EditableObject(const std::string &chr, const ShadyCore::Schema &schema, const ShadyCore::Palette &palette, const std::string &palName) :
	EditableObject()
{
	this->_moves = SpiralOfFate::FrameData::loadSchema(chr, schema, palette, palName);
	this->_action = this->_moves.begin()->first;
}

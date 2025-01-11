//
// Created by PinkySmile on 18/09/2021
//

#ifndef BATTLE_EditableObject_HPP
#define BATTLE_EditableObject_HPP


#include <SoFGV.hpp>
#include "OverlayObject.hpp"

class EditableObject : public OverlayObject {
public:
	mutable std::map<unsigned, std::vector<std::vector<SpiralOfFate::FrameData>>> _moves;

	EditableObject();
	EditableObject(const std::string &chr, const ShadyCore::Schema &schema, const ShadyCore::Palette &palette, const std::string &palName);
};


#endif //BATTLE_EditableObject_HPP

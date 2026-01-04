//
// Created by PinkySmile on 10/05/25.
//

#ifndef SOFGV_SPRITECHANGEOPERATION_HPP
#define SOFGV_SPRITECHANGEOPERATION_HPP


#include "BasicDataOperation.hpp"

namespace SpiralOfFate
{
	class EditSpriteOperation : public BasicDataOperation<std::string> {
	public:
		EditSpriteOperation(EditableObject &obj, const std::string &&name, std::string FrameData::*field, const std::string &newValue, bool reset);

		void apply() override;
		void undo() override;
	};
}


#endif //SOFGV_SPRITECHANGEOPERATION_HPP

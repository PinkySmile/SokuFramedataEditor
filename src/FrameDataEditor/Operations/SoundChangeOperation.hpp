//
// Created by PinkySmile on 11/05/25.
//

#ifndef SOFGV_SOUNDCHANGEOPERATION_HPP
#define SOFGV_SOUNDCHANGEOPERATION_HPP


#include "BasicDataOperation.hpp"

namespace SpiralOfFate
{
	class SoundChangeOperation : public BasicDataOperation<std::string> {
	public:
		SoundChangeOperation(EditableObject &obj, const std::string &&name, std::string FrameData::*field, const std::string &newValue, bool reset);

		void apply() override;
		void undo() override;
	};
}


#endif //SOFGV_SOUNDCHANGEOPERATION_HPP

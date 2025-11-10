//
// Created by PinkySmile on 10/11/2025.
//

#ifndef SOFGV_PASTEANIMDATAOPERATION_HPP
#define SOFGV_PASTEANIMDATAOPERATION_HPP


#include "Operation.hpp"
#include "../EditableObject.hpp"

namespace SpiralOfFate
{
	class PasteAnimDataOperation : public Operation {
	protected:
		EditableObject &_obj;
		unsigned _action;
		unsigned _blockId;
		unsigned _id;
		FrameData _oldValue;
		FrameData _newValue;
		std::string _fieldName;

	public:
		PasteAnimDataOperation(
			EditableObject &obj,
			const std::string &&name,
			FrameData newValue
		);
		void apply() override;
		void undo() override;
		std::string getName() const noexcept override;
		bool hasModification() const override;
	};
}


#endif //SOFGV_PASTEANIMDATAOPERATION_HPP

//
// Created by PinkySmile on 10/11/2025.
//

#ifndef SOFGV_PASTEBOXDATAOPERATION_HPP
#define SOFGV_PASTEBOXDATAOPERATION_HPP


#include "Operation.hpp"
#include "../EditableObject.hpp"

namespace SpiralOfFate
{
	class PasteBoxDataOperation : public Operation {
	protected:
		EditableObject &_obj;
		unsigned _action;
		unsigned _blockId;
		unsigned _id;
		FrameData _oldValue;
		FrameData _newValue;
		std::string _fieldName;

	public:
		PasteBoxDataOperation(
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


#endif //SOFGV_PASTEBOXDATAOPERATION_HPP

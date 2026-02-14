//
// Created by PinkySmile on 10/11/2025.
//

#ifndef SOFGV_PASTEDATAOPERATION_HPP
#define SOFGV_PASTEDATAOPERATION_HPP


#include "Operation.hpp"
#include "../EditableObject.hpp"

namespace SpiralOfFate
{
	class PasteDataOperation : public Operation {
	protected:
		EditableObject &_obj;
		unsigned _action;
		unsigned _blockId;
		unsigned _id;
		FrameData _oldValue;
		FrameData _newValue;
		std::string _fieldName;

	public:
		PasteDataOperation(
			EditableObject &obj,
			const std::string &&name,
			FrameData newValue
		);
		void apply() override;
		void undo() override;
		[[nodiscard]] std::string getName() const noexcept override;
		[[nodiscard]] bool hasModification() const override;
		[[nodiscard]] bool hasFramedataModification() const override;
	};
}


#endif //SOFGV_PASTEDATAOPERATION_HPP

//
// Created by PinkySmile on 25/05/25.
//

#ifndef SOFGV_REMOVEFRAMEOPERATION_HPP
#define SOFGV_REMOVEFRAMEOPERATION_HPP


#include "Operation.hpp"
#include "../EditableObject.hpp"

namespace SpiralOfFate
{
	class RemoveFrameOperation : public Operation {
	protected:
		EditableObject &_obj;
		unsigned _action;
		unsigned _actionBlock;
		unsigned _animation;
		FrameData _oldValue;
		std::string _fieldName;

	public:
		RemoveFrameOperation(EditableObject &obj, const std::string &&name);
		void apply() override;
		void undo() override;
		std::string getName() const noexcept override;
		bool hasModification() const override;
	};
}


#endif //SOFGV_REMOVEFRAMEOPERATION_HPP

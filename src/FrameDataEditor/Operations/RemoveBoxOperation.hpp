//
// Created by PinkySmile on 04/07/25.
//

#ifndef SOFGV_REMOVEBOXOPERATION_HPP
#define SOFGV_REMOVEBOXOPERATION_HPP


#include "Operation.hpp"
#include "../EditableObject.hpp"

namespace SpiralOfFate
{
	class RemoveBoxOperation : public Operation {
	protected:
		EditableObject &_obj;
		unsigned _action;
		unsigned _actionBlock;
		unsigned _animation;
		unsigned _boxIndex;
		bool _isCollisionBox;
		bool _isHurtBox;
		unsigned &_boxIndexPtr;
		Box _oldValue;
		std::string _fieldName;

		Box &_getBox();

	public:
		RemoveBoxOperation(EditableObject &obj, const std::string &&name, unsigned boxIndex, unsigned &boxIndexPtr);
		void apply() override;
		void undo() override;
		std::string getName() const noexcept override;
		bool hasModification() const override;
	};
}


#endif //SOFGV_REMOVEBOXOPERATION_HPP

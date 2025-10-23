//
// Created by PinkySmile on 23/05/25.
//

#ifndef SOFGV_BOXMODIFICATIONOPERATION_HPP
#define SOFGV_BOXMODIFICATIONOPERATION_HPP


#include "Operation.hpp"
#include "../EditableObject.hpp"

namespace SpiralOfFate
{
	enum BoxType {
		BOXTYPE_NONE,
		BOXTYPE_HITBOX,
		BOXTYPE_HURTBOX,
		BOXTYPE_COLLISIONBOX,
	};

	class BoxModificationOperation : public Operation {
	protected:
		EditableObject &_obj;
		unsigned _action;
		unsigned _actionBlock;
		unsigned _animation;
		unsigned _boxIndex;
		BoxType _type;
		Box _oldValue;
		Box _newValue;
		std::string _fieldName;

		Box &_getBox();

	public:
		BoxModificationOperation(EditableObject &obj, const std::string &&name, BoxType type, unsigned boxIndex, Box newValue);
		void apply() override;
		void undo() override;
		std::string getName() const noexcept override;
		bool hasModification() const override;
	};
}


#endif //SOFGV_BOXMODIFICATIONOPERATION_HPP

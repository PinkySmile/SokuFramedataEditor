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

	class EditBoxOperation : public Operation {
	protected:
		EditableObject &_obj;
		unsigned _action;
		unsigned _actionBlock;
		unsigned _animation;
		unsigned _boxIndex;
		BoxType _type;
		ShadyCore::Schema::Sequence::BBox _oldValue;
		ShadyCore::Schema::Sequence::BBox _newValue;
		std::string _fieldName;

		ShadyCore::Schema::Sequence::BBox &_getBox();

	public:
		EditBoxOperation(EditableObject &obj, const std::string &&name, BoxType type, unsigned boxIndex, ShadyCore::Schema::Sequence::BBox newValue);
		void apply() override;
		void undo() override;
		[[nodiscard]] std::string getName() const noexcept override;
		[[nodiscard]] bool hasModification() const override;
		[[nodiscard]] bool hasFramedataModification() const override;
	};
}


#endif //SOFGV_BOXMODIFICATIONOPERATION_HPP

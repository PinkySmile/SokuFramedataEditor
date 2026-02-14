//
// Created by PinkySmile on 11/05/25.
//

#ifndef SOFGV_CLEARHITOPERATION_HPP
#define SOFGV_CLEARHITOPERATION_HPP


#include "../FrameDataEditor.hpp"
#include "Operation.hpp"

namespace SpiralOfFate
{
	class ClearAttackOperation : public Operation {
	private:
		EditableObject &_obj;
		unsigned _action;
		unsigned _actionBlock;
		unsigned _animation;
		ShadyCore::Schema::Sequence::MoveTraits oldTraits;
		std::string _name;

	public:
		ClearAttackOperation(EditableObject &obj, const FrameDataEditor &editor);
		void apply() override;
		void undo() override;
		[[nodiscard]] std::string getName() const noexcept override;
		[[nodiscard]] bool hasModification() const override;
		[[nodiscard]] bool hasFramedataModification() const override;
	};
}


#endif //SOFGV_CLEARHITOPERATION_HPP

//
// Created by PinkySmile on 07/01/2026.
//

#ifndef SOFGV_FLATTENPROPRETIESOPERATION_HPP
#define SOFGV_FLATTENPROPRETIESOPERATION_HPP


#include "Operation.hpp"
#include "../EditableObject.hpp"

namespace SpiralOfFate
{
	class FlattenPropretiesOperation : public Operation {
	protected:
		EditableObject &_obj;
		unsigned _action;
		std::vector<std::vector<ShadyCore::Schema::Sequence::MoveTraits>> _oldValues;
		ShadyCore::Schema::Sequence::MoveTraits _newValue;
		std::string _fieldName;

		static void _makeCopy(ShadyCore::Schema::Sequence::MoveTraits &dst, const FrameData &src);

	public:
		FlattenPropretiesOperation(EditableObject &obj, const std::string &&name, const FrameData &newValue);
		void apply() override;
		void undo() override;
		[[nodiscard]] std::string getName() const noexcept override;
		[[nodiscard]] bool hasModification() const override;
		[[nodiscard]] bool hasFramedataModification() const override;
	};
} // SpiralOfFate


#endif //SOFGV_FLATTENPROPRETIESOPERATION_HPP
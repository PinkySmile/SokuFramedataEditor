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
		std::vector<std::vector<FrameData>> _oldValues;
		FrameData _newValue;
		std::string _fieldName;

		static void _makeCopy(FrameData &dst, const FrameData &src);
		static void _restoreCopy(FrameData &dst, const FrameData &src);

	public:
		FlattenPropretiesOperation(EditableObject &obj, const std::string &&name, FrameData newValue);
		void apply() override;
		void undo() override;
		std::string getName() const noexcept override;
		bool hasModification() const override;
	};
} // SpiralOfFate


#endif //SOFGV_FLATTENPROPRETIESOPERATION_HPP
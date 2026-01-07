//
// Created by PinkySmile on 07/01/2026.
//

#ifndef SOFGV_CLEANPROPERTIESOPERATION_HPP
#define SOFGV_CLEANPROPERTIESOPERATION_HPP


#include "Operation.hpp"
#include "../EditableObject.hpp"

namespace SpiralOfFate
{
	class CleanPropertiesOperation : public Operation {
	protected:
		EditableObject &_obj;
		std::map<unsigned, std::vector<std::vector<FrameData>>> _oldValues;
		std::string _fieldName;

		static void _clean(FrameData &data, TypeColor type);

	public:
		CleanPropertiesOperation(EditableObject &obj, const std::string &&name);
		void apply() override;
		void undo() override;
		std::string getName() const noexcept override;
		bool hasModification() const override;
	};
} // SpiralOfFate


#endif //SOFGV_CLEANPROPERTIESOPERATION_HPP
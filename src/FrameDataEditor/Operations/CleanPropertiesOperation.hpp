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
		std::map<unsigned, std::vector<std::vector<ShadyCore::Schema::Sequence::MoveTraits>>> _oldTraits;
		std::string _fieldName;

		static void _clean(FrameData &data);

	public:
		CleanPropertiesOperation(EditableObject &obj, const std::string &&name);
		void apply() override;
		void undo() override;
		[[nodiscard]] std::string getName() const noexcept override;
		[[nodiscard]] bool hasModification() const override;
		[[nodiscard]] bool hasFramedataModification() const override;
	};
} // SpiralOfFate


#endif //SOFGV_CLEANPROPERTIESOPERATION_HPP
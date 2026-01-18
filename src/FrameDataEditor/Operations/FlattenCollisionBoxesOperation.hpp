//
// Created by PinkySmile on 06/01/2026.
//

#ifndef SOFGV_FLATTENCOLLISIONBOXESOPERATION_HPP
#define SOFGV_FLATTENCOLLISIONBOXESOPERATION_HPP


#include "Operation.hpp"
#include "../EditableObject.hpp"

namespace SpiralOfFate
{
	class FlattenCollisionBoxesOperation : public Operation {
	protected:
		EditableObject &_obj;
		unsigned _action;
		std::vector<std::vector<std::optional<ShadyCore::Schema::Sequence::BBox>>> _oldValues;
		std::optional<ShadyCore::Schema::Sequence::BBox> _newValue;
		std::string _fieldName;

	public:
		FlattenCollisionBoxesOperation(EditableObject &obj, const std::string &&name, std::optional<ShadyCore::Schema::Sequence::BBox> newValue);
		void apply() override;
		void undo() override;
		std::string getName() const noexcept override;
		bool hasModification() const override;
	};
} // SpiralOfFate


#endif //SOFGV_FLATTENCOLLISIONBOXESOPERATION_HPP
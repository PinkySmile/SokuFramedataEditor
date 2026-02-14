//
// Created by PinkySmile on 24/05/25.
//

#ifndef SOFGV_CREATEMOVEOPERATION_HPP
#define SOFGV_CREATEMOVEOPERATION_HPP


#include "Operation.hpp"
#include "../EditableObject.hpp"

namespace SpiralOfFate
{
	class CreateMoveOperation : public Operation {
	private:
		EditableObject &_obj;
		std::string _name;
		unsigned _id;
		FrameData::Action _newData;

	public:
		CreateMoveOperation(EditableObject &obj, const std::string &&name, unsigned id, const FrameData::Action &newData);
		void apply() override;
		void undo() override;
		[[nodiscard]] bool hasModification() const override;
		[[nodiscard]] std::string getName() const noexcept override;
		[[nodiscard]] bool hasFramedataModification() const override;
	};
}


#endif //SOFGV_CREATEMOVEOPERATION_HPP

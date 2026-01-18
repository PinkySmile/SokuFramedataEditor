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
		bool hasModification() const override;
		std::string getName() const noexcept override;
	};
}


#endif //SOFGV_CREATEMOVEOPERATION_HPP

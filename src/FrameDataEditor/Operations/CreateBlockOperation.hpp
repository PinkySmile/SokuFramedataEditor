//
// Created by PinkySmile on 24/05/25.
//

#ifndef SOFGV_CREATEBLOCKOPERATION_HPP
#define SOFGV_CREATEBLOCKOPERATION_HPP


#include "Operation.hpp"
#include "../EditableObject.hpp"

namespace SpiralOfFate
{
	class CreateBlockOperation : public Operation {
	private:
		EditableObject &_obj;
		std::string _name;
		unsigned _action;
		unsigned _id;
		FrameData::Sequence _newData;

	public:
		CreateBlockOperation(EditableObject &obj, const std::string &&name, unsigned id, const FrameData::Sequence &newData);
		void apply() override;
		void undo() override;
		bool hasModification() const override;
		std::string getName() const noexcept override;
	};
}


#endif //SOFGV_CREATEBLOCKOPERATION_HPP

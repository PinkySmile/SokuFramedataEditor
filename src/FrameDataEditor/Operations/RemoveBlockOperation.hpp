//
// Created by PinkySmile on 30/12/2025.
//

#ifndef SOFGV_REMOVEBLOCKOPERATION_HPP
#define SOFGV_REMOVEBLOCKOPERATION_HPP


#include "Operation.hpp"
#include "../EditableObject.hpp"

namespace SpiralOfFate
{
	class RemoveBlockOperation : public Operation {
	protected:
		EditableObject &_obj;
		std::string _name;
		unsigned _action;
		unsigned _id;
		FrameData::Sequence _oldData;

	public:
		RemoveBlockOperation(EditableObject &obj, const std::string &&name, unsigned id);
		void apply() override;
		void undo() override;
		[[nodiscard]] std::string getName() const noexcept override;
		[[nodiscard]] bool hasModification() const override;
		[[nodiscard]] bool hasFramedataModification() const override;
	};
}


#endif //SOFGV_REMOVEBLOCKOPERATION_HPP
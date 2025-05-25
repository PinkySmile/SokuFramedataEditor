//
// Created by PinkySmile on 24/05/25.
//

#ifndef SOFGV_CREATEFRAMEOPERATION_HPP
#define SOFGV_CREATEFRAMEOPERATION_HPP


#include "Operation.hpp"
#include "../EditableObject.hpp"

namespace SpiralOfFate
{
	class CreateFrameOperation : public Operation {
	private:
		EditableObject &_obj;
		std::string _name;
		unsigned _action;
		unsigned _block;
		unsigned _id;
		FrameData _newData;

	public:
		CreateFrameOperation(EditableObject &obj, const std::string &&name, unsigned id, const FrameData &newData);
		void apply() override;
		void undo() override;
		bool hasModification() const override;
		std::string getName() const noexcept override;
	};
}


#endif //SOFGV_CREATEFRAMEOPERATION_HPP

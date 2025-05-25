//
// Created by PinkySmile on 25/05/25.
//

#ifndef SOFGV_CREATEBOXOPERATION_HPP
#define SOFGV_CREATEBOXOPERATION_HPP


#include "Operation.hpp"
#include "../EditableObject.hpp"

namespace SpiralOfFate
{
	class CreateBoxOperation : public Operation {
	protected:
		EditableObject &_obj;
		unsigned _action;
		unsigned _actionBlock;
		unsigned _animation;
		bool _hurtbox;
		std::string _fieldName;

	public:
		CreateBoxOperation(EditableObject &obj, const std::string &&name, bool hurtbox);
		void apply() override;
		void undo() override;
		std::string getName() const noexcept override;
		bool hasModification() const override;
	};
}


#endif //SOFGV_CREATEBOXOPERATION_HPP

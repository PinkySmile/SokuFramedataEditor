//
// Created by PinkySmile on 25/05/25.
//

#ifndef SOFGV_CREATEBOXOPERATION_HPP
#define SOFGV_CREATEBOXOPERATION_HPP


#include "EditBoxOperation.hpp"
#include "Operation.hpp"
#include "../EditableObject.hpp"

namespace SpiralOfFate
{
	class PreviewWidget;

	class CreateBoxOperation : public Operation {
	protected:
		EditableObject &_obj;
		PreviewWidget &_preview;
		unsigned _action;
		unsigned _actionBlock;
		unsigned _animation;
		BoxType _type;
		std::string _fieldName;

	public:
		CreateBoxOperation(EditableObject &obj, const std::string &&name, BoxType type, PreviewWidget &preview);
		void apply() override;
		void undo() override;
		[[nodiscard]] std::string getName() const noexcept override;
		[[nodiscard]] bool hasModification() const override;
		[[nodiscard]] bool hasFramedataModification() const override;
	};
}


#endif //SOFGV_CREATEBOXOPERATION_HPP

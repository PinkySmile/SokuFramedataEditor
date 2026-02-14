//
// Created by PinkySmile on 04/07/25.
//

#ifndef SOFGV_REMOVEBOXOPERATION_HPP
#define SOFGV_REMOVEBOXOPERATION_HPP


#include <functional>
#include "EditBoxOperation.hpp"
#include "Operation.hpp"
#include "../EditableObject.hpp"

namespace SpiralOfFate
{
	typedef std::function<void(BoxType type, unsigned boxIndex)> RemoveBoxApply;
	class RemoveBoxOperation : public Operation {
	protected:
		EditableObject &_obj;
		RemoveBoxApply _onApply;
		unsigned _action;
		unsigned _actionBlock;
		unsigned _animation;
		unsigned _boxIndex;
		BoxType _type;
		ShadyCore::Schema::Sequence::BBox _oldValue;
		std::string _fieldName;

		ShadyCore::Schema::Sequence::BBox &_getBox();

	public:
		RemoveBoxOperation(EditableObject &obj, const std::string &&name, BoxType type, unsigned boxIndex, RemoveBoxApply onApply);
		void apply() override;
		void undo() override;
		[[nodiscard]] std::string getName() const noexcept override;
		[[nodiscard]] bool hasModification() const override;
		[[nodiscard]] bool hasFramedataModification() const override;
	};
}


#endif //SOFGV_REMOVEBOXOPERATION_HPP

//
// Created by PinkySmile on 09/05/25.
//

#ifndef SOFGV_BASICDATAOPERATION_SEQ_HPP
#define SOFGV_BASICDATAOPERATION_SEQ_HPP


#include "Operation.hpp"
#include "../EditableObject.hpp"

namespace SpiralOfFate
{
	template<typename T, typename T2>
	class BasicDataOperationSeq : public Operation {
	protected:
		EditableObject &_obj;
		unsigned _action;
		unsigned _actionBlock;
		unsigned _animation;
		T FrameData::Sequence::*_field;
		T _oldValue;
		T _newValue;
		std::string _fieldName;
		bool _reset;

	public:
		BasicDataOperationSeq(EditableObject &obj, const std::string &&name, T T2::*field, T newValue, bool reset) :
			_obj(obj),
			_action(obj._action),
			_actionBlock(obj._actionBlock),
			_animation(obj._animation),
			_field(field),
			_oldValue(obj.getSequence().*field),
			_newValue(newValue),
			_fieldName(name),
			_reset(reset)
		{
		}

		void apply() override
		{
			bool shouldReset =
				this->_obj._action != this->_action ||
				this->_obj._actionBlock != this->_actionBlock ||
				this->_obj._animation != this->_animation;

			this->_obj._action = this->_action;
			this->_obj._actionBlock = this->_actionBlock;
			this->_obj._animation = this->_animation;
			this->_obj.getSequence().*_field = this->_newValue;
			if (shouldReset || this->_reset) {
				if (shouldReset)
					this->_obj._animationCtr = 0;
				this->_obj.resetState();
			}
		}

		void undo() override
		{
			bool shouldReset =
				this->_obj._action != this->_action ||
				this->_obj._actionBlock != this->_actionBlock ||
				this->_obj._animation != this->_animation;

			this->_obj._action = this->_action;
			this->_obj._actionBlock = this->_actionBlock;
			this->_obj._animation = this->_animation;
			this->_obj.getSequence().*_field = this->_oldValue;
			if (shouldReset || this->_reset) {
				if (shouldReset)
					this->_obj._animationCtr = 0;
				this->_obj.resetState();
			}
		}

		[[nodiscard]] std::string getName() const noexcept override
		{
			return this->_fieldName;
		}

		[[nodiscard]] bool hasFramedataModification() const override
		{
			return this->hasModification();
		}

		[[nodiscard]] bool hasModification() const override
		{
			return this->_oldValue != this->_newValue;
		}
	};
}


#endif //SOFGV_BASICDATAOPERATION_SEQ_HPP

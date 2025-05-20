//
// Created by PinkySmile on 10/05/25.
//

#ifndef SOFGV_FLAGOPERATION_HPP
#define SOFGV_FLAGOPERATION_HPP


#include "IOperation.hpp"
#include "../EditableObject.hpp"

namespace SpiralOfFate
{
	template<typename T>
	class FlagOperation : public IOperation {
	protected:
		EditableObject &_obj;
		unsigned _action;
		unsigned _actionBlock;
		unsigned _animation;
		T FrameData::*_field;
		bool _oldValue;
		bool _newValue;
		std::string _fieldName;
		size_t _index;
		bool _reset;

	public:
		FlagOperation(EditableObject &obj, const std::string &&name, T FrameData::*field, size_t index, bool newValue, bool reset) :
			_obj(obj),
			_action(obj._action),
			_actionBlock(obj._actionBlock),
			_animation(obj._animation),
			_field(field),
			_oldValue((obj.getFrameData().*field).flags & (1ULL << index)),
			_newValue(newValue),
			_fieldName(name),
			_index(index),
			_reset(reset)
		{
		}

		void apply() override
		{
			bool shouldReset = this->_reset ||
				this->_obj._action != this->_action ||
				this->_obj._actionBlock != this->_actionBlock ||
				this->_obj._animation != this->_animation;

			this->_obj._action = this->_action;
			this->_obj._actionBlock = this->_actionBlock;
			this->_obj._animation = this->_animation;

			unsigned long long &flags = (this->_obj.getFrameData().*_field).flags;

			if (this->_newValue)
				flags |= 1ULL << this->_index;
			else
				flags &= ~(1ULL << this->_index);
			if (shouldReset) {
				this->_obj._animationCtr = 0;
				this->_obj.resetState();
			}
		}

		void undo() override
		{
			bool shouldReset = this->_reset ||
				this->_obj._action != this->_action ||
				this->_obj._actionBlock != this->_actionBlock ||
				this->_obj._animation != this->_animation;

			this->_obj._action = this->_action;
			this->_obj._actionBlock = this->_actionBlock;
			this->_obj._animation = this->_animation;

			unsigned long long &flags = (this->_obj.getFrameData().*_field).flags;

			if (this->_oldValue)
				flags |= 1ULL << this->_index;
			else
				flags &= ~(1ULL << this->_index);
			if (shouldReset) {
				this->_obj._animationCtr = 0;
				this->_obj.resetState();
			}
		}

		std::string getName() const noexcept override
		{
			return this->_fieldName;
		}

		bool hasModification() const override
		{
			return this->_newValue != this->_oldValue;
		}
	};
}


#endif //SOFGV_FLAGOPERATION_HPP

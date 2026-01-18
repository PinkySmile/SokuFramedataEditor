//
// Created by PinkySmile on 09/05/25.
//

#ifndef SOFGV_BASICDATADOUBLEOPERATION_HPP
#define SOFGV_BASICDATADOUBLEOPERATION_HPP


#include "Operation.hpp"
#include "../EditableObject.hpp"

namespace SpiralOfFate
{
	template<typename T, typename T2>
	class BasicDataDoubleOperation : public Operation {
	protected:
		EditableObject &_obj;
		unsigned _action;
		unsigned _actionBlock;
		unsigned _animation;
		T2 FrameData::*_field1;
		T2 FrameData::*_field2;
		T2 _oldValue1;
		T2 _oldValue2;
		T2 _newValue1;
		T2 _newValue2;
		std::string _fieldName;
		bool _reset;

	public:
		BasicDataDoubleOperation(EditableObject &obj, const std::string &&name, T2 T::*field1, T2 T::*field2, T2 newValue1, T2 newValue2, bool reset) :
			_obj(obj),
			_action(obj._action),
			_actionBlock(obj._actionBlock),
			_animation(obj._animation),
			_field1(field1),
			_field2(field2),
			_oldValue1(obj.getFrameData().*field1),
			_oldValue2(obj.getFrameData().*field2),
			_newValue1(newValue1),
			_newValue2(newValue2),
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
			this->_obj.getFrameData().*this->_field1 = this->_newValue1;
			this->_obj.getFrameData().*this->_field2 = this->_newValue2;
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
			this->_obj.getFrameData().*this->_field1 = this->_oldValue1;
			this->_obj.getFrameData().*this->_field2 = this->_oldValue2;
			if (shouldReset || this->_reset) {
				if (shouldReset)
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
			return this->_oldValue1 != this->_newValue1 || this->_oldValue2 != this->_newValue2;
		}
	};
}


#endif //SOFGV_BASICDATADOUBLEOPERATION_HPP

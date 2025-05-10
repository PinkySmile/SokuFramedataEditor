//
// Created by PinkySmile on 09/05/25.
//

#ifndef SOFGV_BASICDATAOPERATION_HPP
#define SOFGV_BASICDATAOPERATION_HPP


#include "IOperation.hpp"
#include "../EditableObject.hpp"

namespace SpiralOfFate
{
	template<typename T>
	class BasicDataOperation : public IOperation {
	private:
		EditableObject &_obj;
		unsigned _action;
		unsigned _actionBlock;
		unsigned _animation;
		T FrameData::*_field;
		T _oldValue;
		T _newValue;
		std::string _fieldName;

	public:
		BasicDataOperation(EditableObject &obj, const std::string &&name, T FrameData::*field, T newValue) :
			_obj(obj),
			_action(obj._action),
			_actionBlock(obj._actionBlock),
			_animation(obj._animation),
			_field(field),
			_oldValue(obj.getFrameData().*field),
			_newValue(newValue),
			_fieldName(name)
		{
		}

		void apply() override
		{
			this->_obj._action = this->_action;
			this->_obj._actionBlock = this->_actionBlock;
			this->_obj._animation = this->_animation;
			this->_obj.getFrameData().*_field = this->_newValue;
		}

		void undo() override
		{
			this->_obj._action = this->_action;
			this->_obj._actionBlock = this->_actionBlock;
			this->_obj._animation = this->_animation;
			this->_obj.getFrameData().*_field = this->_oldValue;
		}

		std::string getName() const noexcept override
		{
			return this->_fieldName;
		}
	};
}


#endif //SOFGV_BASICDATAOPERATION_HPP

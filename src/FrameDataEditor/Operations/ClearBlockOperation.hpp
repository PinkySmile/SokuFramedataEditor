//
// Created by PinkySmile on 11/05/25.
//

#ifndef SOFGV_CLEARBLOCKOPERATION_HPP
#define SOFGV_CLEARBLOCKOPERATION_HPP


#include "../FrameDataEditor.hpp"
#include "IOperation.hpp"

namespace SpiralOfFate
{
	class ClearBlockOperation : public IOperation {
	private:
		EditableObject &_obj;
		unsigned _action;
		unsigned _actionBlock;
		unsigned _animation;
		unsigned _oldBlockPlayerHitStop;
		unsigned _oldBlockOpponentHitStop;
		int _oldPushBack;
		int _oldPushBlock;
		unsigned _oldGuardDmg;
		unsigned _oldChipDamage;
		unsigned _oldBlockStun;
		unsigned _oldWrongBlockStun;
		std::string _name;

	public:
		ClearBlockOperation(EditableObject &obj, const FrameDataEditor &editor);

		void apply() override;
		void undo() override;
		std::string getName() const noexcept override;
		bool hasModification() const override;
	};
}


#endif //SOFGV_CLEARBLOCKOPERATION_HPP

//
// Created by PinkySmile on 11/05/25.
//

#ifndef SOFGV_CLEARHITOPERATION_HPP
#define SOFGV_CLEARHITOPERATION_HPP


#include "../FrameDataEditor.hpp"
#include "Operation.hpp"

namespace SpiralOfFate
{
	class ClearHitOperation : public Operation {
	private:
		EditableObject &_obj;
		unsigned _action;
		unsigned _actionBlock;
		unsigned _animation;
		unsigned _oldHitPlayerHitStop;
		unsigned _oldHitOpponentHitStop;
		Vector2f _oldHitSpeed;
		Vector2f _oldCounterHitSpeed;
		unsigned _oldNeutralLimit;
		unsigned _oldSpiritLimit;
		unsigned _oldMatterLimit;
		unsigned _oldVoidLimit;
		float _oldProrate;
		float _oldMinProrate;
		unsigned _oldUntech;
		unsigned _oldDamage;
		unsigned _oldHitStun;
		std::string _oldHitSoundPath;
		std::optional<std::pair<Vector2f, float>> _oldSnap;
		std::string _name;

	public:
		ClearHitOperation(EditableObject &obj, const FrameDataEditor &editor);

		void apply() override;
		void undo() override;
		std::string getName() const noexcept override;
		bool hasModification() const override;
	};
}


#endif //SOFGV_CLEARHITOPERATION_HPP

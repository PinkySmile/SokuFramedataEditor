//
// Created by PinkySmile on 07/01/2026.
//

#include "FlattenPropretiesOperation.hpp"

namespace SpiralOfFate
{
	FlattenPropretiesOperation::FlattenPropretiesOperation(
		EditableObject &obj,
		const std::string &&name,
		const FrameData &newValue
	) :
		_obj(obj),
		_action(obj._action),
		_fieldName(name)
	{
		auto &act = obj._schema.framedata.at(obj._action);

		FlattenPropretiesOperation::_makeCopy(this->_newValue, newValue);

		this->_oldValues.reserve(act.size());
		for (auto &blk : act) {
			this->_oldValues.emplace_back();

			auto &vec = this->_oldValues.back();

			vec.reserve(blk.size());
			for (auto &f : blk) {
				vec.emplace_back();
				FlattenPropretiesOperation::_makeCopy(vec.back(), f);
			}
		}
	}

	void FlattenPropretiesOperation::_makeCopy(ShadyCore::Schema::Sequence::MoveTraits &dst, const FrameData &src)
	{
		dst = src.traits;
		if (!src.aBoxes.empty())
			return;

		dst.damage = 0;
		dst.proration = 0;
		dst.chipDamage = 0;
		dst.spiritDamage = 0;
		dst.untech = 0;
		dst.power = 0;
		dst.limit = 0;
		dst.onHitPlayerStun = 0;
		dst.onHitEnemyStun = 0;
		dst.onBlockPlayerStun = 0;
		dst.onBlockEnemyStun = 0;
		dst.onHitCardGain = 0;
		dst.onBlockCardGain = 0;
		dst.onAirHitSetSequence = 0;
		dst.onGroundHitSetSequence = 0;
		dst.speedX = 0;
		dst.speedY = 0;
		dst.onHitSfx = 0;
		dst.onHitEffect = 0;
		dst.attackLevel = 0;
		dst.comboModifier = 0;
		dst.attackFlags = 0;
	}

	void FlattenPropretiesOperation::apply()
	{
		auto &act = this->_obj._schema.framedata.at(this->_action);

		for (auto &blk : act)
			for (auto &f : blk)
				f.traits = this->_newValue;
	}

	void FlattenPropretiesOperation::undo()
	{
		auto &act = this->_obj._schema.framedata.at(this->_action);

		for (size_t blk = 0; blk < act.size(); blk++)
			for (size_t frame = 0; frame < act[blk].size(); frame++)
				act[blk][frame].traits = this->_oldValues[blk][frame];
	}

	std::string FlattenPropretiesOperation::getName() const noexcept
	{
		return this->_fieldName;
	}

	bool FlattenPropretiesOperation::hasModification() const
	{
		return this->_oldValues.size() > 1;
	}
} // SpiralOfFate
//
// Created by PinkySmile on 07/01/2026.
//

#include "FlattenPropretiesOperation.hpp"

namespace SpiralOfFate
{
	FlattenPropretiesOperation::FlattenPropretiesOperation(
		EditableObject &obj,
		const std::string &&name,
		FrameData newValue
	) :
		_obj(obj),
		_action(obj._action),
		_fieldName(name)
	{
		auto &act = obj._moves.at(obj._action);

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

	void FlattenPropretiesOperation::_makeCopy(FrameData &dst, const FrameData &src)
	{
		dst = src;
		if (!src.hitBoxes.empty())
			return;
		dst.dFlag.flags = 0;
		dst.oFlag.flags = 0;
		dst.wrongBlockStun = 0;
		dst.blockStun = 0;
		dst.hitStun = 0;
		dst.untech = 0;
		dst.guardDmg = 0;
		dst.prorate = 0;
		dst.minProrate = 0;
		dst.neutralLimit = 0;
		dst.voidLimit = 0;
		dst.matterLimit = 0;
		dst.spiritLimit = 0;
		dst.pushBack = 0;
		dst.pushBlock = 0;
		dst.manaGain = 0;
		dst.hitPlayerHitStop = 0;
		dst.hitOpponentHitStop = 0;
		dst.blockPlayerHitStop = 0;
		dst.blockOpponentHitStop = 0;
		dst.damage = 0;
		dst.chipDamage = 0;
	}

	void FlattenPropretiesOperation::_restoreCopy(FrameData &dst, const FrameData &src)
	{
		dst.dFlag = src.dFlag;
		dst.oFlag = src.oFlag;
		dst.wrongBlockStun = src.wrongBlockStun;
		dst.blockStun = src.blockStun;
		dst.hitStun = src.hitStun;
		dst.untech = src.untech;
		dst.guardDmg = src.guardDmg;
		dst.prorate = src.prorate;
		dst.minProrate = src.minProrate;
		dst.neutralLimit = src.neutralLimit;
		dst.voidLimit = src.voidLimit;
		dst.matterLimit = src.matterLimit;
		dst.spiritLimit = src.spiritLimit;
		dst.pushBack = src.pushBack;
		dst.pushBlock = src.pushBlock;
		dst.manaGain = src.manaGain;
		dst.hitPlayerHitStop = src.hitPlayerHitStop;
		dst.hitOpponentHitStop = src.hitOpponentHitStop;
		dst.blockPlayerHitStop = src.blockPlayerHitStop;
		dst.blockOpponentHitStop = src.blockOpponentHitStop;
		dst.damage = src.damage;
		dst.chipDamage = src.chipDamage;
	}

	void FlattenPropretiesOperation::apply()
	{
		auto &act = this->_obj._moves.at(this->_action);

		for (auto &blk : act)
			for (auto &f : blk)
				FlattenPropretiesOperation::_restoreCopy(f, this->_newValue);
	}

	void FlattenPropretiesOperation::undo()
	{
		auto &act = this->_obj._moves.at(this->_action);

		for (size_t blk = 0; blk < act.size(); blk++)
			for (size_t frame = 0; frame < act[blk].size(); frame++)
				FlattenPropretiesOperation::_restoreCopy(act[blk][frame], this->_oldValues[blk][frame]);
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
//
// Created by PinkySmile on 07/01/2026.
//

#include <ranges>
#include "CleanPropertiesOperation.hpp"

namespace SpiralOfFate
{
	CleanPropertiesOperation::CleanPropertiesOperation(
		EditableObject &obj,
		const std::string &&name
	) :
		_obj(obj),
		_fieldName(name)
	{
		for (auto &[id, act] : obj._moves) {
			this->_oldValues[id].reserve(act.size());
			for (auto &blk : act) {
				this->_oldValues[id].emplace_back();

				auto &vec = this->_oldValues[id].back();

				vec.reserve(blk.size());
				for (auto &f : blk) {
					vec.emplace_back();
					auto &frames = vec.back();
					frames = f;
				}
			}
		}
	}

	static TypeColor getMoveType(unsigned id, const std::vector<std::vector<FrameData>> &act)
	{
		switch (id) {
		case ACTION_NEUTRAL_OVERDRIVE:
		case ACTION_NEUTRAL_AIR_OVERDRIVE:
		case ACTION_GROUND_HIGH_NEUTRAL_PARRY:
		case ACTION_GROUND_LOW_NEUTRAL_PARRY:
		case ACTION_AIR_NEUTRAL_PARRY:
			return TYPECOLOR_NEUTRAL;
		case ACTION_SPIRIT_OVERDRIVE:
		case ACTION_SPIRIT_AIR_OVERDRIVE:
		case ACTION_GROUND_HIGH_SPIRIT_PARRY:
		case ACTION_GROUND_LOW_SPIRIT_PARRY:
		case ACTION_AIR_SPIRIT_PARRY:
			return TYPECOLOR_SPIRIT;
		case ACTION_MATTER_OVERDRIVE:
		case ACTION_MATTER_AIR_OVERDRIVE:
		case ACTION_GROUND_HIGH_MATTER_PARRY:
		case ACTION_GROUND_LOW_MATTER_PARRY:
		case ACTION_AIR_MATTER_PARRY:
			return TYPECOLOR_MATTER;
		case ACTION_VOID_OVERDRIVE:
		case ACTION_VOID_AIR_OVERDRIVE:
		case ACTION_GROUND_HIGH_VOID_PARRY:
		case ACTION_GROUND_LOW_VOID_PARRY:
		case ACTION_AIR_VOID_PARRY:
			return TYPECOLOR_VOID;
		default:
			if (id < ACTION_5N)
				return TYPECOLOR_NON_TYPED;
		}
		for (auto &blk : act)
			for (auto &f : blk) {
				if (f.oFlag.spiritElement && f.oFlag.matterElement && f.oFlag.voidElement)
					return TYPECOLOR_NEUTRAL;
				if (f.oFlag.spiritElement)
					return TYPECOLOR_SPIRIT;
				if (f.oFlag.matterElement)
					return TYPECOLOR_MATTER;
				if (f.oFlag.voidElement)
					return TYPECOLOR_VOID;
			}
		return TYPECOLOR_NON_TYPED;
	}

	void CleanPropertiesOperation::_clean(FrameData &data, TypeColor type)
	{
		if (type == TYPECOLOR_NON_TYPED) {
			data.oFlag.voidElement = false;
			data.oFlag.spiritElement = false;
			data.oFlag.matterElement = false;
		} else if (type == TYPECOLOR_NEUTRAL) {
			data.oFlag.voidElement = true;
			data.oFlag.spiritElement = true;
			data.oFlag.matterElement = true;
		} else if (type == TYPECOLOR_MATTER) {
			data.oFlag.voidElement = false;
			data.oFlag.spiritElement = false;
			data.oFlag.matterElement = true;
		} else if (type == TYPECOLOR_SPIRIT) {
			data.oFlag.voidElement = false;
			data.oFlag.spiritElement = true;
			data.oFlag.matterElement = false;
		} else if (type == TYPECOLOR_VOID) {
			data.oFlag.voidElement = true;
			data.oFlag.spiritElement = false;
			data.oFlag.matterElement = false;
		}
		if (!data.hitBoxes.empty())
			return;

		data.oFlag.grab = false;
		data.oFlag.airUnblockable = false;
		data.oFlag.unblockable = false;
		data.oFlag.lowHit = false;
		data.oFlag.highHit = false;
		data.oFlag.autoHitPos = false;
		data.oFlag.canCounterHit = false;
		data.oFlag.jab = false;
		data.oFlag.opposingPush = false;
		data.oFlag.restand = false;
		data.oFlag.nextBlockOnHit = false;
		data.oFlag.nextBlockOnBlock = false;
		data.oFlag.hardKnockDown = false;
		data.oFlag.groundSlam = false;
		data.oFlag.groundSlamCH = false;
		data.oFlag.wallSplat = false;
		data.oFlag.wallSplatCH = false;
		data.oFlag.phantomHit = false;
		data.wrongBlockStun = 8;
		data.blockStun = 0;
		data.hitStun = 0;
		data.untech = 0;
		data.guardDmg = 0;
		data.prorate = 0;
		data.minProrate = 0;
		data.neutralLimit = 0;
		data.voidLimit = 0;
		data.matterLimit = 0;
		data.spiritLimit = 0;
		data.pushBack = 0;
		data.pushBlock = 0;
		data.manaGain = 0;
		data.hitPlayerHitStop = 0;
		data.hitOpponentHitStop = 0;
		data.blockPlayerHitStop = 0;
		data.blockOpponentHitStop = 0;
		data.damage = 0;
		data.chipDamage = 0;
	}

	void CleanPropertiesOperation::apply()
	{
		for (auto &[id, act] : this->_obj._moves) {
			auto type = getMoveType(id, act);
			for (auto &blk : act)
				for (auto &f : blk)
					CleanPropertiesOperation::_clean(f, type);
		}
	}

	void CleanPropertiesOperation::undo()
	{
		for (auto &[id, act] : this->_obj._moves)
			for (size_t blk = 0; blk < act.size(); blk++)
				for (size_t frame = 0; frame < act[blk].size(); frame++)
					act[blk][frame] = this->_oldValues[id][blk][frame];
	}

	std::string CleanPropertiesOperation::getName() const noexcept
	{
		return this->_fieldName;
	}

	bool CleanPropertiesOperation::hasModification() const
	{
		return true;
	}
} // SpiralOfFate
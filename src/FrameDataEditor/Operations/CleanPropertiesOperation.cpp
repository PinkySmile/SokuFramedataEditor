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
		for (auto &[id, act] : obj._schema.framedata) {
			this->_oldTraits[id].reserve(act.size());
			for (auto &blk : act) {
				this->_oldTraits[id].emplace_back();

				auto &vec = this->_oldTraits[id].back();

				vec.reserve(blk.size());
				for (auto &f : blk) {
					vec.emplace_back();
					auto &frames = vec.back();
					frames = f.traits;
				}
			}
		}
	}

	void CleanPropertiesOperation::_clean(FrameData &data)
	{
		if (!data.aBoxes.empty())
			return;

		data.traits.speedX = 0;
		data.traits.speedY = 0;
		data.traits.chipDamage = 0;
		data.traits.spiritDamage = 0;
		data.traits.onBlockPlayerStun = 0;
		data.traits.onBlockEnemyStun = 0;
		data.traits.onBlockCardGain = 0;
		data.traits.damage = 0;
		data.traits.proration = 0;
		data.traits.untech = 0;
		data.traits.power = 0;
		data.traits.limit = 0;
		data.traits.onHitPlayerStun = 0;
		data.traits.onHitEnemyStun = 0;
		data.traits.onHitCardGain = 0;
		data.traits.onAirHitSetSequence = 0;
		data.traits.onGroundHitSetSequence = 0;
		data.traits.onHitSfx = 0;
		data.traits.onHitEffect = 0;
		data.traits.attackLevel = 0;
		data.traits.comboModifier = 0;
		data.traits.attackFlags = 0;
	}

	void CleanPropertiesOperation::apply()
	{
		for (auto &[id, act] : this->_obj._schema.framedata) {
			for (auto &blk : act)
				for (auto &f : blk)
					CleanPropertiesOperation::_clean(f);
		}
	}

	void CleanPropertiesOperation::undo()
	{
		for (auto &[id, act] : this->_obj._schema.framedata)
			for (size_t blk = 0; blk < act.size(); blk++)
				for (size_t frame = 0; frame < act[blk].size(); frame++)
					act[blk][frame].traits = this->_oldTraits[id][blk][frame];
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
//
// Created by PinkySmile on 06/01/2026.
//

#include "FlattenCollisionBoxesOperation.hpp"

namespace SpiralOfFate
{
	FlattenCollisionBoxesOperation::FlattenCollisionBoxesOperation(
		EditableObject &obj,
		const std::string &&name,
		std::optional<ShadyCore::Schema::Sequence::BBox> newValue
	) :
		_obj(obj),
		_action(obj._action),
		_newValue(newValue),
		_fieldName(name)
	{
		auto &act = obj._schema.framedata.at(obj._action);

		this->_oldValues.reserve(act.size());
		for (auto &blk : act) {
			this->_oldValues.emplace_back();

			auto &vec = this->_oldValues.back();

			vec.reserve(blk.size());
			for (auto &f : blk)
				if (f.cBoxes.empty())
					vec.emplace_back();
				else
					vec.emplace_back(f.cBoxes.front());
		}
	}

	void FlattenCollisionBoxesOperation::apply()
	{
		auto &act = this->_obj._schema.framedata.at(this->_action);

		for (auto &blk : act)
			for (auto &f : blk) {
				if (f.cBoxes.empty() && !this->_newValue)
					continue;
				if (f.cBoxes.empty())
					f.cBoxes.resize(1, *this->_newValue);
				else if (!this->_newValue)
					f.cBoxes.clear();
				else
					f.cBoxes.front() = *this->_newValue;
			}
	}

	void FlattenCollisionBoxesOperation::undo()
	{
		auto &act = this->_obj._schema.framedata.at(this->_action);

		for (size_t blk = 0; blk < act.size(); blk++)
			for (size_t frame = 0; frame < act[blk].size(); frame++) {
				auto &f = act[blk][frame];
				auto &o = this->_oldValues[blk][frame];

				if (f.cBoxes.empty() && !o)
					continue;
				if (f.cBoxes.empty())
					f.cBoxes.resize(1, *o);
				else if (!o)
					f.cBoxes.clear();
				else
					f.cBoxes.front() = *o;
			}
	}

	std::string FlattenCollisionBoxesOperation::getName() const noexcept
	{
		return this->_fieldName;
	}

	bool FlattenCollisionBoxesOperation::hasModification() const
	{
		return this->_oldValues.size() > 1;
	}
} // SpiralOfFate
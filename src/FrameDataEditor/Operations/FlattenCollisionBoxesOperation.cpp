//
// Created by PinkySmile on 06/01/2026.
//

#include "FlattenCollisionBoxesOperation.hpp"

namespace SpiralOfFate
{
	FlattenCollisionBoxesOperation::FlattenCollisionBoxesOperation(
		EditableObject &obj,
		const std::string &&name,
		std::optional<Box> newValue
	) :
		_obj(obj),
		_action(obj._action),
		_newValue(newValue),
		_fieldName(name)
	{
		auto &act = obj._moves.at(obj._action);

		this->_oldValues.reserve(act.size());
		for (auto &blk : act) {
			this->_oldValues.emplace_back();

			auto &vec = this->_oldValues.back();

			vec.reserve(blk.size());
			for (auto &f : blk)
				if (f.collisionBox)
					vec.emplace_back(*f.collisionBox);
				else
					vec.emplace_back();
		}
	}

	void FlattenCollisionBoxesOperation::apply()
	{
		auto &act = this->_obj._moves.at(this->_action);

		for (auto &blk : act)
			for (auto &f : blk) {
				if (!f.collisionBox && !this->_newValue)
					continue;
				if (!f.collisionBox)
					f.collisionBox = new Box(*this->_newValue);
				else if (!this->_newValue) {
					delete f.collisionBox;
					f.collisionBox = nullptr;
				} else
					*f.collisionBox = *this->_newValue;
			}
	}

	void FlattenCollisionBoxesOperation::undo()
	{
		auto &act = this->_obj._moves.at(this->_action);

		for (size_t blk = 0; blk < act.size(); blk++)
			for (size_t frame = 0; frame < act[blk].size(); frame++) {
				auto &f = act[blk][frame];
				auto &o = this->_oldValues[blk][frame];

				if (!f.collisionBox && !o)
					continue;
				if (!f.collisionBox)
					f.collisionBox = new Box(*o);
				else if (!o) {
					delete f.collisionBox;
					f.collisionBox = nullptr;
				} else
					*f.collisionBox = *o;
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
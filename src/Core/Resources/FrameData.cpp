//
// Created by andgel on 18/09/2021
//

#include <fstream>
#include "FrameData.hpp"
#include "Game.hpp"
#include "../Logger.hpp"

namespace SpiralOfFate
{
	std::map<unsigned, std::vector<std::vector<FrameData>>> FrameData::loadSchema(const std::string &chr, const ShadyCore::Schema &schema, const ShadyCore::Palette &palette, const std::string &palName)
	{
		std::map<unsigned int, std::vector<std::vector<FrameData>>> result;
		std::vector<ShadyCore::Schema::Clone *> objects;

		for (auto &sequ : schema.objects) {
			if (sequ->getType() == 7)
				objects.push_back(reinterpret_cast<ShadyCore::Schema::Clone *>(sequ));
			else if (sequ->getType() == 9) {
				result[sequ->getId()].emplace_back();

				auto &sequence = *reinterpret_cast<ShadyCore::Schema::Sequence *>(sequ);
				auto &arr = result[sequ->getId()].back();

				for (auto move : sequence.frames) {
#ifdef _DEBUG
					assert(dynamic_cast<ShadyCore::Schema::Sequence::MoveFrame *>(move));
#endif
					arr.emplace_back(chr, schema, *reinterpret_cast<ShadyCore::Schema::Sequence::MoveFrame *>(move), palette, palName);
				}
			} else
				throw std::invalid_argument("Found unknown object id " + std::to_string(sequ->getType()) + " in schema");
		}
		for (auto &object : objects)
			result[object->getId()] = result.at(object->targetId);
		return result;
	}

	FrameData::FrameData(const std::string &chr, const ShadyCore::Schema &schema, const ShadyCore::Palette &palette, const std::string &palName) :
		ShadyCore::Schema::Sequence::MoveFrame(),
		_pal(palName),
		_character(chr),
		_palette(palette),
		_schema(schema)
	{
	}

	FrameData::FrameData(const std::string &chr, const ShadyCore::Schema &schema, ShadyCore::Schema::Sequence::MoveFrame &frame, const ShadyCore::Palette &palette, const std::string &palName) :
		ShadyCore::Schema::Sequence::MoveFrame(frame),
		_pal(palName),
		_character(chr),
		_palette(palette),
		_schema(schema)
	{
		Vector2u textureSize;

		this->textureHandle = game->textureMgr.load(game->package, this->_palette, this->_pal, "data/character/" + this->_character + "/" + this->_schema.images.at(this->imageIndex).name, &textureSize);
		if (this->traits.onHitSfx) {
			char buffer[4];

			sprintf(buffer, "%03d", this->traits.onHitSfx);
			this->hitSoundHandle = game->soundMgr.load(game->package, "data/se/" + std::string(buffer) + ".cv3");
		}
		if (this->cBoxes.size() > 1)
			throw std::runtime_error("FrameData::FrameData: More than one collision box is not supported");
		for (auto &box : this->cBoxes)
			if (box.extra)
				throw std::runtime_error("FrameData::FrameData: Extra collision box is not supported");
		for (auto &box : this->aBoxes)
			if (box.extra)
				throw std::runtime_error("FrameData::FrameData: Extra attack box is not supported");
		for (auto &box : this->hBoxes)
			if (box.extra)
				throw std::runtime_error("FrameData::FrameData: Extra hit box is not supported");
	}

	FrameData::~FrameData()
	{
		if (!this->_slave)
			game->textureMgr.remove(this->textureHandle);
	}

	FrameData::FrameData(const FrameData &other) :
		ShadyCore::Schema::Sequence::MoveFrame(other),
		_pal(other._pal),
		_character(other._character),
		_palette(other._palette),
		_schema(other._schema)
	{
		this->textureHandle = other.textureHandle;
		this->hitSoundHandle = other.hitSoundHandle;
		if (!this->_slave) {
			game->textureMgr.addRef(this->textureHandle);
			game->soundMgr.addRef(this->hitSoundHandle);
		}
	}

	FrameData &FrameData::operator=(const FrameData &other)
	{
		if (!this->_slave) {
			game->textureMgr.remove(this->textureHandle);
			game->soundMgr.remove(this->hitSoundHandle);
		}
		this->textureHandle = other.textureHandle;
		this->hitSoundHandle = other.hitSoundHandle;
		ShadyCore::Schema::Sequence::MoveFrame::operator=(other);
		if (!this->_slave) {
			game->textureMgr.addRef(this->textureHandle);
			game->soundMgr.addRef(this->hitSoundHandle);
		}
		return *this;
	}

	void FrameData::reloadTexture()
	{
		my_assert(!this->_slave);
		game->textureMgr.remove(this->textureHandle);
		// TODO: Use the actual value
		this->textureHandle = game->textureMgr.load(game->package, this->_palette, this->_pal, "data/character/" + this->_character + "/attackAb001.cv3");
	}

	void FrameData::reloadSound()
	{
		my_assert(!this->_slave);
		game->soundMgr.remove(this->hitSoundHandle);
		this->hitSoundHandle = 0;
		if (this->traits.onHitSfx) {
			char buffer[4];

			sprintf(buffer, "%03d", this->traits.onHitSfx);
			this->hitSoundHandle = game->soundMgr.load(game->package, "data/se/" + std::string(buffer) + ".cv0");
		}
	}

	void FrameData::setSlave(bool slave)
	{
		if (!slave && this->_slave)
			game->textureMgr.remove(this->textureHandle);
		this->_slave = slave;
	}

	Box::operator sf::IntRect() const noexcept
	{
		return {
			this->pos.x, this->pos.y,
			static_cast<int>(this->size.x), static_cast<int>(this->size.y)
		};
	}
}

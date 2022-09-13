//
// Created by andgel on 18/09/2021
//

#include <fstream>
#include "FrameData.hpp"
#include "Game.hpp"

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
			result.emplace(object->getId(), object->targetId);
		if (result.find(0) == result.end())
			throw std::invalid_argument("Schema does not contain action with id 0");
		return result;
	}

	FrameData::FrameData(const std::string &chr, const ShadyCore::Schema &schema, ShadyCore::Schema::Sequence::MoveFrame &frame, const ShadyCore::Palette &palette, const std::string &palName) :
		_pal(palName),
		_character(chr),
		_palette(&palette),
		_schema(schema),
		imageIndex(frame.imageIndex),
		unknown(frame.unknown),
		texOffsetX(frame.texOffsetX),
		texOffsetY(frame.texOffsetY),
		texWidth(frame.texWidth),
		texHeight(frame.texHeight),
		offsetX(frame.offsetX),
		offsetY(frame.offsetY),
		duration(frame.duration),
		renderGroup(frame.renderGroup),
		blendOptions(frame.blendOptions),
		traits(frame.traits),
		cBoxes(frame.cBoxes),
		hBoxes(frame.hBoxes),
		aBoxes(frame.aBoxes),
		effect(frame.effect),
		frame(frame)
	{
		Vector2u textureSize;

		for (auto &box : this->hBoxes)
			if (box.up > box.down) {
				auto down = box.up;

				box.up = box.down;
				box.down = down;
			}
		for (auto &box : this->aBoxes)
			if (box.up > box.down) {
				auto down = box.up;

				box.up = box.down;
				box.down = down;
			}
		try {
			this->textureHandle = game->textureMgr.load(game->package, *this->_palette, this->_pal, "data/character/" + this->_character + "/" + this->_schema.images.at(this->imageIndex).name, &textureSize);
		} catch (std::exception &e) {
			game->logger.error("Error loading texture: " + std::string(e.what()) + "\n");
		}
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
		_pal(other._pal),
		_character(other._character),
		_palette(other._palette),
		_schema(other._schema),
		imageIndex(other.imageIndex),
		unknown(other.unknown),
		texOffsetX(other.texOffsetX),
		texOffsetY(other.texOffsetY),
		texWidth(other.texWidth),
		texHeight(other.texHeight),
		offsetX(other.offsetX),
		offsetY(other.offsetY),
		duration(other.duration),
		renderGroup(other.renderGroup),
		blendOptions(other.blendOptions),
		traits(other.traits),
		cBoxes(other.cBoxes),
		hBoxes(other.hBoxes),
		aBoxes(other.aBoxes),
		effect(other.effect),
		frame(other.frame)
	{
		this->textureHandle = other.textureHandle;
		this->hitSoundHandle = other.hitSoundHandle;
		if (!this->_slave) {
			game->textureMgr.addRef(this->textureHandle);
			game->soundMgr.addRef(this->hitSoundHandle);
		}
	}

	void FrameData::reloadTexture()
	{
		my_assert(!this->_slave);
		this->needReload = false;
		game->textureMgr.remove(this->textureHandle);
		try {
			this->textureHandle = game->textureMgr.load(game->package, *this->_palette, this->_pal, "data/character/" + this->_character + "/" + this->_schema.images.at(this->imageIndex).name);
		} catch (std::exception &e) {
			game->logger.error("Error loading texture: " + std::string(e.what()) + "\n");
		}
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
		if (!slave && this->_slave) {
			game->textureMgr.addRef(this->textureHandle);
			game->soundMgr.addRef(this->hitSoundHandle);
		} else if (slave && !this->_slave) {
			game->textureMgr.remove(this->textureHandle);
			game->soundMgr.remove(this->hitSoundHandle);
		}
		this->_slave = slave;
	}

	void FrameData::setPalette(const ShadyCore::Palette &palette, const std::string &name)
	{
		this->needReload = !this->_slave;
		this->_pal = name;
		this->_palette = &palette;
	}

	FrameData &FrameData::operator=(FrameData &other)
	{
		this->_pal = other._pal;
		this->_character = other._character;
		this->_palette = other._palette;
		if (!this->_slave) {
			game->textureMgr.remove(this->textureHandle);
			game->soundMgr.remove(this->hitSoundHandle);
			game->textureMgr.addRef(other.textureHandle);
			game->soundMgr.addRef(other.hitSoundHandle);
		}
		this->textureHandle = other.textureHandle;
		this->hitSoundHandle = other.hitSoundHandle;
		this->needReload = other.needReload;
		(*(const ShadyCore::Schema **)&this->_schema) = &other._schema;
		(*(uint16_t **)&this->imageIndex) = &other.imageIndex;
		(*(uint16_t **)&this->unknown) = &other.unknown;
		(*(uint16_t **)&this->texOffsetX) = &other.texOffsetX;
		(*(uint16_t **)&this->texOffsetY) = &other.texOffsetY;
		(*(uint16_t **)&this->texWidth) = &other.texWidth;
		(*(uint16_t **)&this->texHeight) = &other.texHeight;
		(*(int16_t **)&this->offsetX) = &other.offsetX;
		(*(int16_t **)&this->offsetY) = &other.offsetY;
		(*(uint16_t **)&this->duration) = &other.duration;
		(*(uint8_t **)&this->renderGroup) = &other.renderGroup;
		(*(ShadyCore::Schema::Sequence::MoveTraits **)&this->traits) = &other.traits;
		(*(std::vector<ShadyCore::Schema::Sequence::BBox> **)&this->cBoxes) = &other.cBoxes;
		(*(std::vector<ShadyCore::Schema::Sequence::BBox> **)&this->hBoxes) = &other.hBoxes;
		(*(std::vector<ShadyCore::Schema::Sequence::BBox> **)&this->aBoxes) = &other.aBoxes;
		(*(ShadyCore::Schema::Sequence::MoveEffect **)&this->effect) = &other.effect;
		(*(ShadyCore::Schema::Sequence::MoveFrame **)&this->frame) = &other.frame;
		return *this;
	}

	FrameData &FrameData::operator=(FrameData &&other)
	{
		this->_pal = other._pal;
		this->_character = other._character;
		this->_palette = other._palette;
		if (!this->_slave) {
			game->textureMgr.remove(this->textureHandle);
			game->soundMgr.remove(this->hitSoundHandle);
			game->textureMgr.addRef(other.textureHandle);
			game->soundMgr.addRef(other.hitSoundHandle);
		}
		this->textureHandle = other.textureHandle;
		this->hitSoundHandle = other.hitSoundHandle;
		this->needReload = other.needReload;
		(*(const ShadyCore::Schema **)&this->_schema) = &other._schema;
		(*(uint16_t **)&this->imageIndex) = &other.imageIndex;
		(*(uint16_t **)&this->unknown) = &other.unknown;
		(*(uint16_t **)&this->texOffsetX) = &other.texOffsetX;
		(*(uint16_t **)&this->texOffsetY) = &other.texOffsetY;
		(*(uint16_t **)&this->texWidth) = &other.texWidth;
		(*(uint16_t **)&this->texHeight) = &other.texHeight;
		(*(int16_t **)&this->offsetX) = &other.offsetX;
		(*(int16_t **)&this->offsetY) = &other.offsetY;
		(*(uint16_t **)&this->duration) = &other.duration;
		(*(uint8_t **)&this->renderGroup) = &other.renderGroup;
		(*(ShadyCore::Schema::Sequence::MoveTraits **)&this->traits) = &other.traits;
		(*(std::vector<ShadyCore::Schema::Sequence::BBox> **)&this->cBoxes) = &other.cBoxes;
		(*(std::vector<ShadyCore::Schema::Sequence::BBox> **)&this->hBoxes) = &other.hBoxes;
		(*(std::vector<ShadyCore::Schema::Sequence::BBox> **)&this->aBoxes) = &other.aBoxes;
		(*(ShadyCore::Schema::Sequence::MoveEffect **)&this->effect) = &other.effect;
		(*(ShadyCore::Schema::Sequence::MoveFrame **)&this->frame) = &other.frame;
		return *this;
	}

	Box::operator sf::IntRect() const noexcept
	{
		return {
			this->pos.x, this->pos.y,
			static_cast<int>(this->size.x), static_cast<int>(this->size.y)
		};
	}
}

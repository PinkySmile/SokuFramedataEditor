//
// Created by andgel on 18/09/2021
//

#include <fstream>
#include "FrameData.hpp"
#include "Game.hpp"
#include "Utils.hpp"

uint32_t getOrCreateImage(ShadyCore::Schema &schema, const std::string_view& name)
{
	for (size_t i = 0; i < schema.images.size(); ++i)
		if (
			strncmp(schema.images[i].name, name.data(), name.size()) == 0 &&
			schema.images[i].name[name.size()] == '\0'
			)
			return i;

	auto res = new char[name.size() + 1];

	strcpy(res, name.data());
	schema.images.emplace_back(res);
	return schema.images.size() - 1;
}

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

				arr.reserve(sequence.frames.size());
				for (auto move : sequence.frames) {
#ifdef _DEBUG
					assert(dynamic_cast<ShadyCore::Schema::Sequence::MoveFrame *>(move));
#endif
					arr.emplace_back(chr, schema, sequence, *reinterpret_cast<ShadyCore::Schema::Sequence::MoveFrame *>(move), palette, palName);
				}
			} else
				throw std::invalid_argument("Found unknown object id " + std::to_string(sequ->getType()) + " in schema");
		}
		for (auto &object : objects) {
			game->logger.info(std::to_string(object->getId()) + " is a clone of " + std::to_string(object->targetId));

			auto r = result.emplace(object->getId(), result[object->targetId]);

			if (!r.second)
				Utils::dispMsg("Invalid clone", "Action " + std::to_string(object->getId()) + " already exists but is trying to be cloned (from " + std::to_string(object->targetId) + ").", MB_ICONERROR);
			//TODO: Store clone and notify user
			//else
			//	r.first->clone = object->targetId;
		}
		if (result.begin() == result.end())
			throw std::invalid_argument("Schema is empty!");
		return result;
	}

	FrameData::FrameData(const std::string &chr, const ShadyCore::Schema &schema, ShadyCore::Schema::Sequence &parent, ShadyCore::Schema::Sequence::MoveFrame &frame, const ShadyCore::Palette &palette, const std::string &palName) :
		_pal(palName),
		_character(chr),
		_palette(&palette),
		_schema(&schema),
		parent(&parent),
		frame(&frame)
	{
		Vector2u textureSize;

		for (auto &box : this->frame->hBoxes)
			if (box.up > box.down) {
				auto down = box.up;

				box.up = box.down;
				box.down = down;
			}
		for (auto &box : this->frame->aBoxes)
			if (box.up > box.down) {
				auto down = box.up;

				box.up = box.down;
				box.down = down;
			}
		try {
			this->textureHandle = game->textureMgr.load(game->package, *this->_palette, this->_pal, "data/character/" + this->_character + "/" + this->_schema->images.at(this->frame->imageIndex).name, &textureSize);
		} catch (std::exception &e) {
			game->logger.error("Error loading texture: " + std::string(e.what()) + "\n");
		}
		if (this->frame->cBoxes.size() > 1)
			throw std::runtime_error("FrameData::FrameData: More than one collision box is not supported");
		for (auto &box : this->frame->cBoxes)
			if (box.extra)
				throw std::runtime_error("FrameData::FrameData: Extra collision box is not supported");
		for (auto &box : this->frame->aBoxes)
			if (box.extra)
				throw std::runtime_error("FrameData::FrameData: Extra attack box is not supported");
		for (auto &box : this->frame->hBoxes)
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
		parent(other.parent),
		frame(other.frame)
	{
		this->textureHandle = other.textureHandle;
		game->textureMgr.addRef(this->textureHandle);
	}

	FrameData::FrameData(const FrameData &other, ShadyCore::Schema::Sequence::MoveFrame &frame) :
		FrameData(other)
	{
		this->frame = &frame;
	}

	void FrameData::reloadTexture()
	{
		my_assert(!this->_slave);
		this->needReload = false;
		game->textureMgr.remove(this->textureHandle);
		try {
			this->textureHandle = game->textureMgr.load(game->package, *this->_palette, this->_pal, "data/character/" + this->_character + "/" + this->_schema->images.at(this->frame->imageIndex).name);
		} catch (std::exception &e) {
			game->logger.error("Error loading texture: " + std::string(e.what()) + "\n");
		}
	}

	void FrameData::setSlave(bool slave)
	{
		if (!slave && this->_slave)
			game->textureMgr.addRef(this->textureHandle);
		else if (slave && !this->_slave)
			game->textureMgr.remove(this->textureHandle);
		this->_slave = slave;
	}

	void FrameData::setPalette(const ShadyCore::Palette &palette, const std::string &name)
	{
		this->needReload = !this->_slave;
		this->_pal = name;
		this->_palette = &palette;
	}


	FrameData &FrameData::operator=(const FrameData &other)
	{
		this->_pal = other._pal;
		this->_character = other._character;
		this->_palette = other._palette;
		if (!this->_slave) {
			game->textureMgr.remove(this->textureHandle);
			game->textureMgr.addRef(other.textureHandle);
		}
		this->textureHandle = other.textureHandle;
		this->needReload = other.needReload;
		this->_schema = other._schema;
		this->frame = other.frame;
		return *this;
	}

	FrameData &FrameData::operator=(FrameData &&other) noexcept
	{
		this->_pal = other._pal;
		this->_character = other._character;
		this->_palette = other._palette;
		if (!this->_slave) {
			game->textureMgr.remove(this->textureHandle);
			game->textureMgr.addRef(other.textureHandle);
		}
		this->textureHandle = other.textureHandle;
		this->needReload = other.needReload;
		this->_schema = other._schema;
		this->frame = other.frame;
		return *this;
	}

	nlohmann::json FrameData::saveData() const
	{
		nlohmann::json j{
			{ "collision", std::vector<std::string>() },
			{ "attack", std::vector<std::string>() },
			{ "hit", std::vector<std::string>() }
		};

		for (auto &b : this->frame->cBoxes)
			j["collision"].push_back({
				{ "left", b.left },
				{ "right", b.right },
				{ "up", b.up },
				{ "down", b.down },
			});
		for (auto &b : this->frame->aBoxes)
			j["attack"].push_back({
				{ "left", b.left },
				{ "right", b.right },
				{ "up", b.up },
				{ "down", b.down },
			});
		for (auto &b : this->frame->hBoxes)
			j["hit"].push_back({
				{ "left", b.left },
				{ "right", b.right },
				{ "up", b.up },
				{ "down", b.down },
			});
		j["sprite"] = this->_schema->images.at(this->frame->imageIndex).name;
		j["unknown"] = this->frame->unknown;
		j["texOffsetX"] = this->frame->texOffsetX;
		j["texOffsetY"] = this->frame->texOffsetY;
		j["texWidth"] = this->frame->texWidth;
		j["texHeight"] = this->frame->texHeight;
		j["offsetX"] = this->frame->offsetX;
		j["offsetY"] = this->frame->offsetY;
		j["duration"] = this->frame->duration;
		j["renderGroup"] = this->frame->renderGroup;
		j["blendMode"] = this->frame->blendOptions.mode;
		j["blendColor"] = this->frame->blendOptions.color;
		j["blendScaleX"] = this->frame->blendOptions.scaleX;
		j["blendScaleY"] = this->frame->blendOptions.scaleY;
		j["blendFlipVert"] = this->frame->blendOptions.flipVert;
		j["blendFlipHorz"] = this->frame->blendOptions.flipHorz;
		j["blendAngle"] = this->frame->blendOptions.angle;
		return j;
	}

	void FrameData::_loadSpriteInfo(const nlohmann::json &json)
	{
		this->frame->imageIndex = getOrCreateImage(const_cast<ShadyCore::Schema &>(*this->_schema), json["sprite"].get<std::string>());
		this->frame->unknown = json["unknown"];
		this->frame->texOffsetX = json["texOffsetX"];
		this->frame->texOffsetY = json["texOffsetY"];
		this->frame->texWidth = json["texWidth"];
		this->frame->texHeight = json["texHeight"];
		this->frame->offsetX = json["offsetX"];
		this->frame->offsetY = json["offsetY"];
		this->frame->duration = json["duration"];
		this->frame->renderGroup = json["renderGroup"];
		this->frame->blendOptions.mode = json["blendMode"];
		this->frame->blendOptions.color = json["blendColor"];
		this->frame->blendOptions.scaleX = json["blendScaleX"];
		this->frame->blendOptions.scaleY = json["blendScaleY"];
		this->frame->blendOptions.flipVert = json["blendFlipVert"];
		this->frame->blendOptions.flipHorz = json["blendFlipHorz"];
		this->frame->blendOptions.angle = json["blendAngle"];
	}

	void FrameData::_loadBoxes(const nlohmann::json &json)
	{
		for (auto &b : json["collision"])
			this->frame->cBoxes.push_back({
				.left = b["left"],
				.up = b["up"],
				.right = b["right"],
				.down = b["down"]
			});
		for (auto &b : json["attack"])
			this->frame->aBoxes.push_back({
				.left = b["left"],
				.up = b["up"],
				.right = b["right"],
				.down = b["down"]
			});
		for (auto &b : json["hit"])
			this->frame->hBoxes.push_back({
				.left = b["left"],
				.up = b["up"],
				.right = b["right"],
				.down = b["down"]
			});
	}

	void FrameData::loadData(const nlohmann::json &json)
	{
		auto cBoxes = this->frame->cBoxes;
		auto aBoxes = this->frame->aBoxes;
		auto hBoxes = this->frame->hBoxes;
		ShadyCore::Schema::Sequence::Frame _frame = *this->frame;

		this->frame->cBoxes.clear();
		this->frame->aBoxes.clear();
		this->frame->hBoxes.clear();
		try {
			this->_loadBoxes(json);
			this->_loadSpriteInfo(json);
		} catch (...) {
			this->frame->cBoxes = cBoxes;
			this->frame->aBoxes = aBoxes;
			this->frame->hBoxes = hBoxes;
			this->frame->imageIndex = _frame.imageIndex;
			this->frame->unknown = _frame.unknown;
			this->frame->texOffsetX = _frame.texOffsetX;
			this->frame->texOffsetY = _frame.texOffsetY;
			this->frame->texWidth = _frame.texWidth;
			this->frame->texHeight = _frame.texHeight;
			this->frame->offsetX = _frame.offsetX;
			this->frame->offsetY = _frame.offsetY;
			this->frame->duration = _frame.duration;
			this->frame->renderGroup = _frame.renderGroup;
			this->frame->blendOptions = _frame.blendOptions;
			throw;
		}
		this->reloadTexture();
	}

	void FrameData::loadSpriteInfo(const nlohmann::json &json)
	{
		ShadyCore::Schema::Sequence::Frame _frame = *this->frame;

		try {
			this->_loadSpriteInfo(json);
		} catch (...) {
			this->frame->imageIndex = _frame.imageIndex;
			this->frame->unknown = _frame.unknown;
			this->frame->texOffsetX = _frame.texOffsetX;
			this->frame->texOffsetY = _frame.texOffsetY;
			this->frame->texWidth = _frame.texWidth;
			this->frame->texHeight = _frame.texHeight;
			this->frame->offsetX = _frame.offsetX;
			this->frame->offsetY = _frame.offsetY;
			this->frame->duration = _frame.duration;
			this->frame->renderGroup = _frame.renderGroup;
			this->frame->blendOptions = _frame.blendOptions;
			throw;
		}
		this->reloadTexture();
	}

	void FrameData::loadBoxes(const nlohmann::json &json)
	{
		auto cBoxes = this->frame->cBoxes;
		auto aBoxes = this->frame->aBoxes;
		auto hBoxes = this->frame->hBoxes;

		this->frame->cBoxes.clear();
		this->frame->aBoxes.clear();
		this->frame->hBoxes.clear();
		try {
			this->_loadBoxes(json);
		} catch (...) {
			this->frame->cBoxes = cBoxes;
			this->frame->aBoxes = aBoxes;
			this->frame->hBoxes = hBoxes;
			throw;
		}
	}

	Box::operator sf::IntRect() const noexcept
	{
		return {
			this->pos.x, this->pos.y,
			static_cast<int>(this->size.x), static_cast<int>(this->size.y)
		};
	}
}

//
// Created by PinkySmile on 18/09/2021.
//

#include <fstream>
#include "FrameData.hpp"
#include "Resources/Game.hpp"
#include "Logger.hpp"
#include "Utils.hpp"
#include "Resources/Assert.hpp"

uint32_t getOrCreateImage(ShadyCore::Schema &schema, const std::string_view &name)
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
	FrameData::LoadedSchema loadFileSchema(const ShadyCore::Schema &schema, const std::string &folder, const std::array<Color, 256> *palette)
	{
		FrameData::LoadedSchema result;
		std::vector<ShadyCore::Schema::Clone *> clones;
		bool foundType = false;

		assert_exp(!schema.objects.empty());
		result.isCharacterData = true;
		for (auto &sequ : schema.objects) {
			// TODO: Handle anims
			if (sequ->getType() == 7) {
			#ifdef _DEBUG
				assert_exp(dynamic_cast<ShadyCore::Schema::Clone *>(sequ));
			#endif
				clones.push_back(reinterpret_cast<ShadyCore::Schema::Clone *>(sequ));
			} else if (sequ->getType() == 8) {
			#ifdef _DEBUG
				assert_exp(dynamic_cast<ShadyCore::Schema::Sequence *>(sequ));
			#endif
				if (foundType)
					assert_exp(!result.isCharacterData);
				else
					result.isCharacterData = false;
				foundType = true;

				result.framedata[sequ->getId()]._sequences.emplace_back();

				auto &sequence = *reinterpret_cast<ShadyCore::Schema::Sequence *>(sequ);
				auto &arr = result.framedata[sequ->getId()]._sequences.back();

				arr.loop = sequence.loop;
				arr.moveLock = sequence.moveLock;
				arr.actionLock = sequence.actionLock;
				arr.data.reserve(sequence.frames.size());
				for (auto move : sequence.frames)
					arr.data.emplace_back(*move, folder, palette, schema);
			} else if (sequ->getType() == 9) {
			#ifdef _DEBUG
				assert_exp(dynamic_cast<ShadyCore::Schema::Sequence *>(sequ));
			#endif
				if (foundType)
					assert_exp(result.isCharacterData);
				else
					result.isCharacterData = true;
				foundType = true;

				result.framedata[sequ->getId()]._sequences.emplace_back();

				auto &sequence = *reinterpret_cast<ShadyCore::Schema::Sequence *>(sequ);
				auto &arr = result.framedata[sequ->getId()]._sequences.back();

				arr.loop = sequence.loop;
				arr.moveLock = sequence.moveLock;
				arr.actionLock = sequence.actionLock;
				arr.data.reserve(sequence.frames.size());
				for (auto move : sequence.frames) {
				#ifdef _DEBUG
					assert_exp(dynamic_cast<ShadyCore::Schema::Sequence::MoveFrame *>(move));
				#endif
					arr.data.emplace_back(*reinterpret_cast<ShadyCore::Schema::Sequence::MoveFrame *>(move), folder, palette, schema);
				}
			} else
				throw std::invalid_argument("Found unknown object id " + std::to_string(sequ->getType()) + " in schema");
		}
		for (auto clone : clones) {
			game->logger.info(std::to_string(clone->getId()) + " is a clone of " + std::to_string(clone->targetId));
			if (result.framedata.contains(clone->getId())) {
				Utils::dispMsg(
					game->gui,
					// TODO: Hardcoded string
					"Invalid clone",
					"Action " + std::to_string(clone->getId()) + " already exists but is trying to be cloned (from " + std::to_string(clone->targetId) + ").",
					MB_ICONERROR
				);
				continue;
			}
			if (!result.framedata.contains(clone->targetId))
				throw std::runtime_error("Clone target " + std::to_string(clone->targetId) + " doesn't exist");
			result.framedata[clone->getId()].clonedId = clone->targetId;
			result.framedata[clone->getId()].cloned = &result.framedata.at(clone->targetId);
		}
		return result;
	}

	FrameData::LoadedSchema FrameData::loadFile(const std::string &path, const std::string &folder, const std::array<Color, 256> *palette)
	{
		assert_exp(palette);
		game->logger.debug("Loading framedata file from package " + path);

		auto entry = game->package.find(path, ShadyCore::FileType::TYPE_SCHEMA);
		ShadyCore::Schema schema;

		if (entry == game->package.end())
			// TODO: Hardcoded string
			throw std::runtime_error("Cannot find " + path);
		if (entry.fileType().format == ShadyCore::FileType::SCHEMA_GAME_GUI)
			// TODO: Hardcoded string
			throw std::runtime_error("Cannot load " + path + ": Gui schemas not supported");

		auto &stream = entry.open();

		ShadyCore::getResourceReader(entry.fileType())(&schema, stream);
		entry.close(stream);
		return loadFileSchema(schema, folder, palette);
	}

	FrameData::LoadedSchema FrameData::loadFile(const std::filesystem::path &path, const std::string &folder, const std::array<Color, 256> *palette)
	{
		assert_exp(palette);
		game->logger.debug("Loading framedata file from " + path.string());

		ShadyCore::Schema schema;
		std::ifstream stream{path};

		if (!stream)
			throw std::runtime_error("Cannot open " + path.string());

		ShadyCore::FileType::Format format;
		// TODO: Allow the user to manually set it
		if (path.extension() == ".xml")
			format = ShadyCore::FileType::SCHEMA_XML;
		else if (path.filename().stem().string().ends_with("effect") || path.filename().stem().string().ends_with("stand"))
			format = ShadyCore::FileType::SCHEMA_GAME_ANIM;
		else
			format = ShadyCore::FileType::SCHEMA_GAME_PATTERN;

		ShadyCore::getResourceReader({ShadyCore::FileType::TYPE_SCHEMA, format})(&schema, stream);
		stream.close();
		return loadFileSchema(schema, folder, palette);
	}

	FrameData::FrameData(const FrameData &other) :
		ShadyCore::Schema::Sequence::MoveFrame(other),
		__folder(other.__folder),
		__requireReload(other.__requireReload),
		__paletteData(other.__paletteData),
		textureHandle(other.textureHandle)
	{
		game->textureMgr.addRef(this->textureHandle);
	}

	FrameData::~FrameData()
	{
		if (!this->_slave)
			game->textureMgr.remove(this->textureHandle);
	}

	FrameData::FrameData(const nlohmann::json &json, const std::string &folder, const std::array<Color, 256> *palette) :
		__folder(folder),
		__paletteData(palette)
	{
		this->spritePath = json["sprite"];
		this->unknown = json["unknown"];
		this->texOffsetX = json["texOffsetX"];
		this->texOffsetY = json["texOffsetY"];
		this->texWidth = json["texWidth"];
		this->texHeight = json["texHeight"];
		this->offsetX = json["offsetX"];
		this->offsetY = json["offsetY"];
		this->duration = json["duration"];
		this->renderGroup = json["renderGroup"];

		this->blendOptions.mode = json["blendMode"];
		this->blendOptions.color = json["blendColor"];
		this->blendOptions.scaleX = json["blendScaleX"];
		this->blendOptions.scaleY = json["blendScaleY"];
		this->blendOptions.flipVert = json["blendFlipVert"];
		this->blendOptions.flipHorz = json["blendFlipHorz"];
		this->blendOptions.angle = json["blendAngle"];

		this->traits.damage = json["traitDamage"];
		this->traits.proration = json["traitProration"];
		this->traits.chipDamage = json["traitChipDamage"];
		this->traits.spiritDamage = json["traitSpiritDamage"];
		this->traits.untech = json["traitUntech"];
		this->traits.power = json["traitPower"];
		this->traits.limit = json["traitLimit"];
		this->traits.onHitPlayerStun = json["traitOnHitPlayerStun"];
		this->traits.onHitEnemyStun = json["traitOnHitEnemyStun"];
		this->traits.onBlockPlayerStun = json["traitOnBlockPlayerStun"];
		this->traits.onBlockEnemyStun = json["traitOnBlockEnemyStun"];
		this->traits.onHitCardGain = json["traitOnHitCardGain"];
		this->traits.onBlockCardGain = json["traitOnBlockCardGain"];
		this->traits.onAirHitSetSequence = json["traitOnAirHitSetSequence"];
		this->traits.onGroundHitSetSequence = json["traitOnGroundHitSetSequence"];
		this->traits.speedX = json["traitSpeedX"];
		this->traits.speedY = json["traitSpeedY"];
		this->traits.onHitSfx = json["traitOnHitSfx"];
		this->traits.onHitEffect = json["traitOnHitEffect"];
		this->traits.attackLevel = json["traitAttackLevel"];
		this->traits.comboModifier = json["traitComboModifier"];
		this->traits.frameFlags = json["traitFrameFlags"];
		this->traits.attackFlags = json["traitAttackFlags"];

		this->effect.pivotX = json["effectPivotX"];
		this->effect.pivotY = json["effectPivotY"];
		this->effect.positionXExtra = json["effectPositionXExtra"];
		this->effect.positionYExtra = json["effectPositionYExtra"];
		this->effect.positionX = json["effectPositionX"];
		this->effect.positionY = json["effectPositionY"];
		this->effect.unknown02RESETSTATE = json["effectUnknown02RESETSTATE"];
		this->effect.speedX = json["effectSpeedX"];
		this->effect.speedY = json["effectSpeedY"];

		for (auto &b : json["collision"])
			this->cBoxes.push_back({
				.left = b["left"],
				.up = b["up"],
				.right = b["right"],
				.down = b["down"]
			});
		for (auto &b : json["attack"])
			this->aBoxes.push_back({
				.left = b["left"],
				.up = b["up"],
				.right = b["right"],
				.down = b["down"]
			});
		for (auto &b : json["hit"])
			this->hBoxes.push_back({
				.left = b["left"],
				.up = b["up"],
				.right = b["right"],
				.down = b["down"]
			});
	}

	FrameData::FrameData(const ShadyCore::Schema::Sequence::Frame &frame, const std::string &folder, const std::array<Color, 256> *palette, const ShadyCore::Schema &schema) :
		__folder(folder),
		__paletteData(palette),
		spritePath(schema.images.at(frame.imageIndex).name)
	{
		this->unknown = frame.unknown;
		this->texOffsetX = frame.texOffsetX;
		this->texOffsetY = frame.texOffsetY;
		this->texWidth = frame.texWidth;
		this->texHeight = frame.texHeight;
		this->offsetX = frame.offsetX;
		this->offsetY = frame.offsetY;
		this->duration = frame.duration;
		this->renderGroup = frame.renderGroup;
		this->blendOptions.mode = frame.blendOptions.mode;
		this->blendOptions.color = frame.blendOptions.color;
		this->blendOptions.scaleX = frame.blendOptions.scaleX;
		this->blendOptions.scaleY = frame.blendOptions.scaleY;
		this->blendOptions.flipVert = frame.blendOptions.flipVert;
		this->blendOptions.flipHorz = frame.blendOptions.flipHorz;
		this->blendOptions.angle = frame.blendOptions.angle;
	}

	FrameData::FrameData(const ShadyCore::Schema::Sequence::MoveFrame &frame, const std::string &folder, const std::array<Color, 256> *palette, const ShadyCore::Schema &schema) :
		ShadyCore::Schema::Sequence::MoveFrame(frame),
		__folder(folder),
		__paletteData(palette),
		spritePath(schema.images.at(frame.imageIndex).name)
	{
	}

	FrameData &FrameData::operator=(const FrameData &other)
	{
		auto old = this->textureHandle;

		this->__folder = other.__folder;
		this->__requireReload = other.__requireReload;
		this->__paletteData = other.__paletteData;
		this->spritePath = other.spritePath;
		this->textureHandle = other.textureHandle;
		if (!this->_slave) {
			game->textureMgr.addRef(this->textureHandle);
			game->textureMgr.remove(old);
		}
		return *this;
	}

	void FrameData::checkReloadTexture()
	{
		if (this->__requireReload)
			this->reloadTexture();
		this->__requireReload = false;
	}

	void FrameData::reloadTexture()
	{
		assert_exp(!this->_slave);
		game->textureMgr.remove(this->textureHandle);
		assert_exp(this->__paletteData);
		this->textureHandle = game->textureMgr.load(this->__folder + this->spritePath, *this->__paletteData);
	}

	ShadyCore::Schema::Sequence::BlendOptions FrameData::getBlendOptions() const
	{
		if (this->renderGroup == 2)
			return this->blendOptions;

		ShadyCore::Schema::Sequence::BlendOptions blend;

		if (this->renderGroup == 1) {
			blend.scaleX = 100;
			blend.scaleY = 100;
		} else {
			blend.scaleX = 200;
			blend.scaleY = 200;
		}
		blend.mode = 0;
		blend.color = 0;
		blend.flipVert = 0;
		blend.flipHorz = 0;
		blend.angle = 0;
		return blend;
	}

	nlohmann::json FrameData::toJson() const
	{
		nlohmann::json j{
			{ "collision", std::vector<std::string>() },
			{ "attack", std::vector<std::string>() },
			{ "hit", std::vector<std::string>() }
		};

		j["sprite"] = this->spritePath;
		j["unknown"] = this->unknown;
		j["texOffsetX"] = this->texOffsetX;
		j["texOffsetY"] = this->texOffsetY;
		j["texWidth"] = this->texWidth;
		j["texHeight"] = this->texHeight;
		j["offsetX"] = this->offsetX;
		j["offsetY"] = this->offsetY;
		j["duration"] = this->duration;

		j["renderGroup"] = this->renderGroup;
		j["blendMode"] = this->blendOptions.mode;
		j["blendColor"] = this->blendOptions.color;
		j["blendScaleX"] = this->blendOptions.scaleX;
		j["blendScaleY"] = this->blendOptions.scaleY;
		j["blendFlipVert"] = this->blendOptions.flipVert;
		j["blendFlipHorz"] = this->blendOptions.flipHorz;
		j["blendAngle"] = this->blendOptions.angle;

		j["traitDamage"] = this->traits.damage;
		j["traitProration"] = this->traits.proration;
		j["traitChipDamage"] = this->traits.chipDamage;
		j["traitSpiritDamage"] = this->traits.spiritDamage;
		j["traitUntech"] = this->traits.untech;
		j["traitPower"] = this->traits.power;
		j["traitLimit"] = this->traits.limit;
		j["traitOnHitPlayerStun"] = this->traits.onHitPlayerStun;
		j["traitOnHitEnemyStun"] = this->traits.onHitEnemyStun;
		j["traitOnBlockPlayerStun"] = this->traits.onBlockPlayerStun;
		j["traitOnBlockEnemyStun"] = this->traits.onBlockEnemyStun;
		j["traitOnHitCardGain"] = this->traits.onHitCardGain;
		j["traitOnBlockCardGain"] = this->traits.onBlockCardGain;
		j["traitOnAirHitSetSequence"] = this->traits.onAirHitSetSequence;
		j["traitOnGroundHitSetSequence"] = this->traits.onGroundHitSetSequence;
		j["traitSpeedX"] = this->traits.speedX;
		j["traitSpeedY"] = this->traits.speedY;
		j["traitOnHitSfx"] = this->traits.onHitSfx;
		j["traitOnHitEffect"] = this->traits.onHitEffect;
		j["traitAttackLevel"] = this->traits.attackLevel;
		j["traitComboModifier"] = this->traits.comboModifier;
		j["traitFrameFlags"] = this->traits.frameFlags;
		j["traitAttackFlags"] = this->traits.attackFlags;

		j["effectPivotX"] = this->effect.pivotX;
		j["effectPivotY"] = this->effect.pivotY;
		j["effectPositionXExtra"] = this->effect.positionXExtra;
		j["effectPositionYExtra"] = this->effect.positionYExtra;
		j["effectPositionX"] = this->effect.positionX;
		j["effectPositionY"] = this->effect.positionY;
		j["effectUnknown02RESETSTATE"] = this->effect.unknown02RESETSTATE;
		j["effectSpeedX"] = this->effect.speedX;
		j["effectSpeedY"] = this->effect.speedY;

		for (auto &b : this->cBoxes)
			j["collision"].push_back({
				{ "left", b.left },
				{ "right", b.right },
				{ "up", b.up },
				{ "down", b.down },
			});
		for (auto &b : this->aBoxes)
			j["attack"].push_back({
				{ "left", b.left },
				{ "right", b.right },
				{ "up", b.up },
				{ "down", b.down },
			});
		for (auto &b : this->hBoxes)
			j["hit"].push_back({
				{ "left", b.left },
				{ "right", b.right },
				{ "up", b.up },
				{ "down", b.down },
			});
		return j;
	}

	void FrameData::setSlave(bool slave)
	{
		if (slave == this->_slave)
			return;
		if (slave)
			game->textureMgr.remove(this->textureHandle);
		else
			game->textureMgr.addRef(this->textureHandle);
		this->_slave = slave;
	}
}

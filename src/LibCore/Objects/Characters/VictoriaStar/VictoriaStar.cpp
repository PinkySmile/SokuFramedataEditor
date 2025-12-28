//
// Created by PinkySmile on 26/02/23.
//

#include "Utils.hpp"
#include "VictoriaStar.hpp"
#include "Resources/Game.hpp"
#include "Objects/Characters/VictoriaStar/Shadow.hpp"
#include "Objects/Characters/VictoriaStar/Shadows/NeutralShadow.hpp"
#include "Objects/Characters/VictoriaStar/Shadows/MatterShadow.hpp"
#include "Objects/Characters/VictoriaStar/Shadows/SpiritShadow.hpp"
#include "Objects/Characters/VictoriaStar/Shadows/VoidShadow.hpp"
#include "VictoriaProjectile.hpp"
#include "Objects/CheckUtils.hpp"

#define SCALE_POS_Y 610
#define SCALE_MIDDLE_X 200
#define SCALE_SIZE 200
#define MAX_STACKS 1000
#define STACK_PER_KILL 10
#define PASSIVE_STACK_PER_SHADOW_TIMER 20
#define ACTION_SCREEN_TELEPORT 368
#define ACTION_SHADOW 0
#define ACTION_FLOWER 1
#define ACTION_HAPPY_BUTTERFLY 4
#define ACTION_WEIRD_BUTTERFLY 5
#define WEIRD_BUTTERFLIES_START_ID (1000 + NB_BUTTERFLIES)
#define BUTTERFLIES_END_ID (1000 + NB_BUTTERFLIES * 2)
#define FLOWER_ID BUTTERFLIES_END_ID

namespace SpiralOfFate
{
	VictoriaStar::VictoriaStar(
		unsigned int index,
		const std::string &folder,
		const std::string &palette,
		std::shared_ptr<IInput> input,
		const std::string &opName
	) :
		Character(index, folder, palette, std::move(input)),
		_stacks(MAX_STACKS / 2),
		_hudScale(game->textureMgr.load(folder + "/hud.png")),
		_hudCursor(game->textureMgr.load(folder + "/cursor.png"))
	{
		auto spriteName = "shadow_" + opName + ".png";
		auto opHandle = game->textureMgr.load(folder + "/" + spriteName);

		game->logger.debug("VictoriaStar class created");
		this->_neutralFormFramedata = this->_moves;
		this->_shadowFormFramedata = FrameData::loadFile(folder + "/flower_form_framedata.json", folder, palette);
		this->_flowerFormFramedata = FrameData::loadFile(folder + "/shadow_form_framedata.json", folder, palette);

		assert_exp(this->_shadowFormFramedata.find(ACTION_GAME_START1) != this->_shadowFormFramedata.end());
		assert_exp(this->_flowerFormFramedata.find(ACTION_GAME_START1) != this->_flowerFormFramedata.end());

		// Butterflies actions
		assert_exp(this->_subObjectsData.find(ACTION_HAPPY_BUTTERFLY) != this->_subObjectsData.end());
		assert_exp(this->_subObjectsData.find(ACTION_WEIRD_BUTTERFLY) != this->_subObjectsData.end());
		// Shadow action
		assert_exp(this->_subObjectsData.find(ACTION_SHADOW) != this->_subObjectsData.end());
		this->_shadowActions = this->_subObjectsData.at(ACTION_SHADOW);
		for (auto &block : this->_shadowActions)
			for (auto &frame : block) {
				frame.setSlave();
				frame.spritePath = spriteName;
				frame.textureHandle = opHandle;
				frame.setSlave(false);
			}
		game->textureMgr.remove(opHandle);

		auto size = this->_hudScale.getTextureSize();

		this->_hudScale.setPosition(Vector2f(SCALE_MIDDLE_X - size.x / 2, SCALE_POS_Y));
		this->_hudCursor.setPosition(Vector2f(0, SCALE_POS_Y + size.y));
	}

	bool VictoriaStar::_startMove(unsigned int action)
	{
		if (action == ACTION_BACKWARD_DASH && (this->_position.x == 0 || this->_position.x == 1000))
			return Object::_startMove(ACTION_SCREEN_TELEPORT);
		return Object::_startMove(action);
	}

	void VictoriaStar::_tickMove()
	{
		if (this->_stacksTimer)
			this->_stacksTimer--;
		Character::_tickMove();
		if ((this->_action == ACTION_CROUCHING || this->_action == ACTION_CROUCH) && !this->_flower) {
			auto result = this->_spawnSubObject(*game->battleMgr, FLOWER_ID, true);

			this->_flower = {
				result.first,
				std::shared_ptr<Flower>(result.second, reinterpret_cast<Flower *>(&*result.second))
			};
		}
		if (this->_flower && this->_flower->second->isDead())
			this->_flower.reset();
	}

	unsigned int VictoriaStar::getClassId() const
	{
		return CLASS_ID;
	}

	unsigned int VictoriaStar::getBufferSize() const
	{
		return Character::getBufferSize() + sizeof(Data) + sizeof(unsigned) * this->_shadows.size();
	}

	void VictoriaStar::copyToBuffer(void *data) const
	{
		auto dat = reinterpret_cast<Data *>(reinterpret_cast<uintptr_t>(data) + Character::getBufferSize());
		size_t i;

		Character::copyToBuffer(data);
		game->logger.verbose("Saving VictoriaStar (Data size: " + std::to_string(sizeof(Data)) + ") @" + Utils::toHex((uintptr_t)dat));
		dat->_hitShadow = this->_hitShadow;
		dat->_stacks = this->_stacks;
		dat->_stacksTimer = this->_stacksTimer;
		dat->_flower = this->_flower && !this->_flower->second->isDead() ? this->_flower->first : 0;
		for (i = 0; i < this->_shadows.size(); i++) {
			if (this->_shadows[i].first && this->_shadows[i].second)
				dat->_shadows[i] = this->_shadows[i].first;
			else
				dat->_shadows[i] = 0;
		}
	}

	void VictoriaStar::restoreFromBuffer(void *data)
	{
		Character::restoreFromBuffer(data);

		auto dat = reinterpret_cast<Data *>(reinterpret_cast<uintptr_t>(data) + Character::getBufferSize());
		size_t i;

		this->_hitShadow = dat->_hitShadow;
		this->_stacks = dat->_stacks;
		this->_stacksTimer = dat->_stacksTimer;
		if (dat->_flower == 0)
			this->_flower.reset();
		else
			this->_flower = {dat->_flower, nullptr};
		for (i = 0; i < this->_shadows.size(); i++) {
			this->_shadows[i].first = dat->_shadows[i];
			this->_shadows[i].second.reset();
		}
		game->logger.verbose("Restored VictoriaStar @" + Utils::toHex((uintptr_t)dat));
	}

	size_t VictoriaStar::printDifference(const char *msgStart, void *data1, void *data2, unsigned startOffset) const
	{
		auto length = Character::printDifference(msgStart, data1, data2, startOffset);

		if (length == 0)
			return 0;

		auto dat1 = reinterpret_cast<Data *>(reinterpret_cast<uintptr_t>(data1) + length);
		auto dat2 = reinterpret_cast<Data *>(reinterpret_cast<uintptr_t>(data2) + length);

		game->logger.info("VictoriaStar @" + std::to_string(startOffset + length));
		OBJECT_CHECK_FIELD("VictoriaStar", "", dat1, dat2, _hitShadow, DISP_BOOL);
		OBJECT_CHECK_FIELD("VictoriaStar", "", dat1, dat2, _stacks, std::to_string);
		OBJECT_CHECK_FIELD("VictoriaStar", "", dat1, dat2, _stacksTimer, std::to_string);
		OBJECT_CHECK_FIELD("VictoriaStar", "", dat1, dat2, _flower, std::to_string);
		OBJECT_CHECK_FIELD("VictoriaStar", "", dat1, dat2, _shadows[0], std::to_string);
		OBJECT_CHECK_FIELD("VictoriaStar", "", dat1, dat2, _shadows[1], std::to_string);
		OBJECT_CHECK_FIELD("VictoriaStar", "", dat1, dat2, _shadows[2], std::to_string);
		OBJECT_CHECK_FIELD("VictoriaStar", "", dat1, dat2, _shadows[3], std::to_string);
		return length + sizeof(Data);
	}

	void VictoriaStar::update()
	{
		if (this->_currentForm != 1 && this->_stacks == 0) {
			this->_moves = this->_shadowFormFramedata;
			this->_forceStartMove(ACTION_GAME_START1);
			this->_currentForm = 1;
		} else if (this->_currentForm != 2 && this->_stacks == MAX_STACKS) {
			this->_moves = this->_flowerFormFramedata;
			this->_forceStartMove(ACTION_GAME_START1);
			this->_currentForm = 2;
		} else if (this->_currentForm != 0 && this->_stacks > 0 && this->_stacks < MAX_STACKS) {
			this->_moves = this->_neutralFormFramedata;
			this->_forceStartMove(ACTION_IDLE);
			this->_currentForm = 0;
		}
		Character::update();
	}

	void VictoriaStar::postUpdate()
	{
		float v = 0;

		Character::postUpdate();
		for (auto &s : this->_shadows) {
			if (!s.first)
				continue;
			if (s.second->isDead()) {
				if (s.second->wasKilledByOwner())
					v += STACK_PER_KILL;
				s.second.reset();
				s.first = 0;
			} else if (this->_stacksTimer == 0)
				v -= s.second->getCurrentPoints();
		}

		if (this->_stacks == 0 || this->_stacks == MAX_STACKS);
		else if (v < 0) {
			if (this->_stacks < -v)
				this->_stacks = 0;
			else if (this->_stacks >= MAX_STACKS * 3 / 4 && this->_stacks + v < MAX_STACKS * 3 / 4)
				this->_stacks = MAX_STACKS * 3 / 4;
			else
				this->_stacks += v;
		} else {
			if (this->_stacks + v > MAX_STACKS)
				this->_stacks = MAX_STACKS;
			else if (this->_stacks <= MAX_STACKS / 4 && this->_stacks + v > MAX_STACKS / 4)
				this->_stacks = MAX_STACKS / 4;
			else
				this->_stacks += v;
		}
		if (this->_stacksTimer == 0)
			this->_stacksTimer = PASSIVE_STACK_PER_SHADOW_TIMER;
	}

	static std::unordered_map<std::string, Shadow *(*)(
		const std::vector<std::vector<FrameData>> &frameData,
		unsigned int hp,
		bool direction,
		Vector2f pos,
		bool owner,
		Character *ownerObj,
		unsigned int id,
		bool tint
	)> shadowConstructors{
		{"neutral", NeutralShadow::create },
		{"matter", MatterShadow::create },
		{"spirit", SpiritShadow::create },
		{"void", VoidShadow::create },
	};
	static std::unordered_map<std::string, unsigned> shadowIndex{
		{"neutral", 0 },
		{"matter",  1 },
		{"spirit",  2 },
		{"void",    3 },
	};

	std::pair<unsigned int, std::shared_ptr<Object>> VictoriaStar::_spawnSubObject(BattleManager &manager, unsigned int id, bool needRegister)
	{
		// Butterflies
		if (BUTTERFLIES_START_ID <= id && id < BUTTERFLIES_END_ID)
			return manager.registerObject<Butterfly>(
				needRegister,
				this,
				this->_opponent,
				this->_team,
				id < WEIRD_BUTTERFLIES_START_ID ? nullptr : this->_happyBufferFlies[id - WEIRD_BUTTERFLIES_START_ID].second,
				this->_subObjectsData.at(id < WEIRD_BUTTERFLIES_START_ID ? ACTION_HAPPY_BUTTERFLY : ACTION_WEIRD_BUTTERFLY),
				id
			);
		if (id == FLOWER_ID)
			return manager.registerObject<Flower>(
				needRegister,
				this,
				this->_subObjectsData.at(ACTION_FLOWER),
				this->_direction,
				this->_position + Vector2f{30 * this->_dir, 0},
				this->_team,
				id
			);
		assert_msg(this->_projectileData.find(id) != this->_projectileData.end(), "Cannot find subobject " + std::to_string(id));

		auto &pdat = this->_projectileData[id];
		bool dir = this->_getProjectileDirection(pdat);

		if (!pdat.json.contains("shadow")) {
			Projectile::TypeSwitch flags = Projectile::TYPESWITCH_NONE;

			if (this->_neutralEffectTimer)
				flags = Projectile::TYPESWITCH_NON_TYPED;
			if (this->_installMoveStarted) {
				if (this->_hasVoidInstall)
					flags = Projectile::TYPESWITCH_VOID;
				if (this->_hasMatterInstall)
					flags = Projectile::TYPESWITCH_MATTER;
				if (this->_hasSpiritInstall)
					flags = Projectile::TYPESWITCH_SPIRIT;
			}
			try {
				return manager.registerObject<VictoriaProjectile>(
					needRegister,
					this->_subObjectsData.at(pdat.action),
					this->_team,
					dir,
					this->_calcProjectilePosition(pdat, dir ? 1 : -1),
					this->_team,
					this,
					id,
					pdat.json,
					flags,
					this->getDebuffDuration()
				);
			} catch (std::out_of_range &e) {
				throw std::invalid_argument("Cannot find subobject action id " + std::to_string(id));
			}
		}

		std::string neutral = pdat.json["shadow"];
		unsigned index = shadowIndex.at(neutral);

		if (this->_shadows[index].first)
			return {0, nullptr};

		try {
			unsigned tint = pdat.json["tint"];
			auto obj = std::shared_ptr<Shadow>(shadowConstructors.at(neutral)(
				tint ? this->_shadowActions : this->_subObjectsData.at(ACTION_SHADOW),
				pdat.json["hp"],
				dir,
				this->_calcProjectilePosition(pdat, dir ? 1 : -1),
				this->_team,
				this,
				id,
				tint
			));

			if (!needRegister)
				return {0, obj};

			auto objectId = manager.registerObject(static_cast<std::shared_ptr<Object>>(obj));

			this->_shadows[index] = {objectId, obj};
			return this->_shadows[index];
		} catch (std::out_of_range &e) {
			throw std::invalid_argument("Invalid shadow type '" + pdat.json["shadow"].get<std::string>() + "'");
		}
	}

	void VictoriaStar::getHit(Object &other, const FrameData *data)
	{
		auto old = this->_hasHit;

		if (reinterpret_cast<VictoriaStar *>(&other)->_team == 4)
			this->_hasHit = false;
		Character::getHit(other, data);
		this->_hasHit = old;
	}

	bool VictoriaStar::_canStartMove(unsigned int action, const FrameData &data)
	{
		return Character::_canStartMove(action, data);
	}

	bool VictoriaStar::hits(const Object &other) const
	{
		auto old = this->_hasHit;
		auto t = const_cast<VictoriaStar *>(this);
		auto shadow = dynamic_cast<const Shadow *>(&other);

		if (shadow && shadow->getOwner() == this->_team)
			t->_hasHit = false;

		auto result = Character::hits(other);

		t->_hasHit = old;
		return result;
	}

	void VictoriaStar::hit(Object &other, const FrameData *data)
	{
		auto shadow = dynamic_cast<const Shadow *>(&other);

		if (!shadow || shadow->getOwner() != this->_team)
			return Character::hit(other, data);
		this->_speed.x += data->pushBack * -this->_dir;
		if (!this->_hasHit && this->getCurrentFrameData()->oFlag.nextBlockOnHit) {
			this->_actionBlock++;
			assert_msg(this->_actionBlock != this->_moves.at(this->_action).size(), "Action " + actionToString(this->_action) + " is missing block " + std::to_string(this->_actionBlock));
			this->_animationCtr = 0;
			Object::_onMoveEnd(*data);
		}
		this->_hitShadow = true;
	}

	void VictoriaStar::_blockMove(Object *other, const FrameData &data)
	{
		this->_target = other;
		Character::_blockMove(other, data);
		this->_target = nullptr;
	}

	void VictoriaStar::_forceStartMove(unsigned int action)
	{
		if (isBlockingAction(action) && this->_target) {
			for (auto &butterfly: this->_happyBufferFlies)
				butterfly.second->defensiveFormation(*this->_target);
			for (auto &butterfly : this->_weirdBufferFlies)
				butterfly.second->defensiveFormation(*this->_target);
		}
		if (action == ACTION_FORWARD_DASH)
			for (auto &shadow : this->_shadows)
				if (shadow.first)
					shadow.second->setInvincible(40);
		this->_hitShadow = false;
		Character::_forceStartMove(action);
	}

	bool VictoriaStar::_canCancel(unsigned int action)
	{
		auto old = this->_hasHit;

		this->_hasHit |= this->_hitShadow;

		auto result = Character::_canCancel(action);

		this->_hasHit = old;
		return result;
	}

	void VictoriaStar::resolveSubObjects(const BattleManager &manager)
	{
		for (auto &butterfly : this->_happyBufferFlies)
			butterfly.second = reinterpret_cast<Butterfly *>(&*manager.getObjectFromId(butterfly.first));
		for (unsigned i = 0; i < this->_weirdBufferFlies.size(); i++) {
			auto &butterfly = this->_weirdBufferFlies[i];

			butterfly.second = reinterpret_cast<Butterfly *>(&*manager.getObjectFromId(butterfly.first));
			butterfly.second->_copy = &*this->_happyBufferFlies[i].second;
		}
		for (auto &shadow : this->_shadows) {
			if (!shadow.first)
				continue;

			auto obj = manager.getObjectFromId(shadow.first);

			shadow.second = std::shared_ptr<Shadow>(obj, reinterpret_cast<Shadow *>(&*obj));
		}
		if (this->_flower) {
			auto obj = manager.getObjectFromId(this->_flower->first);

			this->_flower->second = std::shared_ptr<Flower>(obj, reinterpret_cast<Flower *>(&*obj));
		}
		Character::resolveSubObjects(manager);
	}

	void VictoriaStar::init(BattleManager &manager, const Character::InitData &data)
	{
		Character::init(manager, data);
		for (unsigned i = 0; i < this->_happyBufferFlies.size(); i++) {
			auto result = this->_spawnSubObject(manager, BUTTERFLIES_START_ID + i, true);

			this->_happyBufferFlies[i] = {
				result.first,
				reinterpret_cast<Butterfly *>(&*result.second)
			};

			result = this->_spawnSubObject(manager, BUTTERFLIES_START_ID + this->_happyBufferFlies.size() + i,true);
			this->_weirdBufferFlies[i] = {
				result.first,
				reinterpret_cast<Butterfly *>(&*result.second)
			};
		}
	}

	void VictoriaStar::_applyNewAnimFlags()
	{
		if (!this->_newAnim)
			return;

		auto data = this->getCurrentFrameData();

		Character::_applyNewAnimFlags();
		if (data->specialMarker == 15) {
			for (size_t i = 0; i < this->_happyBufferFlies.size(); i++)
				this->_happyBufferFlies[i].second->startAttack(
					this->_getButterflyAttackPos(i),
					data->blockPlayerHitStop,
					data->blockOpponentHitStop,
					data->hitOpponentHitStop,
					data->hitPlayerHitStop
				);
			return;
		}
	}

	void VictoriaStar::drawSpecialHUD(sf::RenderTarget &texture)
	{
		texture.draw(this->_hudScale);
		this->_hudCursor.setPosition(Vector2i(
			(SCALE_MIDDLE_X - SCALE_SIZE / 2) + (this->_stacks * SCALE_SIZE / MAX_STACKS) - this->_hudCursor.getTextureSize().x / 2,
			this->_hudCursor.getPosition().y
		));
		texture.draw(this->_hudCursor);
		Character::drawSpecialHUD(texture);
	}

	Vector2f VictoriaStar::_getButterflyAttackPos(unsigned int id)
	{
		if (this->_action == ACTION_6S)
			return {
				840.f * this->_direction + id * (160 / NB_BUTTERFLIES),
				-20
			};
		return {0, 0};
	}

	size_t VictoriaStar::printContent(const char *msgStart, void *data, unsigned int startOffset, size_t dataSize) const
	{
		auto length = Character::printContent(msgStart, data, startOffset, dataSize);

		if (length == 0)
			return 0;

		auto dat = reinterpret_cast<Data *>(reinterpret_cast<uintptr_t>(data) + length);

		game->logger.info("VictoriaStar @" + std::to_string(startOffset + length));
		if (startOffset + length + sizeof(Data) >= dataSize)
			game->logger.warn("Object is " + std::to_string(startOffset + length + sizeof(Data) - dataSize) + " bytes bigger than input");
		DISPLAY_FIELD("VictoriaStar", "", dat, _hitShadow, DISP_BOOL);
		DISPLAY_FIELD("VictoriaStar", "", dat, _stacks, std::to_string);
		DISPLAY_FIELD("VictoriaStar", "", dat, _stacksTimer, std::to_string);
		DISPLAY_FIELD("VictoriaStar", "", dat, _flower, std::to_string);
		DISPLAY_FIELD("VictoriaStar", "", dat, _shadows[0], std::to_string);
		DISPLAY_FIELD("VictoriaStar", "", dat, _shadows[1], std::to_string);
		DISPLAY_FIELD("VictoriaStar", "", dat, _shadows[2], std::to_string);
		DISPLAY_FIELD("VictoriaStar", "", dat, _shadows[3], std::to_string);
		return length + sizeof(Data);
	}
}
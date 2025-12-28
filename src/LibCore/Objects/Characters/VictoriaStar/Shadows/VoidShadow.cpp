//
// Created by PinkySmile on 06/05/23.
//

#include "VoidShadow.hpp"
#include "Resources/Game.hpp"
#include "Objects/Characters/VictoriaStar/VictoriaStar.hpp"
#include "Utils.hpp"
#include "Objects/CheckUtils.hpp"

namespace SpiralOfFate
{
	VoidShadow::VoidShadow(
		const std::vector<std::vector<FrameData>> &frameData,
		unsigned int hp,
		bool direction,
		Vector2f pos,
		bool owner,
		class Character *ownerObj,
		unsigned int id,
		bool tint
	) :
		Shadow(frameData, hp, direction, pos, owner, ownerObj, id, tint ? sf::Color{0x67, 0x03, 0x3D} : sf::Color::White, ANIMBLOCK_VOID_ACTIVATED)
	{
	}

	Shadow *VoidShadow::create(
		const std::vector<std::vector<FrameData>> &frameData,
		unsigned int hp,
		bool direction,
		Vector2f pos,
		bool owner,
		class Character *ownerObj,
		unsigned int id,
		bool tint
	)
	{
		return new VoidShadow(frameData, hp, direction, pos, owner, ownerObj, id, tint);
	}

	void VoidShadow::_onMoveEnd(const FrameData &lastData)
	{
		if (this->_actionBlock == this->_activateBlock) {
			this->_actionBlock = ANIMBLOCK_IDLE;
			this->_idleCounter = 0;
			this->_boxSize = 0;
			return Object::_onMoveEnd(lastData);
		}
		Shadow::_onMoveEnd(lastData);
	}

	void VoidShadow::update()
	{
		if (this->_hitStop) {
			this->_hitStop--;
			return;
		}
		Shadow::update();
		if (this->_idleCounter < 120)
			return;

		auto color = this->_sprite.getColor();
		auto opponent = reinterpret_cast<VoidShadow *>(this->getOwner() ? game->battleMgr->getLeftCharacter() : game->battleMgr->getRightCharacter());

		if (!this->_attacking) {
			if (color.a <= 225)
				color.a += 30;
			else
				color.a = 255;
			this->_sprite.setColor(color);
			if (this->_actionBlock != ANIMBLOCK_IDLE)
				return;

			if (!VictoriaStar::isHitAction(opponent->_action))
				return;
			this->_attacking = true;
			this->_animationCtr = 0;
			this->_animation = 0;
			this->_invincibleTime = 60;
			this->_actionBlock = this->_activateBlock;
		}
		if (color.a < 30) {
			color.a = 0;
			this->_position = opponent->_position;
			this->_attacking = false;
		} else
			color.a -= 30;
		this->_sprite.setColor(color);
	}

	unsigned int VoidShadow::getBufferSize() const
	{
		return Shadow::getBufferSize() + sizeof(Data);
	}

	void VoidShadow::copyToBuffer(void *data) const
	{
		auto dat = reinterpret_cast<Data *>(reinterpret_cast<uintptr_t>(data) + Shadow::getBufferSize());

		Shadow::copyToBuffer(data);
		game->logger.verbose("Saving VoidShadow (Data size: " + std::to_string(sizeof(Data)) + ") @" + Utils::toHex((uintptr_t)dat));
		dat->_attacking = this->_attacking;
	}

	void VoidShadow::restoreFromBuffer(void *data)
	{
		Shadow::restoreFromBuffer(data);

		auto dat = reinterpret_cast<Data *>(reinterpret_cast<uintptr_t>(data) + Shadow::getBufferSize());

		this->_attacking = dat->_attacking;
		game->logger.verbose("Restored VictoriaStar @" + Utils::toHex((uintptr_t)dat));
	}

	size_t VoidShadow::printDifference(const char *msgStart, void *data1, void *data2, unsigned startOffset) const
	{
		auto length = Shadow::printDifference(msgStart, data1, data2, startOffset);

		if (length == 0)
			return 0;

		auto dat1 = reinterpret_cast<Data *>(reinterpret_cast<uintptr_t>(data1) + length);
		auto dat2 = reinterpret_cast<Data *>(reinterpret_cast<uintptr_t>(data2) + length);

		game->logger.info("VoidShadow @" + std::to_string(startOffset + length));
		OBJECT_CHECK_FIELD("VoidShadow", "", dat1, dat2, _attacking, DISP_BOOL);
		return length + sizeof(Data);
	}

	size_t VoidShadow::printContent(const char *msgStart, void *data, unsigned int startOffset, size_t dataSize) const
	{
		auto length = Shadow::printContent(msgStart, data, startOffset, dataSize);

		if (length == 0)
			return 0;

		auto dat = reinterpret_cast<Data *>(reinterpret_cast<uintptr_t>(data) + length);

		game->logger.info("VoidShadow @" + std::to_string(startOffset + length));
		if (startOffset + length + sizeof(Data) >= dataSize)
			game->logger.warn("Object is " + std::to_string(startOffset + length + sizeof(Data) - dataSize) + " bytes bigger than input");
		DISPLAY_FIELD("VoidShadow", "", dat, _attacking, DISP_BOOL);
		if (startOffset + length + sizeof(Data) >= dataSize) {
			game->logger.fatal("Invalid input frame");
			return 0;
		}
		return length + sizeof(Data);
	}
}

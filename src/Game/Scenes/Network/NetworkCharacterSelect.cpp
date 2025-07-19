//
// Created by PinkySmile on 26/08/2022.
//

#include "NetworkCharacterSelect.hpp"
#include "Resources/Game.hpp"

namespace SpiralOfFate
{
	NetworkCharacterSelect::NetworkCharacterSelect() :
		CharacterSelect(nullptr, nullptr, ""),
		_localRealInput{new NetworkInput(*game->activeNetInput, *game->connection)},
		_remoteRealInput{new RemoteInput(*game->connection)},
		_localInput{new DelayInput(this->_localRealInput)},
		_remoteInput{new DelayInput(this->_remoteRealInput)}
	{
		this->_localInput->setDelay(CHARACTER_SELECT_DELAY);
		this->_remoteInput->setDelay(CHARACTER_SELECT_DELAY);
	}

	NetworkCharacterSelect::NetworkCharacterSelect(
		int leftPos, int rightPos,
		int leftPalette, int rightPalette,
		int stage, int platformCfg
	) :
		CharacterSelect(nullptr, nullptr, leftPos, rightPos, leftPalette, rightPalette, stage, platformCfg, ""),
		_localRealInput{new NetworkInput(*game->activeNetInput, *game->connection)},
		_remoteRealInput{new RemoteInput(*game->connection)},
		_localInput{new DelayInput(this->_localRealInput)},
		_remoteInput{new DelayInput(this->_remoteRealInput)}
	{
		this->_localInput->setDelay(CHARACTER_SELECT_DELAY);
		this->_remoteInput->setDelay(CHARACTER_SELECT_DELAY);
	}

	void NetworkCharacterSelect::update()
	{
		this->_localInput->fillBuffer();
		this->_remoteInput->fillBuffer();
		this->_remoteRealInput->refreshInputs();
		if (this->_remoteInput->hasInputs()) {
			this->_leftInput->update();
			this->_rightInput->update();

			if (this->_quit) {
				game->connection->quitGame();
				game->connection->terminate();
				return game->scene.switchScene("title_screen");
			}
			if (this->_selectingStage)
				this->_selectStageUpdate();
			else
				this->_selectCharacterUpdate();
		}
	}

	void NetworkCharacterSelect::render() const
	{
		CharacterSelect::render();
		game->screen->textSize(20);
		if (!game->connection)
			return;
		game->screen->displayElement(game->connection->getNames().first, {0, 2}, 540, Screen::ALIGN_CENTER);
		game->screen->displayElement(game->connection->getNames().second, {1140, 2}, 540, Screen::ALIGN_CENTER);
		game->screen->textSize(30);
		game->screen->displayElement("Delay " + std::to_string(game->connection->getCurrentDelay()), {0, 920});
	}

	std::shared_ptr<RemoteInput> NetworkCharacterSelect::getRemoteRealInput()
	{
		return this->_remoteRealInput;
	}

	void NetworkCharacterSelect::flushInputs(unsigned int delay)
	{
		this->_localInput->flush(delay);
		this->_remoteInput->flush(delay);
	}
}
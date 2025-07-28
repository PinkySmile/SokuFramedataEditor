//
// Created by PinkySmile on 05/03/2022.
//

#include <cstring>
#include "ReplayInput.hpp"
#include "Logger.hpp"
#include "Resources/Game.hpp"
#include "Utils.hpp"

namespace SpiralOfFate
{
	ReplayInput::ReplayInput(const std::deque<ReplayData> &inputs) :
		_inputs(inputs)
	{
		this->_keyStates.reset();
		this->_keyDuration.fill(0);
		this->_fillStates();
		for (auto &input : this->_inputs)
			this->_totalTime += input.time + 1;
	}

	bool ReplayInput::isPressed(InputEnum input) const
	{
		if (input == INPUT_RIGHT)
			return (this->_keyDuration[INPUT_RIGHT] - this->_keyDuration[INPUT_LEFT]) > 0;
		if (input == INPUT_LEFT)
			return (this->_keyDuration[INPUT_RIGHT] - this->_keyDuration[INPUT_LEFT]) < 0;
		return this->_keyStates[input];
	}

	InputStruct ReplayInput::getInputs() const
	{
		return {
			this->_keyDuration[INPUT_RIGHT] - this->_keyDuration[INPUT_LEFT],
			this->_keyDuration[INPUT_UP] - this->_keyDuration[INPUT_DOWN],
			this->_keyDuration[INPUT_N],
			this->_keyDuration[INPUT_M],
			this->_keyDuration[INPUT_S],
			this->_keyDuration[INPUT_V],
			this->_keyDuration[INPUT_A],
			this->_keyDuration[INPUT_D],
			false
		};
	}

	void ReplayInput::update()
	{
		if (!this->_inputs.empty()) {
			this->_totalTime--;
			if (this->_inputs.front().time == 0)
				this->_inputs.pop_front();
			else
				this->_inputs.front().time--;
			this->_fillStates();
		}
		for (size_t i = 0; i < this->_keyStates.size(); i++)
			if (this->_keyStates[i])
				this->_keyDuration[i]++;
			else
				this->_keyDuration[i] = 0;
	}

	void ReplayInput::consumeEvent(const sf::Event &)
	{
	}

	std::string ReplayInput::getName() const
	{
		return "Replay input";
	}

	std::vector<std::string> ReplayInput::getKeyNames() const
	{
		return std::vector<std::string>{INPUT_NUMBER};
	}

	void ReplayInput::_fillStates()
	{
		if (this->_inputs.empty()) {
			this->_keyStates.reset();
			return;
		}
		this->_keyStates[INPUT_RIGHT] = this->_inputs.front()._h > 0;
		this->_keyStates[INPUT_LEFT] = this->_inputs.front()._h < 0;
		this->_keyStates[INPUT_UP] = this->_inputs.front()._v > 0;
		this->_keyStates[INPUT_DOWN] = this->_inputs.front()._v < 0;
		this->_keyStates[INPUT_N] = this->_inputs.front().n;
		this->_keyStates[INPUT_M] = this->_inputs.front().m;
		this->_keyStates[INPUT_S] = this->_inputs.front().s;
		this->_keyStates[INPUT_V] = this->_inputs.front().v;
		this->_keyStates[INPUT_A] = this->_inputs.front().a;
		this->_keyStates[INPUT_D] = this->_inputs.front().d;
	}

	bool ReplayInput::hasData() const
	{
		return !this->_inputs.empty();
	}

	size_t ReplayInput::getRemainingTime() const
	{
		return this->_totalTime;
	}

	size_t ReplayInput::getBufferSize() const
	{
		return sizeof(Data) + this->_inputs.size() * sizeof(*Data::inputs);
	}

	void ReplayInput::copyToBuffer(unsigned char *buffer) const
	{
		auto data = reinterpret_cast<Data *>(buffer);

		game->logger.verbose("Saving ReplayInput (Data size: " + std::to_string(sizeof(Data)) + ") @" + Utils::toHex((uintptr_t)buffer));
		data->totalTime = this->_totalTime;
		data->keyStates = this->_keyStates.to_ulong();
		data->keyDuration = this->_keyDuration;
		data->nbInputs = this->_inputs.size();
		for (size_t i = 0; i < this->_inputs.size(); i++)
			data->inputs[i] = this->_inputs[i];
	}

	void ReplayInput::restoreFromBuffer(unsigned char *buffer)
	{
		auto data = reinterpret_cast<Data *>(buffer);

		this->_totalTime = data->totalTime;
		this->_keyStates = data->keyStates;
		this->_keyDuration = data->keyDuration;
		this->_inputs.resize(data->nbInputs);
		for (size_t i = 0; i < this->_inputs.size(); i++)
			this->_inputs[i] = data->inputs[i];
		game->logger.verbose("Restored ReplayInput @" + Utils::toHex((uintptr_t)buffer));
	}

	size_t ReplayInput::getBufferSizeLight() const
	{
		return sizeof(DataLight);
	}

	void ReplayInput::copyToBufferLight(unsigned char *buffer) const
	{
		auto data = reinterpret_cast<DataLight *>(buffer);

		game->logger.verbose("Saving ReplayInput (Light) (Data size: " + std::to_string(sizeof(DataLight)) + ") @" + Utils::toHex((uintptr_t)buffer));
		data->totalTime = this->_totalTime;
		data->keyStates = this->_keyStates.to_ulong();
		data->keyDuration = this->_keyDuration;
		data->lastInputs = this->_inputs.front();
	}

	void ReplayInput::restoreFromBufferLight(unsigned char *buffer)
	{
		auto data = reinterpret_cast<DataLight *>(buffer);

		this->_totalTime = data->totalTime;
		this->_keyStates = data->keyStates;
		this->_keyDuration = data->keyDuration;
		if (data->lastInputs.time == 0)
			this->_inputs.push_front(data->lastInputs);
		else
			this->_inputs.front() = data->lastInputs;
		game->logger.verbose("Restored ReplayInput (Light) @" + Utils::toHex((uintptr_t)buffer));
	}
}

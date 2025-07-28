//
// Created by PinkySmile on 05/03/2022.
//

#ifndef SOFGV_REPLAYINPUT_HPP
#define SOFGV_REPLAYINPUT_HPP


#include <thread>
#include <SFML/Network.hpp>
#include <array>
#include <list>
#include <bitset>
#include <deque>
#include "IInput.hpp"
#include "Resources/Battle/BattleManager.hpp"

namespace SpiralOfFate
{
	class ReplayInput : public IInput {
	private:
	#pragma pack(push, 1)
		// Stores the entire state
		struct Data {
			size_t nbInputs;
			size_t totalTime;
			unsigned long keyStates;
			std::array<int, INPUT_NUMBER - 1> keyDuration;
			ReplayData inputs[];
		};
		// Stores state to go back one frame (and one frame only)
		struct DataLight {
			size_t totalTime;
			unsigned long keyStates;
			std::array<int, INPUT_NUMBER - 1> keyDuration;
			ReplayData lastInputs;
		};
	#pragma pack(pop)

		size_t _totalTime = 0;
		std::deque<ReplayData> _inputs;
		std::bitset<INPUT_NUMBER - 1> _keyStates;
		std::array<int, INPUT_NUMBER - 1> _keyDuration;

		void _fillStates();

	public:
		ReplayInput(const std::deque<ReplayData> &inputs);
		bool isPressed(InputEnum input) const override;
		InputStruct getInputs() const override;
		void update() override;
		void consumeEvent(const sf::Event &event) override;
		std::string getName() const override;
		std::vector<std::string> getKeyNames() const override;
		bool hasData() const;
		size_t getRemainingTime() const;

		size_t getBufferSize() const;
		void copyToBuffer(unsigned char *) const;
		void restoreFromBuffer(unsigned char *);

		size_t getBufferSizeLight() const;
		void copyToBufferLight(unsigned char *) const;
		void restoreFromBufferLight(unsigned char *);
	};
}



#endif //SOFGV_REPLAYINPUT_HPP

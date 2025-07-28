//
// Created by PinkySmile on 20/03/2022.
//

#ifndef SOFGV_REPLAYINGAME_HPP
#define SOFGV_REPLAYINGAME_HPP


#include "PracticeInGame.hpp"
#include "CharacterSelect.hpp"

namespace SpiralOfFate
{
	// FIXME: Create a forward header with all classes' forward declarations
	class ReplayInput;

	class ReplayInGame : public PracticeInGame {
	protected:
		constexpr static const char *_menuStrings[] = {
			"Resume",
			"P1 Move list",
			"P2 Move list",
			"Replay options",
			"Return to title screen"
		};
		constexpr static const char *_practiceMenuStrings[] = {
			"Hitboxes: %s",
			"Debug: %s",
			"Input display: %s",
		};

		Character *_chr;
		size_t _startTime = 0;
		class ReplayInput *_p1;
		class ReplayInput *_p2;
		std::vector<std::vector<unsigned char>> _savedFrames;

		void _pauseUpdate() override;
		void _practiceUpdate() override;
		void _renderPause() const override;
		bool _pauseConfirm() override;
		void _practiceRender() const override;
		bool _practiceConfirm() override;
		std::vector<unsigned char> _saveState() override;
		void _restoreState(unsigned char *buffer) override;

	public:
		struct Arguments : public InGame::Arguments {
			unsigned frameCount;
		};

		ReplayInGame(const GameParams &params, unsigned frameCount, const std::vector<struct PlatformSkeleton> &platforms, const struct StageEntry &stage, Character *leftChr, Character *rightChr, unsigned licon, unsigned ricon, const nlohmann::json &lJson, const nlohmann::json &rJson);
		void update() override;
		void consumeEvent(const sf::Event &event) override;
		void render() const override;

		static ReplayInGame *create(SceneArguments *);
	};
}


#endif //SOFGV_REPLAYINGAME_HPP

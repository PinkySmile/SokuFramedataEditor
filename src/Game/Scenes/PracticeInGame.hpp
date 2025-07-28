//
// Created by PinkySmile on 18/02/2022.
//

#ifndef SOFGV_PRACTICEINGAME_HPP
#define SOFGV_PRACTICEINGAME_HPP


#include <memory>
#include "InGame.hpp"
#include "Utils.hpp"

namespace SpiralOfFate
{
	class PracticeInGame : public InGame {
	protected:
		float _time = 0;
		bool _step = false;
		bool _next = false;
		std::vector<unsigned char> _startingState;
		std::optional<std::vector<unsigned char>> _savedState;
		unsigned char _speed = 60;

		class PracticeBattleManager *_manager;
		bool _replay = false;
		unsigned _practiceCursor = 0;
		bool _practice = false;
		unsigned char _inputDisplay = 0;
		unsigned char _block = 0;
		unsigned char _inputDelay = 0;
		unsigned char _guardBar = 0;
		unsigned char _overdrive = 0;
		unsigned _rightCounter = 0;
		unsigned char _mana = 0;

		constexpr static const char *_menuStrings[] = {
			"Resume",
			"Move list",
			"Practice options",
			"Return to character select",
			"Return to title screen"
		};
		constexpr static const char *_practiceMenuStrings[] = {
			"Dummy ground tech: %s",
			"Dummy air tech: %s",
			"Dummy state: %s",
			"Dummy block: %s",
			"Input delay: %s",
			"Counter hit: %s",
			"Guard bar: %s",
			"Overdrive: %s",
			"Hitboxes: %s",
			"Debug: %s",
			"Mana: %s",
			"Input display: %s",
		};
		static bool isOnLastFrame(const Character &chr);
		static bool hasControl(const Character &chr);
		char const *dummyGroundTechToString() const;
		char const *dummyAirTechToString() const;
		char const *dummyStateToString() const;
		char const *blockToString() const;
		char const *manaStateToString() const;
		void _renderPause() const override;
		void _pauseUpdate() override;
		bool _pauseConfirm() override;
		void _updateLoop();

		virtual void _practiceRender() const;
		virtual void _practiceUpdate();
		virtual bool _practiceConfirm();

		virtual std::vector<unsigned char> _saveState();
		virtual void _restoreState(unsigned char *buffer);

	public:
		PracticeInGame(
			const GameParams &params,
			const std::vector<struct PlatformSkeleton> &platforms,
			const struct StageEntry &stage,
			Character *leftChr, Character *rightChr,
			unsigned licon, unsigned ricon,
			const nlohmann::json &lJson, const nlohmann::json &rJson,
			bool saveState = true
		);
		void update() override;
		void render() const override;
		void consumeEvent(const Event &event) override;

		static PracticeInGame *create(SceneArguments *);
	};
}


#endif //SOFGV_PRACTICEINGAME_HPP

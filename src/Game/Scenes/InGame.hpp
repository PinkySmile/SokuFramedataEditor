//
// Created by PinkySmile on 24/09/2021.
//

#ifndef SOFGV_INGAME_HPP
#define SOFGV_INGAME_HPP


#include <memory>
#include <random>
#include "Resources/IScene.hpp"
#include "Inputs/IInput.hpp"
#include "Objects/Characters/Character.hpp"
#include "Resources/Assets/MoveListData.hpp"
#include "Resources/Battle/RandomWrapper.hpp"
#include "Objects/FakeObject.hpp"
#include "Resources/SceneArgument.hpp"
#include "SkeletonData.hpp"

namespace SpiralOfFate
{
	class InGame : public IScene {
	public:
		struct GameParams {
			unsigned stage;
			unsigned music;
			unsigned platforms;
		};

		struct InitParams {
			GameParams params;
			std::vector<PlatformSkeleton> platforms;
			StageEntry stage;
			Character *leftChr;
			Character *rightChr;
			unsigned licon;
			unsigned ricon;
			nlohmann::json lJson;
			nlohmann::json rJson;
		};

		struct Arguments : public SceneArguments {
			GameStartParams params;
			std::vector<StageEntry> stages;
			std::vector<CharacterEntry> entries;
			std::shared_ptr<IInput> leftInput;
			std::shared_ptr<IInput> rightInput;
			std::string endScene;
			bool saveReplay;
		};

		InGame(
			const GameParams &params,
			const std::vector<PlatformSkeleton> &platforms,
			const StageEntry &stage,
			Character *leftChr,
			Character *rightChr,
			unsigned licon,
			unsigned ricon,
			const nlohmann::json &lJson,
			const nlohmann::json &rJson,
			bool saveReplay,
			const std::string &endScene = "char_select"
		);
		~InGame() override;
		void saveReplay();
		void render() const override;
		void update() override;
		void consumeEvent(const sf::Event &event) override;

		static InGame *create(SceneArguments *);
		static Character *createCharacter(const CharacterEntry &entry, const CharacterEntry &entryOp, int pos, int palette, std::shared_ptr<IInput> input);
		static InGame::InitParams createParams(
			std::vector<StageEntry> &stages,
			std::vector<CharacterEntry> &entries,
			std::function<void (const std::wstring &)> reportProgressW,
			const GameStartParams &params,
			std::shared_ptr<IInput> leftInput,
			std::shared_ptr<IInput> rightInput
		);

	protected:
		constexpr static const char *_menuStrings[] = {
			"Resume",
			"Move list",
			"Command list",
			"Return to character select",
			"Return to title screen"
		};
		const std::unordered_map<unsigned, std::vector<std::vector<FrameData>>> *_moveList = nullptr;
		std::unordered_map<unsigned, MoveData> _moveData;
		std::unordered_map<unsigned, MoveData> _leftMoveData;
		std::unordered_map<unsigned, MoveData> _rightMoveData;
		std::unique_ptr<FakeObject> _moveListObject;
		std::wstring _moveListName;
		unsigned int _moveListCursorMax = 0;
		unsigned int _moveListCursor = 0;
		unsigned int _moveListTop = 0;
		unsigned int _moveListTimer = 0;
		unsigned char _paused = 0;
		unsigned char _pauseCursor = 0;
		std::array<unsigned, NB_SPRITES> _moveSprites;
		std::vector<unsigned> _moveOrder;
		std::vector<unsigned> _moveDisplayed;
		std::string _endScene;
		bool _replaySaved = false;

		uint32_t _random;
		GameParams _params;

		virtual void _renderPause() const;
		virtual void _pauseUpdate();
		virtual bool _pauseConfirm();
		void _renderMoveList(Character *chr, const std::wstring &title) const;
		void _moveListUpdate(InputStruct input);
		void _calculateMoveListOrder();
		std::vector<Object *> _generateStageObjects(const StageEntry &stage);

		InGame(const GameParams &params);
		InGame(const InitParams &params, const std::string &endScene, bool saveReplay);
	};
}


#endif //SOFGV_INGAME_HPP

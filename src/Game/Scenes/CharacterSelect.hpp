//
// Created by PinkySmile on 29/09/2021.
//

#ifndef SOFGV_CHARACTERSELECT_HPP
#define SOFGV_CHARACTERSELECT_HPP


#include <memory>
#include "nlohmann/json.hpp"
#include "Resources/IScene.hpp"
#include "Inputs/IInput.hpp"
#include "Resources/Assets/FrameData.hpp"
#include "Objects/Characters/Character.hpp"
#include "InGame.hpp"
#include "LoadingScene.hpp"
#include "Resources/SceneArgument.hpp"
#include "SkeletonData.hpp"

namespace SpiralOfFate
{
	class CharacterSelect : public IScene {
	protected:
		mutable Sprite _stageSprite;
		mutable Sprite _randomSprite;
		std::shared_ptr<IInput> _leftInput;
		std::shared_ptr<IInput> _rightInput;
		mutable std::vector<StageEntry> _stages;
		mutable std::vector<CharacterEntry> _entries;
		std::string _inGameName;
		bool _quit = false;

		//Game State
		int _leftPos = 0;
		int _rightPos = 0;
		int _leftPalette = 0;
		int _rightPalette = 1;
		int _stage = 0;
		int _platform = 0;
		bool _selectingStage = false;

		virtual void _launchGame();
		void _selectCharacterRender() const;
		void _selectStageRender() const;
		void _selectCharacterUpdate();
		void _selectStageUpdate();
		void _displayPlatformPreview() const;

	public:
		struct Arguments : public SceneArguments {
			std::shared_ptr<IInput> leftInput;
			std::shared_ptr<IInput> rightInput;
			int leftPos = 0;
			int rightPos = 0;
			int leftPalette = 0;
			int rightPalette = 1;
			int stage = 0;
			int platformCfg = 0;
			std::string inGameName;

			~Arguments() override = default;
		};

		CharacterSelect(const Arguments &);
		CharacterSelect(
			std::shared_ptr<IInput> leftInput,
			std::shared_ptr<IInput> rightInput,
			const std::string &inGameName
		);
		CharacterSelect(
			std::shared_ptr<IInput> leftInput, std::shared_ptr<IInput> rightInput,
			int leftPos, int rightPos,
			int leftPalette, int rightPalette,
			int stage, int platformCfg,
			const std::string &inGameName
		);

		void render() const override;
		void update() override;
		void consumeEvent(const sf::Event &event) override;
		std::pair<std::shared_ptr<IInput>, std::shared_ptr<IInput>> getInputs() const;
		std::pair<std::vector<StageEntry>, std::vector<CharacterEntry>> getData() const;

		static std::pair<std::vector<StageEntry>, std::vector<CharacterEntry>> loadData();
		static CharacterSelect *create(SceneArguments *args);
	};
}


#endif //SOFGV_CHARACTERSELECT_HPP

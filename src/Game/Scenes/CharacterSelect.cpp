//
// Created by PinkySmile on 29/09/2021.
//

#include <utility>
#include <fstream>
#include "CharacterSelect.hpp"
#include "InGame.hpp"
#include "Resources/Game.hpp"
#include "Utils.hpp"

namespace SpiralOfFate
{
	CharacterSelect::CharacterSelect(
		std::shared_ptr<IInput> leftInput,
		std::shared_ptr<IInput> rightInput,
		const std::string &inGameName
	) :
		_randomSprite(game->textureMgr.load("assets/stages/random.png")),
		_leftInput(std::move(leftInput)),
		_rightInput(std::move(rightInput)),
		_inGameName(inGameName)
	{
		ViewPort view{{{0, 0}, {1680, 960}}};
		nlohmann::json json;
		auto chrList = game->getCharacters();

		game->screen->setView(view);
		game->logger.info("CharacterSelect scene created");
		this->_entries.reserve(chrList.size());
		for (auto &entry : chrList) {
			auto file = entry + "/chr.json";

			game->logger.debug("Loading character from " + file);

			auto data = game->fileMgr.readFull(file);

			json = nlohmann::json::parse(data);
#ifndef _DEBUG
			if (json.contains("hidden") && json["hidden"])
				continue;
#endif
			this->_entries.emplace_back(json, entry);
		}
		std::sort(this->_entries.begin(), this->_entries.end(), [](CharacterEntry &a, CharacterEntry &b){
			return a.pos < b.pos;
		});

		auto data = game->fileMgr.readFull("assets/stages/list.json");

		json = nlohmann::json::parse(data);
		this->_stages.reserve(json.size());
		for (auto &elem: json)
			this->_stages.emplace_back(elem);
	}

	CharacterSelect::CharacterSelect(
		std::shared_ptr<IInput> leftInput, std::shared_ptr<IInput> rightInput,
		int leftPos, int rightPos,
		int leftPalette, int rightPalette,
		int stage, int platformCfg,
		const std::string &inGameName
	) :
		CharacterSelect(std::move(leftInput), std::move(rightInput), inGameName)
	{
		this->_leftPos = leftPos;
		this->_rightPos = rightPos;
		this->_leftPalette = leftPalette;
		this->_rightPalette = rightPalette;
		this->_stage = stage;
		this->_platform = platformCfg;
	}

	CharacterSelect::CharacterSelect(const CharacterSelect::Arguments &arg) :
		CharacterSelect(
			std::move(arg.leftInput),
			std::move(arg.rightInput),
			arg.leftPos,
			arg.rightPos,
			arg.leftPalette,
			arg.rightPalette,
			arg.stage,
			arg.platformCfg,
			arg.inGameName
		)
	{
	}

	void CharacterSelect::render() const
	{
		this->_selectingStage ? this->_selectStageRender() : this->_selectCharacterRender();
	}

	void CharacterSelect::update()
	{
		this->_leftInput->update();
		this->_rightInput->update();

		if (this->_quit)
			return game->scene.switchScene("title_screen");
		if (this->_selectingStage)
			this->_selectStageUpdate();
		else
			this->_selectCharacterUpdate();
	}

	void CharacterSelect::consumeEvent(const sf::Event &event)
	{
		this->_leftInput->consumeEvent(event);
		this->_rightInput->consumeEvent(event);
		if (auto e = event.getIf<sf::Event::KeyPressed>()) {
			if (e->code == sf::Keyboard::Key::Escape)
				this->_quit = true;
		}
	}

	void CharacterSelect::_launchGame()
	{
		game->soundMgr.play(BASICSOUND_GAME_LAUNCH);
		if (this->_inGameName.empty())
			return;

		auto args = new InGame::Arguments{};

		args->params.seed = game->random();
		args->params.p1chr = this->_leftPos;
		args->params.p1pal = this->_leftPalette;
		args->params.p2chr = this->_rightPos;
		args->params.p2pal = this->_rightPalette;
		args->params.stage = this->_stage;
		args->params.platformConfig = this->_platform;
		args->stages = this->_stages;
		args->entries = this->_entries;
		args->leftInput = this->_leftInput;
		args->rightInput = this->_rightInput;
		args->saveReplay = true;
		args->endScene = game->scene.getCurrentScene().first;
		game->scene.switchScene(this->_inGameName, args);
	}

	void CharacterSelect::_selectCharacterRender() const
	{
		std::uniform_int_distribution<size_t> dist{0, this->_entries.size() - 1};
		size_t left  = this->_leftPos == -1  ? dist(game->random) : this->_leftPos;
		size_t right = this->_rightPos == -1 ? dist(game->random) : this->_rightPos;
		auto lInputs = this->_leftInput->getInputs();
		auto rInputs = this->_rightInput->getInputs();

		game->screen->fillColor(sf::Color::Black);
		game->screen->displayElement({0, 0, 1680, 960}, sf::Color{
			static_cast<uint8_t>(lInputs.d),
			static_cast<uint8_t>(rInputs.d),
			static_cast<uint8_t>((lInputs.d + rInputs.d) / 2)
		});
		game->screen->displayElement({0, 0, 560, 480}, sf::Color{0xA0, 0xA0, 0xA0, 0xFF});
		game->screen->displayElement({0, 480, 560, 480}, sf::Color::White);

		game->screen->displayElement({1120, 0, 560, 480}, sf::Color{0xA0, 0xA0, 0xA0, 0xFF});
		game->screen->displayElement({1120, 480, 560, 480}, sf::Color::White);

		game->screen->displayElement(this->_leftPos == -1 ? L"Random select" : this->_entries[this->_leftPos].name, {0, 480}, 560, Screen::ALIGN_CENTER);
		game->screen->displayElement(this->_rightPos == -1 ? L"Random select" : this->_entries[this->_rightPos].name, {1120, 480}, 560, Screen::ALIGN_CENTER);

		auto &leftSprites  = this->_entries[left].icon;
		auto &rightSprites = this->_entries[right].icon;
		std::uniform_int_distribution<size_t> ldist{0, this->_entries[left].palettes.size() - 1};
		std::uniform_int_distribution<size_t> rdist{0, this->_entries[right].palettes.size() - 1};
		int realLPal = this->_leftPos == -1  ? 0 : this->_leftPalette;
		int realRPal = this->_rightPos == -1 ? 0 : this->_rightPalette;
		auto &leftSprite  = leftSprites[realLPal % leftSprites.size()];
		auto &rightSprite = rightSprites[realRPal % rightSprites.size()];
		auto leftTexture  = leftSprite.getTextureSize();
		auto rightTexture = rightSprite.getTextureSize();

		leftSprite.setPosition({0, 0});
		leftSprite.setScale({560.f / leftTexture.x, 480.f / leftTexture.y});
		game->screen->displayElement(leftSprite);

		rightSprite.setPosition({1680, 0});
		rightSprite.setScale({-560.f / rightTexture.x, 480.f / rightTexture.y});
		game->screen->displayElement(rightSprite);

		game->screen->displayElement({540, 0, 600, 40}, sf::Color{0xB0, 0xB0, 0xB0, 0xFF});
		game->screen->displayElement("Character select", {540, 0}, 600, Screen::ALIGN_CENTER);
	}

	void CharacterSelect::_selectStageRender() const
	{
		Sprite &sprite = this->_stage == -1 ? this->_randomSprite : this->_stageSprite;

		sprite.setPosition({0, 0});
		sprite.setScale({
			1680.f / sprite.getTextureSize().x,
			960.f / sprite.getTextureSize().y
		});
		game->screen->displayElement(sprite);
		this->_displayPlatformPreview();
		game->screen->displayElement({540, 0, 600, 40}, sf::Color{0xB0, 0xB0, 0xB0, 0xFF});
		game->screen->displayElement("Stage select", {540, 0}, 600, Screen::ALIGN_CENTER);
		game->screen->textSize(20);
		game->screen->displayElement({540, 250, 600, 50}, sf::Color{0xB0, 0xB0, 0xB0, 0xA0});
		game->screen->displayElement("^", {540, 250}, 600, Screen::ALIGN_CENTER);
		game->screen->displayElement(this->_stage == -1 ? "Random select" : this->_stages[this->_stage].name, {540, 260}, 600, Screen::ALIGN_CENTER);
		game->screen->displayElement("v", {540, 280}, 600, Screen::ALIGN_CENTER);
		if (this->_stage != -1) {
			auto txt = "Credits to " + this->_stages[this->_stage].credits;
			auto size = game->screen->getTextSize(txt);

			game->screen->displayElement({static_cast<int>(1676 - size), 930, static_cast<int>(size + 4), 30}, sf::Color{0xB0, 0xB0, 0xB0, 0xFF});
			game->screen->displayElement(txt, {1678 - size, 932});
		}
		game->screen->textSize(30);
	}

	void CharacterSelect::_selectCharacterUpdate()
	{
		auto lInputs = this->_leftInput->getInputs();
		auto rInputs = this->_rightInput->getInputs();

		if (lInputs.horizontalAxis == -1) {
			game->soundMgr.play(BASICSOUND_MENU_MOVE);
			if (this->_leftPos == -1)
				this->_leftPos = static_cast<int>(this->_entries.size());
			this->_leftPos--;
		} else if (lInputs.horizontalAxis == 1) {
			game->soundMgr.play(BASICSOUND_MENU_MOVE);
			this->_leftPos++;
			if (this->_leftPos == static_cast<int>(this->_entries.size()))
				this->_leftPos = -1;
		}

		if (rInputs.horizontalAxis == -1) {
			game->soundMgr.play(BASICSOUND_MENU_MOVE);
			if (this->_rightPos == -1)
				this->_rightPos = static_cast<int>(this->_entries.size());
			this->_rightPos--;
		} else if (rInputs.horizontalAxis == 1) {
			game->soundMgr.play(BASICSOUND_MENU_MOVE);
			this->_rightPos++;
			if (this->_rightPos == static_cast<int>(this->_entries.size()))
				this->_rightPos = -1;
		}

		if (this->_leftPos >= 0) {
			if (lInputs.verticalAxis == -1) {
				game->soundMgr.play(BASICSOUND_MENU_MOVE);
				do {
					if (this->_leftPalette == 0)
						this->_leftPalette = static_cast<int>(this->_entries[this->_leftPos].palettes.size());
					this->_leftPalette--;
				} while (this->_entries[this->_leftPos].palettes.size() > 1 && this->_rightPalette == this->_leftPalette && this->_leftPos == this->_rightPos);
			} else if (lInputs.verticalAxis == 1) {
				game->soundMgr.play(BASICSOUND_MENU_MOVE);
				do {
					this->_leftPalette++;
					if (this->_leftPalette == static_cast<int>(this->_entries[this->_leftPos].palettes.size()))
						this->_leftPalette = 0;
				} while (this->_entries[this->_leftPos].palettes.size() > 1 && this->_rightPalette == this->_leftPalette && this->_leftPos == this->_rightPos);
			}
		}
		if (this->_rightPos >= 0) {
			if (rInputs.verticalAxis == -1) {
				game->soundMgr.play(BASICSOUND_MENU_MOVE);
				do {
					if (this->_rightPalette == 0)
						this->_rightPalette = static_cast<int>(this->_entries[this->_rightPos].palettes.size());
					this->_rightPalette--;
				} while (this->_entries[this->_rightPos].palettes.size() > 1 && this->_rightPalette == this->_leftPalette && this->_leftPos == this->_rightPos);
			} else if (rInputs.verticalAxis == 1) {
				game->soundMgr.play(BASICSOUND_MENU_MOVE);
				do {
					this->_rightPalette++;
					if (this->_rightPalette == static_cast<int>(this->_entries[this->_rightPos].palettes.size()))
						this->_rightPalette = 0;
				} while (this->_entries[this->_rightPos].palettes.size() > 1 && this->_rightPalette == this->_leftPalette && this->_leftPos == this->_rightPos);
			}
		}

		if (lInputs.n == 1 || rInputs.n == 1) {
			game->soundMgr.play(BASICSOUND_MENU_CONFIRM);
			this->_selectingStage = true;
			this->_stageSprite.setTexture(this->_stages[0].imageHandle, true);
		}
	}

	void CharacterSelect::_selectStageUpdate()
	{
		auto lInputs = this->_leftInput->getInputs();
		auto rInputs = this->_rightInput->getInputs();

		if (this->_stage != -1) {
			if (lInputs.horizontalAxis == -1 || rInputs.horizontalAxis == -1) {
				game->soundMgr.play(BASICSOUND_MENU_MOVE);
				if (this->_platform == -1)
					this->_platform = static_cast<int>(this->_stages[this->_stage].platforms.size());
				this->_platform--;
			} else if (lInputs.horizontalAxis == 1 || rInputs.horizontalAxis == 1) {
				game->soundMgr.play(BASICSOUND_MENU_MOVE);
				this->_platform++;
				if (this->_platform == static_cast<int>(this->_stages[this->_stage].platforms.size()))
					this->_platform = -1;
			}
		}
		if (lInputs.verticalAxis == -1 || rInputs.verticalAxis == -1) {
			game->soundMgr.play(BASICSOUND_MENU_MOVE);
			if (this->_stage == -1)
				this->_stage = static_cast<int>(this->_stages.size());
			this->_stage--;
			if (this->_stage != -1)
				this->_stageSprite.setTexture(this->_stages[this->_stage].imageHandle, true);
			this->_platform = 0;
		} else if (lInputs.verticalAxis == 1 || rInputs.verticalAxis == 1) {
			game->soundMgr.play(BASICSOUND_MENU_MOVE);
			this->_stage++;
			if (this->_stage == static_cast<int>(this->_stages.size()))
				this->_stage = -1;
			if (this->_stage != -1)
				this->_stageSprite.setTexture(this->_stages[this->_stage].imageHandle, true);
			this->_platform = 0;
		}

		if (lInputs.n == 1 || rInputs.n == 1)
			return this->_launchGame();
		if (lInputs.s == 1 || rInputs.s == 1) {
			this->_selectingStage = false;
			game->soundMgr.play(BASICSOUND_MENU_CANCEL);
		}
	}

	void CharacterSelect::_displayPlatformPreview() const
	{
		if (this->_stage == -1)
			return;

		std::uniform_int_distribution<size_t> dist{0, this->_stages[this->_stage].platforms.size() - 1};
		auto plat = this->_platform == -1 ? dist(game->random) : this->_platform;
		const Vector2f scale = {
			1680.f / 1100,
			960.f / 700
		};

		for (auto &platform : this->_stages[this->_stage].platforms[plat]) {
			auto scale2 = Vector2f{
				platform.data.scale.x,
				platform.data.scale.y
			};
			auto size2 = Vector2f{
				platform.data.textureBounds.size.x * platform.data.scale.x,
				platform.data.textureBounds.size.y * platform.data.scale.y
			};
			auto result = platform.data.offset + platform.pos;
			Sprite sprite{platform.data.textureHandle};

			result.y *= -1;
			result += Vector2f{
				size2.x / -2.f,
				-size2.y
			};
			result += Vector2f{
				platform.data.textureBounds.size.x * scale2.x / 2,
				platform.data.textureBounds.size.y * scale2.y / 2
			};
			sprite.setOrigin(platform.data.textureBounds.size / 2.f);
			sprite.setPosition({(result.x + 50 - STAGE_X_MIN) * scale.x, (result.y + 600) * scale.y});
			sprite.setScale({scale.x * scale2.x, scale.y * scale2.y});
			sprite.setTextureRect(platform.data.textureBounds);
			game->screen->displayElement(sprite);
		}
	}

	CharacterSelect *CharacterSelect::create(SceneArguments *args)
	{
		checked_cast(realArgs, CharacterSelect::Arguments, args);

		if (realArgs->reportProgressA)
			realArgs->reportProgressA("Loading assets...");
		return new CharacterSelect(*realArgs);
	}

	std::pair<std::vector<StageEntry>, std::vector<CharacterEntry>> CharacterSelect::loadData()
	{
		auto chrList = game->getCharacters();
		std::vector<StageEntry> stages;
		std::vector<CharacterEntry> entries;
		nlohmann::json json;

		entries.reserve(chrList.size());
		for (auto &entry : chrList) {
			auto file = entry + "/chr.json";

			game->logger.debug("Loading character from " + file);

			auto data = game->fileMgr.readFull(file);

			json = nlohmann::json::parse(data);
#ifndef _DEBUG
			if (json.contains("hidden") && json["hidden"])
				continue;
#endif
			entries.emplace_back(json, entry);
		}
		std::sort(entries.begin(), entries.end(), [](CharacterEntry &a, CharacterEntry &b){
			return a.pos < b.pos;
		});

		auto data = game->fileMgr.readFull("assets/stages/list.json");

		json = nlohmann::json::parse(data);
		stages.reserve(json.size());
		for (auto &elem: json)
			stages.emplace_back(elem);
		return { stages, entries };
	}

	std::pair<std::vector<StageEntry>, std::vector<CharacterEntry>> CharacterSelect::getData() const
	{
		return {this->_stages, this->_entries};
	}

	std::pair<std::shared_ptr<IInput>, std::shared_ptr<IInput>> CharacterSelect::getInputs() const
	{
		return {this->_leftInput, this->_rightInput};
	}

	CharacterEntry::CharacterEntry(const nlohmann::json &json, const std::string &folder) :
		entry(json)
	{
		assert_exp(json.contains("pos"));
		assert_exp(json.contains("name"));
		assert_exp(json.contains("ground_drag"));
		assert_exp(json.contains("air_drag"));
		assert_exp(json["air_drag"].contains("x"));
		assert_exp(json["air_drag"].contains("y"));
		assert_exp(json.contains("hp"));
		assert_exp(json.contains("guard_bar"));
		assert_exp(json.contains("guard_break_cooldown"));
		assert_exp(json.contains("neutral_overdrive_cooldown"));
		assert_exp(json.contains("spirit_overdrive_cooldown"));
		assert_exp(json.contains("matter_overdrive_cooldown"));
		assert_exp(json.contains("void_overdrive_cooldown"));
		assert_exp(json.contains("roman_cancel_cooldown"));
		assert_exp(json.contains("gravity"));
		assert_exp(json["gravity"].contains("x"));
		assert_exp(json["gravity"].contains("y"));
		assert_exp(json.contains("jump_count"));
		assert_exp(json.contains("air_dash_count"));
		assert_exp(json.contains("air_movements"));
		assert_exp(json.contains("mana_max"));
		assert_exp(json.contains("mana_start"));
		assert_exp(json.contains("mana_regen"));
		assert_exp(json.contains("palettes"));
		assert_exp(json.contains("air_drift"));
		assert_exp(json["air_drift"].contains("up"));
		assert_exp(json["air_drift"].contains("down"));
		assert_exp(json["air_drift"].contains("back"));
		assert_exp(json["air_drift"].contains("front"));
		assert_exp(json["air_drift"]["up"].contains("accel"));
		assert_exp(json["air_drift"]["up"].contains("max"));
		assert_exp(json["air_drift"]["down"].contains("accel"));
		assert_exp(json["air_drift"]["down"].contains("max"));
		assert_exp(json["air_drift"]["back"].contains("accel"));
		assert_exp(json["air_drift"]["back"].contains("max"));
		assert_exp(json["air_drift"]["front"].contains("accel"));
		assert_exp(json["air_drift"]["front"].contains("max"));

		for (auto &j : json["palettes"])
			this->palettes.push_back(folder + "/" + j.get<std::string>());

		this->pos = json["pos"];
		this->name = Utils::utf8ToUtf16(json["name"].get<std::string>());
		//FIXME: Temporary hack
		if (json.contains("class"))
			this->_class = json["class"];
		this->folder = folder;
		this->data = FrameData::loadFile(folder + "/charSelect.json", folder);
		if (this->palettes.empty())
			this->icon.emplace_back(game->textureMgr.load(folder + "/icon.png"));
		else {
			this->icon.reserve(this->palettes.size());
			for (auto &palette : this->palettes)
				this->icon.emplace_back(game->textureMgr.load(folder + "/icon.png", palette));
		}
	}


	CharacterEntry::CharacterEntry(const CharacterEntry &entry) :
		entry(entry.entry),
		pos(entry.pos),
		_class(entry._class),
		name(entry.name),
		folder(entry.folder),
		palettes(entry.palettes),
		icon(entry.icon),
		data(entry.data)
	{
	}

	PlatformSkeleton::PlatformSkeleton(const nlohmann::json &json) :
		entry(json)
	{
		this->framedata = json["framedata"];
		this->_class = json["class"];
		this->width = json["width"];
		this->hp = json["hp"];
		this->cd = json["cd"];
		this->pos.x = json["pos"]["x"];
		this->pos.y = json["pos"]["y"];
		//TODO
		this->data = FrameData::loadFile(this->framedata, this->framedata.substr(0, this->framedata.find_last_of('/')))[0].front().front();
	}

	StageEntry::StageEntry(const nlohmann::json &json) :
		entry(json)
	{
		this->name = json["name"];
		this->credits = json["credits"];
		this->imagePath = json["image"];
		this->imageHandle = game->textureMgr.load(this->imagePath);
		if (json.contains("objects"))
			this->objectPath = json["objects"];
		for (auto &platformArray : json["platforms"]) {
			this->platforms.emplace_back();
			for (auto &platform : platformArray)
				this->platforms.back().emplace_back(platform);
		}
	}

	StageEntry::StageEntry(const StageEntry &other)
	{
		this->entry = other.entry;
		this->name = other.name;
		this->credits = other.credits;
		this->objectPath = other.objectPath;
		this->imagePath = other.imagePath;
		this->imageHandle = other.imageHandle;
		this->platforms = other.platforms;
		game->textureMgr.addRef(this->imageHandle);
	}

	StageEntry::~StageEntry()
	{
		game->textureMgr.remove(this->imageHandle);
	}
}

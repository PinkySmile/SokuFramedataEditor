//
// Created by PinkySmile on 29/12/2025.
//

#include <any>
#include <iomanip>
#define private public
#define protected public
#include <iostream>
#include "Objects/Characters/Stickman/Stickman.hpp"
#include "Objects/StageObjects/Cloud.hpp"
#include "Objects/StageObjects/StageObject.hpp"
#include "Resources/Game.hpp"

using namespace SpiralOfFate;

Character *createCharacter(const std::string &folder, int _class)
{
	assert_eq(_class, 1);
	return new Stickman{
		0,
		folder,
		{},
		std::make_shared<RollbackInput>()
	};
}

std::string formatTime(uint64_t microseconds)
{
	if (microseconds < 1000)
		return std::to_string(microseconds) + "µs";
	if (microseconds < 1000000)
		return std::to_string(microseconds / 1000) + "." + std::to_string(microseconds % 1000) + "ms";
	return std::to_string(microseconds / 1000000) + "." + std::to_string(microseconds % 1000000) + "s";
}

int main()
{
	new Game("assets/fonts/Retro Gaming.ttf", "settings.json", "bench.log");

	std::ifstream stream;
	nlohmann::json stagesJson;
	nlohmann::json lJson;
	nlohmann::json rJson;

	stream.open("assets/stages/list.json");
	assert_msg(!stream.fail(), "assets/stages/list.json: " + strerror(errno));
	stream >> stagesJson;
	stream.close();
	stream.open("assets/characters/stickman/chr.json");
	assert_msg(!stream.fail(), std::string("assets/characters/stickman/chr.json: ") + strerror(errno));
	stream >> lJson;
	stream.close();
	stream.open("assets/characters/stickman/chr.json");
	assert_msg(!stream.fail(), std::string("assets/characters/stickman/chr.json: ") + strerror(errno));
	stream >> rJson;
	stream.close();
	assert_exp(!stagesJson.empty());
	assert_exp(!stagesJson[0]["platforms"].empty());

	auto lchr = BattleManager::CharacterParams{ true,  createCharacter("assets/characters/stickman", lJson["class"]), 0, lJson };
	auto rchr = BattleManager::CharacterParams{ false, createCharacter("assets/characters/stickman", rJson["class"]), 0, rJson };

	game->battleMgr = std::make_unique<BattleManager>(
		BattleManager::StageParams{
			stagesJson[0]["image"],
			[&stagesJson] () -> std::vector<Object *>{
				if (!stagesJson[0].contains("objects")) {
					std::cout << "Stage has no objects" << std::endl;
					return {};
				}

				std::ifstream stream2{stagesJson[0]["objects"].get<std::string>()};
				nlohmann::json json;
				std::vector<Object *> objects;

				if (stream2.fail()) {
					game->logger.error("Failed to open stage object file: " + stagesJson[0]["objects"].get<std::string>() + ": " + strerror(errno));
					return objects;
				}

				try {
					stream2 >> json;
					for (auto &obj : json) {
						switch (obj["class"].get<int>()) {
						case 1:
							objects.push_back(new Cloud(obj));
							break;
						default:
							objects.push_back(new StageObject(obj));
						}
					}
					return objects;
				} catch (std::exception &e) {
					game->logger.error("Error while loading objects: " + std::string(e.what()));
					for (auto object : objects)
						delete object;
				}
				return {};
			},
			[&stagesJson]{
				std::vector<Platform *> objects;

				for (auto &platform : stagesJson[0]["platforms"][0])
					objects.push_back(new Platform(platform["framedata"], platform["width"], platform["hp"], platform["cd"], Vector2f{
						platform["pos"]["x"],
						platform["pos"]["y"]
					}));
				return objects;
			}
		},
		lchr, rchr
	);
	lchr.character->_hasVoidInstall = true;
	lchr.character->_spiritEffectTimer = 300;
	lchr.character->_spawnSystemParticles(SYS_PARTICLE_GENERATOR_VOID_TYPE_SWITCH);
	lchr.character->_spawnSystemParticles(SYS_PARTICLE_GENERATOR_SPIRIT_TYPE_DEBUFF);
	rchr.character->_hasMatterInstall = true;
	rchr.character->_voidEffectTimer = 300;
	rchr.character->_spawnSystemParticles(SYS_PARTICLE_GENERATOR_MATTER_TYPE_SWITCH);
	rchr.character->_spawnSystemParticles(SYS_PARTICLE_GENERATOR_VOID_TYPE_DEBUFF);
	for (size_t i = 0; i < 120; i++)
		game->battleMgr->update();

	std::ofstream s{"s.frame", std::ofstream::binary};
	auto mem = new unsigned char[64 * 1024];

	assert_exp(game->battleMgr->getBufferSize() <= 64 * 1024);
	game->battleMgr->copyToBuffer(mem);
	s.write(reinterpret_cast<const std::ostream::char_type *>(mem), game->battleMgr->getBufferSize());
	game->battleMgr->printContent(mem, 64 * 1024);

	std::cout << "Testing copyToBuffer" << std::endl;
	sf::Clock clock;
	for (size_t i = 0; i < 100000; i++) {
		if (i % 1000 == 0)
			printf("%zu/100000\n\033[A", i);
		game->battleMgr->copyToBuffer(mem);
	}

	std::cout << "Testing restoreFromBuffer" << std::endl;
	sf::Time time1 = clock.reset();
	clock.start();
	for (size_t i = 0; i < 100000; i++) {
		if (i % 1000 == 0)
			printf("%zu/100000\n\033[A", i);
		game->battleMgr->restoreFromBuffer(mem);
	}

	sf::Time time2 = clock.reset();
	std::cout << "copyToBuffer: Total " << time1.asSeconds() << "s, Avg " << formatTime(time1.asMicroseconds() / 100000) << std::endl;
	std::cout << "restoreFromBuffer: Total " << time2.asSeconds() << "s, Avg " << formatTime(time2.asMicroseconds() / 100000) << std::endl;
	delete[] mem;
	return EXIT_SUCCESS;
}
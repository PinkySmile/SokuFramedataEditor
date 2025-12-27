//
// Created by PinkySmile on 11/07/23.
//

#include <fcntl.h>
#include <unistd.h>
#include <Resources/Game.hpp>
#include <fstream>
#include <iostream>
#include <memory>
#include "Objects/StageObjects/Cloud.hpp"
#include "Objects/StageObjects/StageObject.hpp"
#include "Objects/Characters/VictoriaStar/VictoriaStar.hpp"
#include "Objects/Characters/Stickman/Stickman.hpp"

using namespace SpiralOfFate;

Character *createCharacter(const std::string &folder, int _class)
{
	switch (_class) {
	case 2:
		return new VictoriaStar{
			0,
			folder,
			{},
			nullptr,
			""
		};
	case 1:
		return new Stickman{
			0,
			folder,
			{},
			nullptr
		};
	default:
		return new Character{
			0,
			folder,
			{},
			nullptr
		};
	}
}

#define BOLD_OFF 2
#define D1_OFF 4
#define D2_OFF 5
#define PAIR_SIZE 7
#define ADDR_SIZE 10

char digits[] = "0123456789ABCDEF";

void displayBeforeDiff(size_t index, size_t lastIndex, const char *buffer1, const char *buffer2, char *lineBuffer)
{
	size_t currentLine = index / 10;
	size_t lastLine = lastIndex / 10;
	char *line1Buffer = lineBuffer + ADDR_SIZE + 2;
	char *line2Buffer = lineBuffer + ADDR_SIZE + 78;

	if (lastLine != currentLine) {
		char *ptr = lineBuffer + ADDR_SIZE - 2;
		size_t i = currentLine;

		*ptr = '0';
		while (i != 0) {
			ptr--;
			*ptr = '0' + i % 10;
			i /= 10;
		}
	}
	for (size_t i = currentLine; i < index; i++) {
		if (i <= lastIndex)
			continue;

		size_t slot = i % 10;
		unsigned char c1 = buffer1[i];
		unsigned char c2 = buffer2[i];

		line1Buffer[slot * PAIR_SIZE + BOLD_OFF] = '0';
		line1Buffer[slot * PAIR_SIZE + D1_OFF] = digits[c1 >> 4];
		line1Buffer[slot * PAIR_SIZE + D2_OFF] = digits[c1 & 0xF];
		line2Buffer[slot * PAIR_SIZE + BOLD_OFF] = '0';
		line2Buffer[slot * PAIR_SIZE + D1_OFF] = digits[c2 >> 4];
		line2Buffer[slot * PAIR_SIZE + D2_OFF] = digits[c2 & 0xF];
	}
}

void displayAfterDiff(size_t index, size_t lastIndex, const char *buffer1, const char *buffer2, char *lineBuffer, bool last)
{
	size_t currentLine = index / 10;
	size_t lastLine = lastIndex / 10;
	char *line1Buffer = lineBuffer + ADDR_SIZE + 2;
	char *line2Buffer = lineBuffer + ADDR_SIZE + 78;

	for (size_t i = lastIndex + 1; i < index && i < lastLine + 10; i++) {
		size_t slot = i % 10;
		unsigned char c1 = buffer1[i];
		unsigned char c2 = buffer2[i];

		line1Buffer[slot* PAIR_SIZE + BOLD_OFF] = '0';
		line1Buffer[slot* PAIR_SIZE + D1_OFF] = digits[c1 >> 4];
		line1Buffer[slot* PAIR_SIZE + D2_OFF] = digits[c1 & 0xF];
		line2Buffer[slot* PAIR_SIZE + BOLD_OFF] = '0';
		line2Buffer[slot* PAIR_SIZE + D1_OFF] = digits[c2 >> 4];
		line2Buffer[slot* PAIR_SIZE + D2_OFF] = digits[c2 & 0xF];
	}
	if (currentLine > lastLine || last)
		game->logger.info(lineBuffer);
	if (currentLine > lastLine && currentLine - lastLine > 10)
		game->logger.info("| ....... | .. .. .. .. .. .. .. .. .. .. | .. .. .. .. .. .. .. .. .. .. |");
}

void displayDiff(size_t index, const char *buffer1, const char *buffer2, char *lineBuffer)
{
	size_t slot = index % 10;
	char *line1Buffer = lineBuffer + ADDR_SIZE + 2;
	char *line2Buffer = lineBuffer + ADDR_SIZE + 78;
	unsigned char c1 = buffer1[index];
	unsigned char c2 = buffer2[index];

	line1Buffer[slot* PAIR_SIZE + BOLD_OFF] = '1';
	line1Buffer[slot* PAIR_SIZE + D1_OFF] = digits[c1 >> 4];
	line1Buffer[slot* PAIR_SIZE + D2_OFF] = digits[c1 & 0xF];
	line2Buffer[slot* PAIR_SIZE + BOLD_OFF] = '1';
	line2Buffer[slot* PAIR_SIZE + D1_OFF] = digits[c2 >> 4];
	line2Buffer[slot* PAIR_SIZE + D2_OFF] = digits[c2 & 0xF];
}

void displayDiffs(const std::vector<size_t> &diffs, const char *buffer1, const char *buffer2, size_t size)
{
	size_t lastPos = -1;
	char lineBuffer[] = "|         | \033[0m00 \033[0m00 \033[0m00 \033[0m00 \033[0m00 \033[0m00 \033[0m00 \033[0m00 \033[0m00 \033[0m00\033[0m | \033[0m00 \033[0m00 \033[0m00 \033[0m00 \033[0m00 \033[0m00 \033[0m00 \033[0m00 \033[0m00 \033[0m00\033[0m |";

	game->logger.info("*---------*-------------------------------*-------------------------------*");
	game->logger.info("| Address |            File 1             |             File 2            |");
	game->logger.info("*---------*-------------------------------*-------------------------------*");
	for (size_t diff : diffs) {
		displayAfterDiff(diff, lastPos, buffer1, buffer2, lineBuffer, false);
		displayBeforeDiff(diff, lastPos, buffer1, buffer2, lineBuffer);
		displayDiff(diff, buffer1, buffer2, lineBuffer);
		lastPos = diff;
	}
	displayAfterDiff(lastPos, size, buffer1, buffer2, lineBuffer, true);
	game->logger.info("*---------*-------------------------------*-------------------------------*");
}

int main(int argc, char **argv)
{
#ifndef _DEBUG
	try {
#endif
		if (argc < 6) {
			printf("Usage: %s <stage_id> <platform_id> <chr1_path> <chr2_path> <file1.frame> [<file2.frame>]\n", argv[0]);
			return EXIT_FAILURE;
		}

		std::ifstream stream;
		nlohmann::json stagesJson;
		nlohmann::json lJson;
		nlohmann::json rJson;
		unsigned stage = atoi(argv[1]);
		unsigned platforms = atoi(argv[2]);
		char *chr1_path = argv[3];
		char *chr2_path = argv[4];
		char *frame_file = argv[5];
		char *frame_file2 = argv[6];

		new Game("assets/fonts/Retro Gaming.ttf", "settings.json", "comparer.log");
	//	game->screen->setView(view);
	//	game->logger.info("CharacterSelect scene created");
	//	this->_entries.reserve(chrList.size());
	//	for (auto &entry : chrList) {
	//		auto file = entry / "chr.json";
	//		std::ifstream s{file};

	//		game->logger.debug("Loading character from " + file.string());
	//		my_assert2(!s.fail(), file.string() + ": " + strerror(errno));
	//		s >> json;
	//#ifndef _DEBUG
	//		if (json.contains("hidden") && json["hidden"])
	//				continue;
	//#endif
	//		this->_entries.emplace_back(json, entry.string());
	//	}

		stream.open("assets/stages/list.json");
		assert_msg(!stream.fail(), "assets/stages/list.json: " + strerror(errno));
		stream >> stagesJson;
		stream.close();
		stream.open(chr1_path + std::string("/chr.json"));
		assert_msg(!stream.fail(), chr1_path + std::string("/chr.json: ") + strerror(errno));
		stream >> lJson;
		stream.close();
		stream.open(chr2_path + std::string("/chr.json"));
		assert_msg(!stream.fail(), chr2_path + std::string("/chr.json: ") + strerror(errno));
		stream >> rJson;
		stream.close();
		assert_exp(stagesJson.size() > stage);
		assert_exp(stagesJson[stage]["platforms"].size() > platforms);

		game->battleMgr = std::make_unique<BattleManager>(
			BattleManager::StageParams{
				stagesJson[stage]["image"],
				[&stagesJson, stage] () -> std::vector<Object *>{
					if (!stagesJson[stage].contains("objects")) {
						std::cout << "Stage has no objects" << std::endl;
						return {};
					}

					std::ifstream stream2{stagesJson[stage]["objects"].get<std::string>()};
					nlohmann::json json;
					std::vector<Object *> objects;

					if (stream2.fail()) {
						game->logger.error("Failed to open stage object file: " + stagesJson[stage]["objects"].get<std::string>() + ": " + strerror(errno));
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
				[&stagesJson, stage, platforms]{
					std::vector<Platform *> objects;

					for (auto &platform : stagesJson[stage]["platforms"][platforms])
						objects.push_back(new Platform(platform["framedata"], platform["width"], platform["hp"], platform["cd"], Vector2f{
							platform["pos"]["x"],
							platform["pos"]["y"]
						}));
					return objects;
				}
			},
			BattleManager::CharacterParams{ true,  createCharacter(chr1_path, lJson["class"]), 0, lJson },
			BattleManager::CharacterParams{ false, createCharacter(chr2_path, rJson["class"]), 0, rJson }
		);

		char buffer1[32 * 1024];
		char buffer2[32 * 1024];
		int fd = open(frame_file, O_RDONLY);
		assert_msg(fd >= 0, frame_file + std::string(": ") + strerror(errno));
		ssize_t size = read(fd, buffer1, sizeof(buffer1));
		assert_msg(size >= 0, std::string("read(") + frame_file + "): " + strerror(errno));
		close(fd);

		if (frame_file2) {
			fd = open(frame_file2, O_RDONLY);
			assert_msg(fd >= 0, frame_file2 + std::string(": ") + strerror(errno));
			ssize_t size2 = read(fd, buffer2, sizeof(buffer2));
			assert_msg(size2 >= 0, std::string("read(") + frame_file2 + "): " + strerror(errno));
			close(fd);

			if (size == size2) {
				std::vector<size_t> diffs;

				for (size_t i = 0; i < static_cast<size_t>(size); i++)
					if (buffer1[i] != buffer2[i])
						diffs.push_back(i);
				if (diffs.empty()) {
					game->logger.info("Files are identical");
					return EXIT_SUCCESS;
				}
				displayDiffs(diffs, buffer1, buffer2, size);
			} else
				game->logger.info("Size mismatch: " + std::to_string(size) + " != " + std::to_string(size2));
			game->battleMgr->logDifference(buffer1, buffer2);
		} else
			game->battleMgr->printContent(buffer1, size);
		return EXIT_SUCCESS;
#ifndef _DEBUG
	} catch (std::exception &e) {
		printf("%s\n", e.what());
	}
#endif
}

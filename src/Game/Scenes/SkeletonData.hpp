//
// Created by PinkySmile on 19/07/25.
//

#ifndef SOFGV_SKELETONDATA_HPP
#define SOFGV_SKELETONDATA_HPP


#include <string>
#include <nlohmann/json.hpp>
#include "Sprite.hpp"
#include "Resources/Assets/FrameData.hpp"

namespace SpiralOfFate
{
	struct CharacterEntry {
		nlohmann::json entry;
		int pos;
		unsigned _class;
		std::wstring name;
		std::string folder;
		std::vector<std::string> palettes;
		std::vector<Sprite> icon;
		std::unordered_map<unsigned, std::vector<std::vector<FrameData>>> data;

		CharacterEntry(const nlohmann::json &json, const std::string &folder);
		CharacterEntry(const CharacterEntry &entry);
	};

	struct PlatformSkeleton {
		nlohmann::json entry;
		std::string framedata;
		FrameData data;
		unsigned _class;
		float width;
		unsigned hp;
		unsigned cd;
		Vector2f pos;

		PlatformSkeleton(const nlohmann::json &json);
	};

	struct StageEntry {
		nlohmann::json entry;
		std::string name;
		std::string credits;
		std::string objectPath;
		std::string imagePath;
		unsigned imageHandle;
		std::vector<std::vector<PlatformSkeleton>> platforms;

		StageEntry(const StageEntry &other);
		StageEntry(const nlohmann::json &json);
		~StageEntry();
	};
}


#endif //SOFGV_SKELETONDATA_HPP

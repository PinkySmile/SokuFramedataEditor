//
// Created by PinkySmile on 08/08/23
//

#ifndef SOFGV_FILEMANAGER_HPP
#define SOFGV_FILEMANAGER_HPP


#include <vector>
#include <string>

#ifdef __ANDROID__
#include <android/asset_manager.h>
#endif

namespace SpiralOfFate
{
	class FileManager {
	public:
		struct DirectoryEntry {
			bool dir;
			std::filesystem::path path;
		};

		FileManager() = default;
		std::vector<DirectoryEntry> listDirectory(const std::filesystem::path &path);
		bool exists(const std::filesystem::path &path);
		std::string readFull(const std::filesystem::path &path);
	};
}


#endif //SOFGV_FILEMANAGER_HPP

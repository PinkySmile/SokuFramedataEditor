//
// Created by PinkySmile on 18/09/2021.
//

#include <sys/stat.h>
#include "SoundManager.hpp"
#include "Resources/Game.hpp"
#include "Logger.hpp"

#define MAX_SOUND 64

namespace SpiralOfFate
{
	unsigned SoundManager::load(const std::filesystem::path &file)
	{
		if (this->_allocatedSounds[file].second != 0) {
			this->_allocatedSounds[file].second++;
			game->logger.verbose("Returning already loaded file " + file.string());
			return this->_allocatedSounds[file].first;
		}

		std::error_code err;

#ifndef __ANDROID__
		if (std::filesystem::is_directory(file, err)) {
			game->logger.error(file.string() + ": Is a directory");
			return 0;
		}
		if (err) {
			game->logger.error(file.string() + ": Is a directory");
			return 0;
		}
#endif

		unsigned index;

		if (this->_freedIndexes.empty()) {
			this->_lastIndex += this->_lastIndex == 0;
			index = this->_lastIndex;
			this->_lastIndex++;
		} else {
			index = this->_freedIndexes.back();
			this->_freedIndexes.pop_back();
		}

		game->logger.debug("Loading sound file " + file.string());
		if (!this->_sounds[index].buffer.loadFromFile(file))
			game->logger.warn("Failed to sound load " + file.string());
		else
			game->logger.verbose("Loaded sound " + file.string() + " successfully");

		this->_sounds[index].sound.setVolume(this->_volume);
		this->_allocatedSounds[file].first = index;
		this->_allocatedSounds[file].second = 1;
		return index;
	}

	void SoundManager::remove(unsigned int id)
	{
		if (!id)
			return;

		for (auto &[loadedPath, attr] : this->_allocatedSounds)
			if (attr.first == id && attr.second) {
				attr.second--;
				if (attr.second) {
					game->logger.verbose("Remove ref to " + loadedPath.string());
					return;
				}
				game->logger.debug("Destroying sound " + loadedPath.string());
				break;
			}

		auto it = this->_sounds.find(id);

		if (it == this->_sounds.end())
			return game->logger.warn("Trying to remove invalid index " + std::to_string(id));
		this->_sounds.erase(it);
		this->_freedIndexes.push_back(id);
	}

	void SoundManager::play(unsigned id) const
	{
		if (id == 0)
			return;
		game->logger.debug("Playing sound " + std::to_string(id));
		this->_sounds.at(id).sound.play();
	}

	void SoundManager::addRef(unsigned int id)
	{
		if (id == 0)
			return;
		for (auto &[loadedPath, attr] : this->_allocatedSounds)
			if (attr.first == id && attr.second) {
				if (attr.second < 1)
					return game->logger.warn("Cannot add ref to " + loadedPath.string() + " (" + std::to_string(id) + ") because it was unloaded");
				game->logger.verbose("Adding ref to " + loadedPath.string());
				attr.second++;
				return;
			}
		game->logger.warn("Cannot add ref to unknown sound " + std::to_string(id));
	}

	void SoundManager::setVolume(float volume)
	{
		this->_volume = volume;
		for (auto &pair : this->_sounds)
			pair.second.sound.setVolume(volume);
	}
}

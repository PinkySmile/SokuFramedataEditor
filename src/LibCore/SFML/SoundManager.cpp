//
// Created by PinkySmile on 18/09/2021.
//

#include <sys/stat.h>
#include "SoundManager.hpp"
#include "Resources/Game.hpp"
#include "Logger.hpp"
#include "Resources/Assert.hpp"

#define MAX_SOUND 64

namespace SpiralOfFate
{
	unsigned SoundManager::load(const std::string &file)
	{
		if (this->_allocatedSounds[file].second != 0) {
			this->_allocatedSounds[file].second++;
			game->logger.verbose("Returning already loaded file " + file);
			return this->_allocatedSounds[file].first;
		}

		auto entry = game->package.find(file);
		if (entry == game->package.end())
			return 0;

		auto type = entry.fileType();

		if (type.type != ShadyCore::FileType::TYPE_SFX)
			return 0;

		ShadyCore::Sfx sfx;
		auto &stream = entry.open();
		ShadyCore::getResourceReader(type)(&sfx, stream);
		entry.close(stream);

		unsigned index;
		if (this->_freedIndexes.empty()) {
			this->_lastIndex += this->_lastIndex == 0;
			index = this->_lastIndex;
			this->_lastIndex++;
		} else {
			index = this->_freedIndexes.back();
			this->_freedIndexes.pop_back();
		}

		bool result;
		std::vector channels{sfx.channels, sf::SoundChannel::Mono};

		game->logger.debug("Loading sound file " + file);
		assert_exp(sfx.bitsPerSample == 16);
		/*if (sfx.bitsPerSample == 8) {
			std::vector<int16_t> samples;

			samples.resize(sfx.size);
			for (size_t i = 0; i < sfx.size; i++) {
				uint16_t sample =
					(sfx.data[i] & 0x80U) << 8U |
					(sfx.data[i] & 0x40U) << 7U |
					(sfx.data[i] & 0x20U) << 6U |
					(sfx.data[i] & 0x10U) << 5U |
					(sfx.data[i] & 0x08U) << 4U |
					(sfx.data[i] & 0x04U) << 3U |
					(sfx.data[i] & 0x02U) << 2U |
					(sfx.data[i] & 0x01U) << 1U;

				samples[i] = static_cast<int>(sample) + INT16_MIN;
			}
			result = this->_sounds[index].buffer.loadFromSamples(samples.data(), sfx.size, sfx.channels, sfx.sampleRate, channels);
		}*/
		result = this->_sounds[index].buffer.loadFromSamples(
			reinterpret_cast<const std::int16_t *>(sfx.data),
			sfx.size / 2,
			sfx.channels,
			sfx.sampleRate, channels
		);
		if (!result)
			game->logger.warn("Failed to sound load " + file);
		else
			game->logger.verbose("Loaded sound " + file + " successfully");

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
					game->logger.verbose("Remove ref to " + loadedPath);
					return;
				}
				game->logger.debug("Destroying sound " + loadedPath);
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
					return game->logger.warn("Cannot add ref to " + loadedPath + " (" + std::to_string(id) + ") because it was unloaded");
				game->logger.verbose("Adding ref to " + loadedPath);
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

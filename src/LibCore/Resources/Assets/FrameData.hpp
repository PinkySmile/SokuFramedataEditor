//
// Created by PinkySmile on 18/09/2021.
//

#ifndef SOFGV_FRAMEDATA_HPP
#define SOFGV_FRAMEDATA_HPP


#include <filesystem>
#include <unordered_map>
#include <optional>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <resource.hpp>
#include "Color.hpp"

uint32_t getOrCreateImage(ShadyCore::Schema &schema, const std::string_view& name);

namespace SpiralOfFate
{
	class FrameData : public ShadyCore::Schema::Sequence::MoveFrame {
	private:
		bool _slave = false;

	public:
		std::string __folder;
		bool __requireReload = false;
		const std::array<Color, 256> *__paletteData = nullptr;

		std::string spritePath;
		unsigned textureHandle = 0;

		FrameData();
		~FrameData();
		FrameData(const FrameData &other);
		FrameData(const nlohmann::json &json,                          const std::string &folder, const std::array<Color, 256> *palette);
		FrameData(const ShadyCore::Schema::Sequence::Frame &frame,     const std::string &folder, const std::array<Color, 256> *palette, const ShadyCore::Schema &schema);
		FrameData(const ShadyCore::Schema::Sequence::MoveFrame &frame, const std::string &folder, const std::array<Color, 256> *palette, const ShadyCore::Schema &schema);
		FrameData &operator=(const FrameData &other);
		void checkReloadTexture();
		void reloadTexture();
		void setSlave(bool slave = true);
		nlohmann::json toJson() const;
		ShadyCore::Schema::Sequence::BlendOptions getBlendOptions() const;

		struct Sequence {
			std::vector<FrameData> data;
			uint16_t moveLock;
			uint16_t actionLock;
			bool loop;

			[[nodiscard]] size_t size() const
			{
				return this->data.size();
			}
			[[nodiscard]] const FrameData &at(size_t i) const
			{
				return this->data.at(i);
			}
			[[nodiscard]] FrameData &at(size_t i)
			{
				return this->data.at(i);
			}
			[[nodiscard]] const FrameData &operator[](size_t i) const
			{
				return this->data[i];
			}
			[[nodiscard]] FrameData &operator[](size_t i)
			{
				return this->data[i];
			}
			[[nodiscard]] auto begin() const
			{
				return this->data.begin();
			}
			[[nodiscard]] auto end() const
			{
				return this->data.end();
			}
			[[nodiscard]] auto begin()
			{
				return this->data.begin();
			}
			[[nodiscard]] auto end()
			{
				return this->data.end();
			}
		};
		struct Action {
			std::vector<Sequence> _sequences;
			Action *cloned = nullptr;
			unsigned clonedId;

			[[nodiscard]] size_t size() const
			{
				return this->_sequences.size();
			}
			[[nodiscard]] const Sequence &at(size_t i) const
			{
				return this->_sequences.at(i);
			}
			[[nodiscard]] Sequence &at(size_t i)
			{
				return this->_sequences.at(i);
			}
			[[nodiscard]] const Sequence &operator[](size_t i) const
			{
				return this->_sequences[i];
			}
			[[nodiscard]] Sequence &operator[](size_t i)
			{
				return this->_sequences[i];
			}
			[[nodiscard]] const std::vector<Sequence> &sequences() const
			{
				if (this->cloned)
					return this->cloned->sequences();
				return this->_sequences;
			}
			[[nodiscard]] std::vector<Sequence> &sequences()
			{
				if (this->cloned)
					return this->cloned->sequences();
				return this->_sequences;
			}

			[[nodiscard]] auto begin() const
			{
				return this->sequences().begin();
			}
			[[nodiscard]] auto end() const
			{
				return this->sequences().end();
			}
			[[nodiscard]] auto begin()
			{
				return this->sequences().begin();
			}
			[[nodiscard]] auto end()
			{
				return this->sequences().end();
			}
		};
		struct LoadedSchema {
			std::map<unsigned, Action> framedata;
			bool isCharacterData;
		};

		static LoadedSchema loadFile(const std::string &path, const std::string &folder, const std::array<Color, 256> *palette);
		static LoadedSchema loadFile(const std::filesystem::path &path, const std::string &folder, const std::array<Color, 256> *palette);
	};
}


#endif //SOFGV_FRAMEDATA_HPP

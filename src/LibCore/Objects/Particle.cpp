//
// Created by PinkySmile on 16/11/23.
//

#include "Particle.hpp"

#include <utility>

#include "CheckUtils.hpp"
#include "Objects/Characters/Character.hpp"
#include "Resources/Game.hpp"

namespace SpiralOfFate
{
	Particle::Particle(Source source, const Particle::InitData &initData, const Character &owner, unsigned int sprite, Vector2f position, bool compute) :
		_source(std::move(source)),
		_owner(owner),
		_data(initData),
		_sprite(sprite),
		_position(position)
	{
		if (compute) {
			this->_aliveTimer = random_distrib(game->battleRandom, initData.lifeSpan.first, initData.lifeSpan.second);
			this->_maxFadeTime = random_distrib(game->battleRandom, initData.fadeTime.first, initData.fadeTime.second);
			this->_updateTimer = random_distrib(game->battleRandom, initData.updateInterval.first, initData.updateInterval.second);
			this->_scale = random_distrib(game->battleRandom, initData.scale.first, initData.scale.second);
			this->_fadeTime = this->_maxFadeTime;
		}
	}

	void Particle::render() const
	{
		{
			// TODO: Check if it works
			sf::VertexArray arr{sf::PrimitiveType::TriangleFan, 4};
			auto size = this->_data.textureBounds.size * this->_scale;
			Rectangle box = {
				{this->_position.x - size.x / 2.f, -this->_position.y - size.y / 2.f},
				{this->_position.x + size.x / 2.f, -this->_position.y - size.y / 2.f},
				{this->_position.x + size.x / 2.f, -this->_position.y + size.y / 2.f},
				{this->_position.x - size.x / 2.f, -this->_position.y + size.y / 2.f},
			};

			for (int i = 0; i < 4; i++) {
				auto col = this->_data.colors[this->_currentColors][i];

				arr[i].color = {col.r, col.g, col.b, col.a};
				arr[i].color.a *= this->_fadeTime * 1.f / this->_maxFadeTime;
				arr[i].position = (&box.pt1)[i];
			}
			arr[this->_mirror.y * 2 + this->_mirror.x].texCoords = {
				static_cast<float>(this->_data.textureBounds.pos.x),
				static_cast<float>(this->_data.textureBounds.pos.y)
			};
			arr[this->_mirror.y * 2 + !this->_mirror.x].texCoords = {
				static_cast<float>(this->_data.textureBounds.pos.x + this->_data.textureBounds.size.x),
				static_cast<float>(this->_data.textureBounds.pos.y)
			};
			arr[!this->_mirror.y * 2 + this->_mirror.x].texCoords = {
				static_cast<float>(this->_data.textureBounds.pos.x + this->_data.textureBounds.size.x),
				static_cast<float>(this->_data.textureBounds.pos.y + this->_data.textureBounds.size.y)
			};
			arr[!this->_mirror.y * 2 + !this->_mirror.x].texCoords = {
				static_cast<float>(this->_data.textureBounds.pos.x),
				static_cast<float>(this->_data.textureBounds.pos.y + this->_data.textureBounds.size.y)
			};
			game->screen->draw(arr, sf::RenderStates(&this->_sprite.getTexture()));
		}
		if (this->showAttributes) {
			sf::VertexArray arr{sf::PrimitiveType::TriangleFan, 4};
			sf::VertexArray arr2{sf::PrimitiveType::LineStrip, 5};
			Rectangle box = {
				{this->_position.x - 4.5f, -this->_position.y - 4.5f},
				{this->_position.x + 4.5f, -this->_position.y - 4.5f},
				{this->_position.x + 4.5f, -this->_position.y + 4.5f},
				{this->_position.x - 4.5f, -this->_position.y + 4.5f},
			};

			for (int i = 0; i < 4; i++) {
				arr[i].color = Color::Black;
				arr[i].color.a *= 0x30 / 255.f;
				arr[i].position = (&box.pt1)[i];
			}
			game->screen->draw(arr);

			for (unsigned i = 0; i < 5; i++) {
				arr2[i].color = Color::Black;
				arr2[i].position = (&box.pt1)[i % 4];
			}
			game->screen->draw(arr2);
		}
	}

	void Particle::update()
	{
		if (this->_aliveTimer <= 1 || (this->_data.disabledOnHit && this->_owner.isHit())) {
			this->kill();
			if (this->_fadeTime == 0)
				return;
			this->_fadeTime--;
		} else
			this->_aliveTimer--;
		if (--this->_updateTimer)
			return;
		this->_updateTimer = random_distrib(game->battleRandom, this->_data.updateInterval.first, this->_data.updateInterval.second);
		if (this->_data.useRandomColor)
			this->_currentColors = random_distrib(game->battleRandom, 0, this->_data.colors.size());
		else
			this->_currentColors = (this->_currentColors + 1) % this->_data.colors.size();
		this->_speed.x += random_distrib(game->battleRandom, this->_data.acceleration.first.x, this->_data.acceleration.second.x);
		this->_speed.y += random_distrib(game->battleRandom, this->_data.acceleration.first.y, this->_data.acceleration.second.y);
		if (this->_speed.x < this->_data.velocity_limit.first.x)
			this->_speed.x = this->_data.velocity_limit.first.x;
		else if (this->_speed.x > this->_data.velocity_limit.second.x)
			this->_speed.x = this->_data.velocity_limit.second.x;
		if (this->_speed.y < this->_data.velocity_limit.first.y)
			this->_speed.y = this->_data.velocity_limit.first.y;
		else if (this->_speed.y > this->_data.velocity_limit.second.y)
			this->_speed.y = this->_data.velocity_limit.second.y;
		this->_position += this->_speed;
		if (random_distrib(game->battleRandom, 0, 100) < this->_data.mirror.x)
			this->_mirror.x = !this->_mirror.x;
		if (random_distrib(game->battleRandom, 0, 100) < this->_data.mirror.y)
			this->_mirror.y = !this->_mirror.y;
	}

	bool Particle::isDead() const
	{
		return this->_aliveTimer == 0 && this->_fadeTime == 0;
	}

	void Particle::kill()
	{
		this->_aliveTimer = 0;
	}

	int Particle::getLayer() const
	{
		return this->_data.layer;
	}

	unsigned int Particle::getBufferSize() const
	{
		return sizeof(Data);
	}

	void Particle::copyToBuffer(void *data) const
	{
		auto dat = reinterpret_cast<Data *>(data);

		dat->_aliveTimer = this->_aliveTimer;
		dat->_fadeTime = this->_fadeTime;
		dat->_maxFadeTime = this->_maxFadeTime;
		dat->_updateTimer = this->_updateTimer;
		dat->_currentColors = this->_currentColors;
		dat->_scale = this->_scale;
		dat->_mirror = this->_mirror;
		dat->_speed = this->_speed;
		dat->_position = this->_position;
	}

	void Particle::restoreFromBuffer(void *data)
	{
		auto dat = reinterpret_cast<Data *>(data);

		this->_aliveTimer = dat->_aliveTimer;
		this->_fadeTime = dat->_fadeTime;
		this->_maxFadeTime = dat->_maxFadeTime;
		this->_updateTimer = dat->_updateTimer;
		this->_currentColors = dat->_currentColors;
		this->_scale = dat->_scale;
		this->_mirror = dat->_mirror;
		this->_speed = dat->_speed;
		this->_position = dat->_position;
	}

	size_t Particle::printDifference(const char *msgStart, void *data1, void *data2, unsigned int startOffset) const
	{
		auto dat1 = static_cast<Data *>(data1);
		auto dat2 = static_cast<Data *>(data2);

		game->logger.info("Particle @" + std::to_string(startOffset));
		OBJECT_CHECK_FIELD("Particle", "", dat1, dat2, _position, DISP_VEC);
		OBJECT_CHECK_FIELD("Particle", "", dat1, dat2, _speed, DISP_VEC);
		OBJECT_CHECK_FIELD("Particle", "", dat1, dat2, _mirror, DISP_VEC);
		OBJECT_CHECK_FIELD("Particle", "", dat1, dat2, _aliveTimer, std::to_string);
		OBJECT_CHECK_FIELD("Particle", "", dat1, dat2, _fadeTime, std::to_string);
		OBJECT_CHECK_FIELD("Particle", "", dat1, dat2, _maxFadeTime, std::to_string);
		OBJECT_CHECK_FIELD("Particle", "", dat1, dat2, _updateTimer, std::to_string);
		OBJECT_CHECK_FIELD("Particle", "", dat1, dat2, _currentColors, std::to_string);
		OBJECT_CHECK_FIELD("Particle", "", dat1, dat2, _scale, std::to_string);
		return sizeof(Data);
	}

	size_t Particle::printContent(const char *msgStart, void *data, unsigned int startOffset, size_t dataSize) const
	{
		auto dat = static_cast<Data *>(data);

		game->logger.info("Particle @" + std::to_string(startOffset));
		if (startOffset + sizeof(Data) >= dataSize)
			game->logger.warn("Object is " + std::to_string(startOffset + sizeof(Data) - dataSize) + " bytes bigger than input");

		DISPLAY_FIELD("Particle", "", dat, _position, DISP_VEC);
		DISPLAY_FIELD("Particle", "", dat, _speed, DISP_VEC);
		DISPLAY_FIELD("Particle", "", dat, _mirror, DISP_VEC);
		DISPLAY_FIELD("Particle", "", dat, _aliveTimer, std::to_string);
		DISPLAY_FIELD("Particle", "", dat, _fadeTime, std::to_string);
		DISPLAY_FIELD("Particle", "", dat, _maxFadeTime, std::to_string);
		DISPLAY_FIELD("Particle", "", dat, _updateTimer, std::to_string);
		DISPLAY_FIELD("Particle", "", dat, _currentColors, std::to_string);
		DISPLAY_FIELD("Particle", "", dat, _scale, std::to_string);
		if (startOffset + sizeof(Data) >= dataSize) {
			game->logger.fatal("Invalid input frame");
			return 0;
		}
		return sizeof(Data);
	}

	unsigned int Particle::getClassId() const
	{
		return CLASS_ID;
	}

	const Particle::Source &Particle::getSource() const
	{
		return this->_source;
	}

	const Character &Particle::getOwner() const
	{
		return this->_owner;
	}

	Particle::InitData::InitData(const nlohmann::json &data, const std::vector<Color> &colors)
	{
		// TODO: Do better error detection
		for (auto &color : data["colors"])
			this->colors.push_back({ colors[color[0]], colors[color[1]], colors[color[2]], colors[color[3]] });
		this->useRandomColor = data["use_random_color"];
		this->disabledOnHit = data["disabled_on_hit"];
		this->updateInterval.first = data["update_interval"][0];
		this->updateInterval.second = data["update_interval"][1];
		this->textureBounds.pos.x = data["bounds"]["left"];
		this->textureBounds.pos.y = data["bounds"]["top"];
		this->textureBounds.size.x = data["bounds"]["width"];
		this->textureBounds.size.y = data["bounds"]["height"];
		this->scale.first = data["scale"][0];
		this->scale.second = data["scale"][1];
		this->mirror.x = data["mirror"][0];
		this->mirror.y = data["mirror"][1];
		this->rotation.first = data["rotation"][0];
		this->rotation.second = data["rotation"][1];
		this->lifeSpan.first = data["life_span"][0];
		this->lifeSpan.second = data["life_span"][1];
		this->fadeTime.first = data["fade_time"][0];
		this->fadeTime.second = data["fade_time"][1];
		this->acceleration.first.x = data["acceleration"][0]["x"];
		this->acceleration.first.y = data["acceleration"][0]["y"];
		this->acceleration.second.x = data["acceleration"][1]["x"];
		this->acceleration.second.y = data["acceleration"][1]["y"];
		this->velocity_limit.first.x = data["min_velocity"]["x"];
		this->velocity_limit.first.y = data["min_velocity"]["y"];
		this->velocity_limit.second.x = data["max_velocity"]["x"];
		this->velocity_limit.second.y = data["max_velocity"]["y"];
		this->layer = data["layer"];
		this->spawnChance = data["spawn_chance"];
	}
}
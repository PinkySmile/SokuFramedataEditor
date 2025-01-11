//
// Created by PinkySmile on 18/09/2021
//

#ifndef BATTLE_OVERLAYOBJECT_HPP
#define BATTLE_OVERLAYOBJECT_HPP


#include <SoFGV.hpp>

struct SokuColor {
	union {
		struct {
			unsigned char a;
			unsigned char r;
			unsigned char g;
			unsigned char b;
		};
		unsigned color;
	};

	SokuColor(unsigned c) : color(c) {}
	SokuColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a) : a(a), r(r), g(g), b(b) {}
	operator sf::Color() { return {r, g, b, a}; }
};

class OverlayObject : public SpiralOfFate::IObject {
public:
	float scale = 1;
	SpiralOfFate::Vector2f translate = {0, 0};
	bool displayScaled = false;
	mutable SpiralOfFate::FrameData *_setuped;

	mutable SpiralOfFate::Sprite _sprite;
	std::map<unsigned, std::vector<std::vector<SpiralOfFate::FrameData>>> &_moves;

	SpiralOfFate::Vector2f _position = {0, 0};
	SpiralOfFate::Vector2f _speed = {0, 0};
	float _gravity = 0;
	unsigned _action = 0;
	unsigned _actionBlock = 0;
	unsigned _animation = 0;
	unsigned _animationCtr = 0;

	OverlayObject(std::map<unsigned, std::vector<std::vector<SpiralOfFate::FrameData>>> &moves);
	~OverlayObject() override = default;
	void render(bool setup) const;

	void render() const override;

	void update() override;
	bool collides(const SpiralOfFate::IObject &other) const override;
	void reset() override;
	bool isDead() const override;
	bool hits(const SpiralOfFate::IObject &other) const override;
	void hit(SpiralOfFate::IObject &other, const SpiralOfFate::FrameData *data) override;
	void getHit(SpiralOfFate::IObject &other, const SpiralOfFate::FrameData *data) override;
	const SpiralOfFate::FrameData *getCurrentFrameData() const override;
	void collide(SpiralOfFate::IObject &other) override;
	void kill() override;
	unsigned int getBufferSize() const override;
	void copyToBuffer(void *data) const override;
	void restoreFromBuffer(void *data) override;
	unsigned int getClassId() const override;
	std::pair<SpiralOfFate::Vector2i, SpiralOfFate::Vector2u> setupSprite() const;
};


#endif //BATTLE_OVERLAYOBJECT_HPP

//
// Created by PinkySmile on 18/09/2021.
//

#ifndef BATTLE_EditableObject_HPP
#define BATTLE_EditableObject_HPP


#include <LibCore.hpp>

class EditableObject {
public:
	std::map<unsigned, std::vector<std::vector<SpiralOfFate::FrameData>>> _moves;
	SpiralOfFate::Vector2f _position = {0, 0};
	SpiralOfFate::Vector2f _speed = {0, 0};
	unsigned _action = 0;
	unsigned _actionBlock = 0;
	unsigned _animation = 0;
	unsigned _animationCtr = 0;
	float _rotation = 0;
	std::string _folder;

	std::vector<SpiralOfFate::Rectangle> _getModifiedBoxes(const SpiralOfFate::FrameData &data, const std::vector<SpiralOfFate::Box> &boxes) const;
	std::vector<SpiralOfFate::Rectangle> _getModifiedHurtBoxes() const;
	std::vector<SpiralOfFate::Rectangle> _getModifiedHitBoxes() const;
	void _simulate(const SpiralOfFate::FrameData &data);

	EditableObject() = default;
	EditableObject(const std::string &frameData);
	~EditableObject() = default;
	SpiralOfFate::FrameData &getFrameData();
	const SpiralOfFate::FrameData &getFrameData() const;
	void render(sf::RenderTarget &target, sf::RenderStates states) const;
	void update();
	void resetState();
};


#endif //BATTLE_EditableObject_HPP

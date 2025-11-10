//
// Created by PinkySmile on 18/09/2021.
//

#ifndef BATTLE_EDITABLEOBJECT_HPP
#define BATTLE_EDITABLEOBJECT_HPP


#include <LibCore.hpp>

class EditableObject {
public:
	SpiralOfFate::Sprite _sprite;
	SpiralOfFate::Sprite _spriteEffect;
	sf::Sprite _overlaySprite{this->_overlayTexture};
	sf::Texture _overlayTexture;
	std::map<unsigned, std::vector<std::vector<SpiralOfFate::FrameData>>> _moves;
	SpiralOfFate::Vector2f _position = {0, 0};
	SpiralOfFate::Vector2f _speed = {0, 0};
	SpiralOfFate::Vector2f _mousePos = {0, 0};
	unsigned _action = 0;
	unsigned _actionBlock = 0;
	unsigned _animation = 0;
	unsigned _animationCtr = 0;
	int _paletteIndex = -1;
	float _rotation = 0;
	std::string _folder;
	bool _textureValid = false;
	bool _needGenerate = false;
	unsigned char _generateCd = 0;

	SpiralOfFate::Vector2f _mousePosToImgPos(const SpiralOfFate::Vector2i &mouse);
	void _generateOverlaySprite();
	std::vector<SpiralOfFate::Rectangle> _getModifiedBoxes(bool displaceObject, const SpiralOfFate::FrameData &data, const std::vector<SpiralOfFate::Box> &boxes) const;
	std::vector<SpiralOfFate::Rectangle> _getModifiedHurtBoxes(bool displaceObject) const;
	std::vector<SpiralOfFate::Rectangle> _getModifiedHitBoxes(bool displaceObject) const;
	void _simulate(const SpiralOfFate::FrameData &data);

	EditableObject() = default;
	EditableObject(const std::string &frameData);
	~EditableObject() = default;
	SpiralOfFate::FrameData &getFrameData();
	const SpiralOfFate::FrameData &getFrameData() const;
	void render(sf::RenderTarget &target, sf::RenderStates states, bool displaceBoxes);
	void update();
	void resetState();
	void setMousePosition(const SpiralOfFate::Vector2f *pos);
};


#endif //BATTLE_EDITABLEOBJECT_HPP

//
// Created by PinkySmile on 18/09/2021.
//

#ifndef BATTLE_EDITABLEOBJECT_HPP
#define BATTLE_EDITABLEOBJECT_HPP


#include <vector>
#include <LibCore.hpp>
#include "Rectangle.hpp"

class EditableObject {
public:
	SpiralOfFate::Sprite _sprite;
	sf::Sprite _overlaySprite{this->_overlayTexture};
	sf::Texture _overlayTexture;
	SpiralOfFate::FrameData::LoadedSchema _schema;
	std::optional<SpiralOfFate::Vector2f> _mousePos = {};
	unsigned _action = 0;
	unsigned _actionBlock = 0;
	unsigned _animation = 0;
	unsigned _animationCtr = 0;
	int _paletteIndex = -1;
	int _oldPaletteIndex = -1;
	std::string _folder;
	bool _textureValid = false;
	bool _needGenerate = false;
	unsigned char _generateCd = 0;
	std::function<void (int, int)> _onHoverChange;

	SpiralOfFate::Vector2f _mousePosToImgPos(const SpiralOfFate::Vector2i &mouse);
	void _generateOverlaySprite();
	std::vector<FDE::Rectangle> _getModifiedBoxes(const SpiralOfFate::FrameData &data, const std::vector<ShadyCore::Schema::Sequence::BBox> &boxes) const;
	std::vector<FDE::Rectangle> _getModifiedHurtBoxes() const;
	std::vector<FDE::Rectangle> _getModifiedHitBoxes() const;

	EditableObject() = default;
	EditableObject(const std::string &folder, const std::string &frameData, const std::array<SpiralOfFate::Color, 256> *palette);
	EditableObject(const std::string &folder, const std::filesystem::path &frameData, const std::array<SpiralOfFate::Color, 256> *palette);
	~EditableObject() = default;
	SpiralOfFate::FrameData::Sequence &getSequence();
	const SpiralOfFate::FrameData::Sequence &getSequence() const;
	SpiralOfFate::FrameData &getFrameData();
	const SpiralOfFate::FrameData &getFrameData() const;
	void render(sf::RenderTarget &target, sf::RenderStates states);
	void update();
	void resetState();
	void setMousePosition(const SpiralOfFate::Vector2f *pos);
	void tick();
};


#endif //BATTLE_EDITABLEOBJECT_HPP

//
// Created by PinkySmile on 11/01/2026.
//

#ifndef SOKUFD_EDITOR_RECTANGLE_HPP
#define SOKUFD_EDITOR_RECTANGLE_HPP


#include "Data/Vector.hpp"

namespace FDE {
	struct Rectangle {
		SpiralOfFate::Vector2f pt1;
		SpiralOfFate::Vector2f pt2;
		SpiralOfFate::Vector2f pt3;
		SpiralOfFate::Vector2f pt4;

		bool contains(const SpiralOfFate::Vector2f &point);
	};
}


#endif //SOKUFDEDITOR_RECTANGLE_HPP
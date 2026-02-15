//
// Created by PinkySmile on 11/01/2026.
//

#include "Rectangle.hpp"

bool FDE::Rectangle::contains(const SpiralOfFate::Vector2f &point)

{
	// 0 <= dot(AB,AM) <= dot(AB,AB) &&
	// 0 <= dot(BC,BM) <= dot(BC,BC)
	auto AB = this->pt2 - this->pt1;
	auto BC = this->pt3 - this->pt2;
	auto AM = point - this->pt1;
	auto BM = point - this->pt2;
	auto dotAB_AM = AB * AM;

	if (0 > dotAB_AM)
		return false;

	auto dotAB_AB = AB * AB;

	if (dotAB_AM > dotAB_AB)
		return false;

	auto dotBC_BM = BC * BM;

	if (0 > dotBC_BM)
		return false;

	auto dotBC_BC = BC * BC;

	if (dotBC_BM > dotBC_BC)
		return false;

	return true;
}

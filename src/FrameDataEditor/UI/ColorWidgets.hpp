//
// Created by PinkySmile on 26/10/2025.
//

#ifndef SOFGV_COLORWIDGETS_HPP
#define SOFGV_COLORWIDGETS_HPP


#include <functional>
#include <array>
#include <map>
#include <SFML/Graphics/Color.hpp>
#include "Color.hpp"

typedef std::function<SpiralOfFate::Color(unsigned, unsigned, unsigned)> ColorConversionCb;
typedef std::array<std::pair<unsigned, unsigned>, 3> ColorSpaceRanges;
typedef std::array<unsigned, 3> WidgetColor;
typedef std::function<WidgetColor(SpiralOfFate::Color)> ReverseColorConversionCb;


#endif //SOFGV_COLORWIDGETS_HPP

//
// Created by PinkySmile on 04/01/2026.
//

#include "EditColorsOperation.hpp"

namespace SpiralOfFate
{
	EditColorsOperation::EditColorsOperation(
		const std::string &&name,
		MainWindow::Palette &palette,
		unsigned &selectedPalette,
		std::array<Color, 256> newValue
	) :
		_palette(palette),
		_oldValue(palette.colors),
		_newValue(newValue),
		_fieldName(name),
		_oldSelectedPalette(selectedPalette),
		_selectedPalette(selectedPalette)
	{
	}

	void EditColorsOperation::apply()
	{
		this->_selectedPalette = this->_oldSelectedPalette;
		this->_palette.colors = this->_newValue;
		this->_palette.modifications++;
	}

	void EditColorsOperation::undo()
	{
		this->_selectedPalette = this->_oldSelectedPalette;
		this->_palette.colors = this->_oldValue;
		this->_palette.modifications--;
	}

	std::string EditColorsOperation::getName() const noexcept
	{
		return this->_fieldName;
	}

	bool EditColorsOperation::hasModification() const
	{
		return this->_oldValue != this->_newValue;
	}

	bool EditColorsOperation::hasFramedataModification() const
	{
		return false;
	}
} // SpiralOfFate
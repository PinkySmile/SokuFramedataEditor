//
// Created by PinkySmile on 06/11/2025.
//

#include "EditColorOperation.hpp"

namespace SpiralOfFate
{
	EditColorOperation::EditColorOperation(
		const std::string &&name,
		MainWindow::Palette &palette,
		unsigned &selectedPalette,
		unsigned &selectedColor,
		Color newValue
	) :
		_palette(palette),
		_oldValue(palette.colors[selectedColor]),
		_newValue(newValue),
		_fieldName(name),
		_oldSelectedPalette(selectedPalette),
		_oldSelectedColor(selectedColor),
		_selectedPalette(selectedPalette),
		_selectedColor(selectedColor)
	{
	}

	void EditColorOperation::apply()
	{
		if (this->_selectedColor == 0 || this->_selectedColor >= 251)
			return;
		this->_selectedPalette = this->_oldSelectedPalette;
		this->_selectedColor = this->_oldSelectedColor;
		this->_palette.colors[this->_selectedColor] = this->_newValue;
		this->_palette.modifications++;
	}

	void EditColorOperation::undo()
	{
		if (this->_selectedColor == 0 || this->_selectedColor >= 251)
			return;
		this->_selectedPalette = this->_oldSelectedPalette;
		this->_selectedColor = this->_oldSelectedColor;
		this->_palette.colors[this->_selectedColor] = this->_oldValue;
		this->_palette.modifications--;
	}

	std::string EditColorOperation::getName() const noexcept
	{
		return this->_fieldName;
	}

	bool EditColorOperation::hasModification() const
	{
		if (this->_selectedColor == 0 || this->_selectedColor >= 251)
			return false;
		return this->_oldValue != this->_newValue;
	}

	bool EditColorOperation::hasFramedataModification() const
	{
		return false;
	}
}

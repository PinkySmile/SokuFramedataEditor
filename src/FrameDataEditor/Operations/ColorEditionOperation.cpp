//
// Created by PinkySmile on 06/11/2025.
//

#include "ColorEditionOperation.hpp"

namespace SpiralOfFate
{
	ColorEditionOperation::ColorEditionOperation(
		const std::string &&name,
		MainWindow::Palette &palette,
		unsigned &selectedPalette,
		unsigned &selectedColor,
		Color newValue,
		std::function<void()> updateUi
	) :
		_palette(palette),
		_oldValue(palette.colors[selectedColor]),
		_newValue(newValue),
		_fieldName(name),
		_wasModified(palette.modified),
		_oldSelectedPalette(selectedPalette),
		_oldSelectedColor(selectedColor),
		_selectedPalette(selectedPalette),
		_selectedColor(selectedColor),
		_updateUi(std::move(updateUi))
	{
	}

	void ColorEditionOperation::apply()
	{
		if (this->_selectedColor == 0 || this->_selectedColor >= 251)
			return;
		this->_selectedPalette = this->_oldSelectedPalette;
		this->_selectedColor = this->_oldSelectedColor;
		this->_palette.colors[this->_selectedColor] = this->_newValue;
		this->_palette.modified = true;
		this->_updateUi();
	}

	void ColorEditionOperation::undo()
	{
		if (this->_selectedColor == 0 || this->_selectedColor >= 251)
			return;
		this->_selectedPalette = this->_oldSelectedPalette;
		this->_selectedColor = this->_oldSelectedColor;
		this->_palette.colors[this->_selectedColor] = this->_oldValue;
		this->_palette.modified = this->_wasModified;
		this->_updateUi();
	}

	std::string ColorEditionOperation::getName() const noexcept
	{
		return this->_fieldName;
	}

	bool ColorEditionOperation::hasModification() const
	{
		if (this->_selectedColor == 0 || this->_selectedColor >= 251)
			return false;
		return this->_oldValue != this->_newValue;
	}
}
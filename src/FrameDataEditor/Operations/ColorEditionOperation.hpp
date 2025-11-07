//
// Created by PinkySmile on 09/05/25.
//

#ifndef SOFGV_COLOREDITIONOPERATION_HPP
#define SOFGV_COLOREDITIONOPERATION_HPP


#include "Operation.hpp"
#include "../EditableObject.hpp"
#include "../UI/MainWindow.hpp"

namespace SpiralOfFate
{
	class ColorEditionOperation : public Operation {
	protected:
		MainWindow::Palette &_palette;
		Color _oldValue;
		Color _newValue;
		std::string _fieldName;
		bool _wasModified;
		unsigned _oldSelectedPalette;
		unsigned _oldSelectedColor;
		unsigned &_selectedPalette;
		unsigned &_selectedColor;
		std::function<void()> _updateUi;

	public:
		ColorEditionOperation(
			const std::string &&name,
			MainWindow::Palette &palette,
			unsigned &selectedPalette,
			unsigned &selectedColor,
			Color newValue,
			std::function<void()> updateUI
		);
		void apply() override;
		void undo() override;
		std::string getName() const noexcept override;
		bool hasModification() const override;
	};
}


#endif //SOFGV_COLOREDITIONOPERATION_HPP

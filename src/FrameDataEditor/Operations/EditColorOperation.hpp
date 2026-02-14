//
// Created by PinkySmile on 09/05/25.
//

#ifndef SOFGV_COLOREDITIONOPERATION_HPP
#define SOFGV_COLOREDITIONOPERATION_HPP


#include "Operation.hpp"
#include "../UI/MainWindow.hpp"

namespace SpiralOfFate
{
	class EditColorOperation : public Operation {
	protected:
		MainWindow::Palette &_palette;
		Color _oldValue;
		Color _newValue;
		std::string _fieldName;
		unsigned _oldSelectedPalette;
		unsigned _oldSelectedColor;
		unsigned &_selectedPalette;
		unsigned &_selectedColor;

	public:
		EditColorOperation(
			const std::string &&name,
			MainWindow::Palette &palette,
			unsigned &selectedPalette,
			unsigned &selectedColor,
			Color newValue
		);
		void apply() override;
		void undo() override;
		[[nodiscard]] std::string getName() const noexcept override;
		[[nodiscard]] bool hasModification() const override;
		[[nodiscard]] bool hasFramedataModification() const override;
	};
}


#endif //SOFGV_COLOREDITIONOPERATION_HPP

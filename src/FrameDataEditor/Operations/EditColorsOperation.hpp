//
// Created by PinkySmile on 04/01/2026.
//

#ifndef SOFGV_EDITCOLORSOPERATION_HPP
#define SOFGV_EDITCOLORSOPERATION_HPP


#include "Operation.hpp"
#include "../UI/MainWindow.hpp"

namespace SpiralOfFate
{
	class EditColorsOperation : public Operation {
	protected:
		MainWindow::Palette &_palette;
		std::array<Color, 256> _oldValue;
		std::array<Color, 256> _newValue;
		std::string _fieldName;
		unsigned _oldSelectedPalette;
		unsigned &_selectedPalette;

	public:
		EditColorsOperation(
			const std::string &&name,
			MainWindow::Palette &palette,
			unsigned &selectedPalette,
			std::array<Color, 256> newValue
		);
		void apply() override;
		void undo() override;
		[[nodiscard]] std::string getName() const noexcept override;
		[[nodiscard]] bool hasModification() const override;
		[[nodiscard]] bool hasFramedataModification() const override;
	};
} // SpiralOfFate


#endif //SOFGV_EDITCOLORSOPERATION_HPP
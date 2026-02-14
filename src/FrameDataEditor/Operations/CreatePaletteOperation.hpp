//
// Created by PinkySmile on 07/11/2025.
//

#ifndef SOFGV_CREATEPALETTEOPERATION_HPP
#define SOFGV_CREATEPALETTEOPERATION_HPP


#include "Operation.hpp"
#include "../EditableObject.hpp"
#include "../UI/MainWindow.hpp"

namespace SpiralOfFate
{
	class CreatePaletteOperation : public Operation {
	private:
		std::vector<MainWindow::Palette> &_list;
		std::string _name;
		MainWindow::Palette _source;
		unsigned _startIndex;
		const unsigned &_index;
		std::function<void (unsigned)> _setIndex;

	public:
		CreatePaletteOperation(std::vector<MainWindow::Palette> &list, const std::string &&name, const MainWindow::Palette &source, const unsigned &index, const std::function<void (unsigned)> &setIndex);
		void apply() override;
		void undo() override;
		[[nodiscard]] bool hasModification() const override;
		[[nodiscard]] std::string getName() const noexcept override;
		[[nodiscard]] bool hasFramedataModification() const override;
	};
}


#endif //SOFGV_CREATEPALETTEOPERATION_HPP

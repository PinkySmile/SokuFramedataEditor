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
		unsigned &_index;

	public:
		CreatePaletteOperation(std::vector<MainWindow::Palette> &list, const std::string &&name, const MainWindow::Palette &source, unsigned &index);
		void apply() override;
		void undo() override;
		bool hasModification() const override;
		std::string getName() const noexcept override;
	};
}


#endif //SOFGV_CREATEPALETTEOPERATION_HPP

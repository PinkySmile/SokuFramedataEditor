//
// Created by PinkySmile on 07/11/2025.
//

#ifndef SOFGV_REMOVEPALETTEOPERATION_HPP
#define SOFGV_REMOVEPALETTEOPERATION_HPP


#include "Operation.hpp"
#include "../EditableObject.hpp"
#include "../UI/MainWindow.hpp"

namespace SpiralOfFate
{
	class RemovePaletteOperation : public Operation {
	private:
		std::vector<MainWindow::Palette> &_list;
		std::string _name;
		unsigned _id;
		unsigned &_index;
		MainWindow::Palette _old;

	public:
		RemovePaletteOperation(std::vector<MainWindow::Palette> &list, const std::string &&name, unsigned &index);
		void apply() override;
		void undo() override;
		bool hasModification() const override;
		std::string getName() const noexcept override;
	};
}


#endif //SOFGV_REMOVEPALETTEOPERATION_HPP

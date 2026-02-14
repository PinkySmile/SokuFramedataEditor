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
		const unsigned &_index;
		std::function<void (unsigned)> _setIndex;
		MainWindow::Palette _old;

	public:
		RemovePaletteOperation(std::vector<MainWindow::Palette> &list, const std::string &&name, const unsigned &index, const std::function<void (unsigned)> &setIndex);
		void apply() override;
		void undo() override;
		[[nodiscard]] bool hasModification() const override;
		[[nodiscard]] std::string getName() const noexcept override;
		[[nodiscard]] bool hasFramedataModification() const override;
	};
}


#endif //SOFGV_REMOVEPALETTEOPERATION_HPP

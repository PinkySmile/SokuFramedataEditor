//
// Created by PinkySmile on 07/11/2025.
//

#include "CreatePaletteOperation.hpp"

namespace SpiralOfFate
{
	CreatePaletteOperation::CreatePaletteOperation(
		std::vector<MainWindow::Palette> &list,
		const std::string &&name,
		const MainWindow::Palette &source,
		unsigned &index
	) :
		_list(list),
		_name(name),
		_source(source),
		_index(index)
	{
	}

	void CreatePaletteOperation::apply()
	{
		this->_list.push_back(this->_source);
		this->_index = this->_list.size() - 1;
	}

	void CreatePaletteOperation::undo()
	{
		this->_list.pop_back();
		if (this->_index >= this->_list.size())
			this->_index = this->_list.size() - 1;
	}

	bool CreatePaletteOperation::hasModification() const
	{
		return true;
	}

	std::string CreatePaletteOperation::getName() const noexcept
	{
		return this->_name;
	}
}
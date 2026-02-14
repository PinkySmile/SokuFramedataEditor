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
		const unsigned &index,
		const std::function<void (unsigned)> &setIndex
	) :
		_list(list),
		_name(name),
		_source(source),
		_startIndex(index),
		_index(index),
		_setIndex(setIndex)
	{
	}

	void CreatePaletteOperation::apply()
	{
		this->_list.push_back(this->_source);
		this->_setIndex(this->_list.size() - 1);
	}

	void CreatePaletteOperation::undo()
	{
		this->_list.pop_back();
		this->_setIndex(this->_startIndex);
	}

	bool CreatePaletteOperation::hasModification() const
	{
		return true;
	}

	std::string CreatePaletteOperation::getName() const noexcept
	{
		return this->_name;
	}

	bool CreatePaletteOperation::hasFramedataModification() const
	{
		return false;
	}
}

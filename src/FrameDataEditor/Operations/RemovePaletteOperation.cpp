//
// Created by PinkySmile on 07/11/2025.
//

#include "RemovePaletteOperation.hpp"

namespace SpiralOfFate
{
	RemovePaletteOperation::RemovePaletteOperation(std::vector<MainWindow::Palette> &list, const std::string &&name, unsigned int &index) :
		_list(list),
		_name(name),
		_id(index),
		_index(index),
		_old(list[index])
	{
	}

	void RemovePaletteOperation::apply()
	{
		if (this->_list.size() <= 1)
			return;
		this->_list.erase(this->_list.begin() + this->_id);
		if (this->_index >= this->_list.size())
			this->_index = this->_list.size() - 1;
	}

	void RemovePaletteOperation::undo()
	{
		this->_list.insert(this->_list.begin() + this->_id, this->_old);
		this->_index = this->_id;
	}

	bool RemovePaletteOperation::hasModification() const
	{
		return this->_list.size() > 1;
	}

	std::string RemovePaletteOperation::getName() const noexcept
	{
		return this->_name;
	}
}
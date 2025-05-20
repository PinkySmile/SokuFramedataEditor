//
// Created by PinkySmile on 20/05/25.
//

#include "DummyOperation.hpp"

void SpiralOfFate::DummyOperation::apply()
{
}

void SpiralOfFate::DummyOperation::undo()
{
}

bool SpiralOfFate::DummyOperation::hasModification() const
{
	return false;
}

std::string SpiralOfFate::DummyOperation::getName() const noexcept
{
	return "";
}

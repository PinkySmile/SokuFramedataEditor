//
// Created by PinkySmile on 20/05/25.
//

#ifndef SOFGV_DUMMYOPERATION_HPP
#define SOFGV_DUMMYOPERATION_HPP


#include "IOperation.hpp"

namespace SpiralOfFate
{
	class DummyOperation : public IOperation {
	public:
		void apply() override;
		void undo() override;
		bool hasModification() const override;
		std::string getName() const noexcept override;
	};
}


#endif //SOFGV_DUMMYOPERATION_HPP

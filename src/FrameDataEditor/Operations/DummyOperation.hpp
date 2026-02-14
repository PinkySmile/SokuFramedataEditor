//
// Created by PinkySmile on 20/05/25.
//

#ifndef SOFGV_DUMMYOPERATION_HPP
#define SOFGV_DUMMYOPERATION_HPP


#include "Operation.hpp"

namespace SpiralOfFate
{
	class DummyOperation : public Operation {
	public:
		void apply() override;
		void undo() override;
		[[nodiscard]] bool hasModification() const override;
		[[nodiscard]] std::string getName() const noexcept override;
		[[nodiscard]] bool hasFramedataModification() const override;
	};
}


#endif //SOFGV_DUMMYOPERATION_HPP

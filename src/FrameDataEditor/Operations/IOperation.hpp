//
// Created by PinkySmile on 06/05/25.
//

#ifndef SOFGV_IOPERATION_HPP
#define SOFGV_IOPERATION_HPP


#include <string>

namespace SpiralOfFate
{
	class IOperation {
	public:
		virtual ~IOperation() = default;
		virtual void apply() = 0;
		virtual void undo() = 0;
		[[nodiscard]] virtual std::string getName() const noexcept = 0;
	};
}


#endif //SOFGV_IOPERATION_HPP

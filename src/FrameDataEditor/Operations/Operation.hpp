//
// Created by PinkySmile on 06/05/25.
//

#ifndef SOFGV_OPERATION_HPP
#define SOFGV_OPERATION_HPP


#include <string>

namespace SpiralOfFate
{
	class Operation {
	public:
		virtual ~Operation() = default;
		virtual void apply() = 0;
		virtual void undo() = 0;
		[[nodiscard]] virtual bool hasModification() const = 0;
		[[nodiscard]] virtual std::string getName() const noexcept = 0;
	};
}


#endif //SOFGV_OPERATION_HPP

//
// Created by PinkySmile on 30/12/2025.
//

#ifndef SOFGV_REMOVEMOVEOPERATION_HPP
#define SOFGV_REMOVEMOVEOPERATION_HPP


#include "Operation.hpp"
#include "../EditableObject.hpp"

namespace SpiralOfFate
{
	class RemoveMoveOperation : public Operation {
	private:
		EditableObject &_obj;
		std::string _name;
		unsigned _id;
		std::vector<std::vector<FrameData>> _oldData;

	public:
		RemoveMoveOperation(EditableObject &obj, const std::string &&name, unsigned id);
		void apply() override;
		void undo() override;
		bool hasModification() const override;
		std::string getName() const noexcept override;
	};
}


#endif //SOFGV_REMOVEMOVEOPERATION_HPP
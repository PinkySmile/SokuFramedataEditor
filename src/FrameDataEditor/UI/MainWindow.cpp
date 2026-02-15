//
// Created by PinkySmile on 05/05/25.
//

#include <TGUI/RendererDefines.hpp>
#include "MainWindow.hpp"

#include <ranges>

#include "ColorPlaneWidget.hpp"
#include "ColorSliderWidget.hpp"
#include "../Operations/DummyOperation.hpp"
#include "../Operations/FlagOperation.hpp"
#include "../Operations/BasicDataOperation.hpp"
#include "../Operations/BasicDataOperationSeq.hpp"
#include "../Operations/BasicData2StepOperation.hpp"
#include "../Operations/BasicDataDoubleOperation.hpp"
#include "../Operations/BasicDataDouble2Operation.hpp"
#include "../Operations/BasicDataQuadOperation.hpp"
#include "../Operations/CleanPropertiesOperation.hpp"
#include "../Operations/EditSpriteOperation.hpp"
#include "../Operations/ClearAttackOperation.hpp"
#include "../Operations/CreateMoveOperation.hpp"
#include "../Operations/CreateFrameOperation.hpp"
#include "../Operations/CreateBlockOperation.hpp"
#include "../Operations/CreateBoxOperation.hpp"
#include "../Operations/EditColorOperation.hpp"
#include "../Operations/CreatePaletteOperation.hpp"
#include "../Operations/EditColorsOperation.hpp"
#include "../Operations/FlattenCollisionBoxesOperation.hpp"
#include "../Operations/FlattenPropretiesOperation.hpp"
#include "../Operations/RemovePaletteOperation.hpp"
#include "../Operations/RemoveBoxOperation.hpp"
#include "../Operations/RemoveFrameOperation.hpp"
#include "../Operations/RemoveBlockOperation.hpp"
#include "../Operations/RemoveMoveOperation.hpp"
#include "../Operations/PasteDataOperation.hpp"
#include "../Operations/PasteAnimDataOperation.hpp"
#include "../Operations/PasteBoxDataOperation.hpp"
#include "Resources/Assert.hpp"

static ColorConversionCb colorConversions[] = {
	[](unsigned r, unsigned g, unsigned b) {
		return SpiralOfFate::Color(r, g, b, 255);
	},
	[](unsigned h, unsigned s1k, unsigned l1k) { // https://en.wikipedia.org/wiki/HSL_and_HSV#HSL_to_RGB_alternative
		double s = s1k / 1000.;
		double l = l1k / 1000.;
		auto f = [h, s, l](unsigned n){
			double k = std::fmod(n + h / 30., 12);
			double a = s * std::min(l, 1 - l);

			return l - a * std::max(-1., std::min(1., std::min(k - 3, 9 - k)));
		};

		return SpiralOfFate::Color(f(0) * 255, f(8) * 255, f(4) * 255, 255);
	},
	[](unsigned h, unsigned s1k, unsigned v1k) { // https://en.wikipedia.org/wiki/HSL_and_HSV#HSV_to_RGB_alternative
		double s = s1k / 1000.;
		double v = v1k / 1000.;
		auto f = [h, s, v](unsigned n){
			double k = std::fmod(n + h / 60., 6);

			return v - v * s * std::max(0., std::min(k, std::min(4 - k, 1.)));
		};

		return SpiralOfFate::Color(f(5) * 255, f(3) * 255, f(1) * 255, 255);
	}
};
static ReverseColorConversionCb colorConversionsReverse[] = {
	[](const SpiralOfFate::Color &color) {
		return WidgetColor{color.r, color.g, color.b};
	},
	[](const SpiralOfFate::Color &color) { // https://en.wikipedia.org/wiki/HSL_and_HSV#From_RGB
		double r = color.r / 255.;
		double g = color.g / 255.;
		double b = color.b / 255.;
		double xmax = std::max(r, std::max(g, b));
		double xmin = std::min(r, std::min(g, b));
		double chroma = xmax - xmin;
		double h;
		double s;
		double v = xmax;
		double l = (xmax + xmin) / 2;

		if (chroma == 0)
			h = 0;
		else if (xmax == r)
			h = 60 * std::fmod((g - b) / static_cast<float>(chroma), 6);
		else if (xmax == g)
			h = 60 * ((b - r) / static_cast<float>(chroma) + 2);
		else if (xmax == b)
			h = 60 * ((r - g) / static_cast<float>(chroma) + 4);
		else
			assert_not_reached();
		h = std::fmod(h + 360, 360);
		if (l == 0 || l == 1)
			s = 0;
		else
			s = (v - l) / std::min(l, 1 - l);
		return WidgetColor{
			static_cast<unsigned>(h),
			static_cast<unsigned>(1000 * s),
			static_cast<unsigned>(1000 * l)
		};
	},
	[](const SpiralOfFate::Color &color) { // https://en.wikipedia.org/wiki/HSL_and_HSV#From_RGB
		double r = color.r / 255.;
		double g = color.g / 255.;
		double b = color.b / 255.;
		double xmax = std::max(r, std::max(g, b));
		double xmin = std::min(r, std::min(g, b));
		double chroma = xmax - xmin;
		double h;
		double s;
		double v = xmax;

		if (chroma == 0)
			h = 0;
		else if (xmax == r)
			h = 60 * std::fmod((g - b) / static_cast<float>(chroma), 6);
		else if (xmax == g)
			h = 60 * ((b - r) / static_cast<float>(chroma) + 2);
		else if (xmax == b)
			h = 60 * ((r - g) / static_cast<float>(chroma) + 4);
		else
			assert_not_reached();
		h = std::fmod(h + 360, 360);
		if (v == 0)
			s = 0;
		else
			s = chroma / v;
		return WidgetColor{
			static_cast<unsigned>(h),
			static_cast<unsigned>(1000 * s),
			static_cast<unsigned>(1000 * v)
		};
	},
};

static ColorSpaceRanges colorSpaces[] = {
	{std::pair{0U, 255U}, std::pair{0U, 255U}, std::pair{0U, 255U}}, // RGB
	{std::pair{0U, 359U}, std::pair{0U, 1000U},std::pair{0U, 1000U}},// HSL
	{std::pair{0U, 359U}, std::pair{0U, 1000U},std::pair{0U, 1000U}},// HSV
};


template<typename T>
std::string to_string(T value, int)
{
	return std::to_string(value);
}

template<>
std::string to_string(float value, int pres)
{
	char buffer[64];

	sprintf(buffer, "%.*f", pres, value);
	return buffer;
}

std::string to_hex(unsigned long long value, int count)
{
	char buffer[17];

	sprintf(buffer, "%0*llX", count, value);
	return buffer;
}

#define PLACE_HOOK_WIDGET(elem, src) do {                                                                \
        if (!elem)                                                                                       \
                break;                                                                                   \
        elem->onChangeStarted([this]{                                                                    \
                if (this->_palettes.empty())                                                             \
                        return;                                                                          \
                this->startTransaction();                                                                \
        });                                                                                              \
        elem->onChanged([this](WidgetColor color){                                                       \
                if (this->_palettes.empty())                                                             \
                        return;                                                                          \
                this->_colorChangeSource = src;                                                          \
                this->_onPaletteChanged();                                                               \
                this->updateTransaction([this, color] {                                                  \
                        return new EditColorOperation(                                                   \
                                this->localize("color.edit"),                                            \
                                this->_palettes[this->_selectedPalette],                                 \
                                this->_selectedPalette,                                                  \
                                this->_selectedColor,                                                    \
                                colorConversions[this->_selectColorMethod](color[0], color[1], color[2]) \
                        );                                                                               \
                });                                                                                      \
        });                                                                                              \
        elem->onChangeEnded([this]{                                                                      \
                if (this->_palettes.empty())                                                             \
                        return;                                                                          \
                this->commitTransaction();                                                               \
        });                                                                                              \
        this->_containers.emplace(&container);                                                           \
} while (false)
#define PLACE_HOOK_HEXCOLOR(elem) do {                                                                  \
        if (!elem)                                                                                      \
                break;                                                                                  \
        elem->onFocus([this]{                                                                           \
                if (this->_palettes.empty())                                                            \
                        return;                                                                         \
	        this->startTransaction();                                                               \
	}); 	        	        	        	                                        \
        elem->onUnfocus([this]{                                                                         \
                if (this->_palettes.empty())                                                            \
                        return;                                                                         \
	        this->commitTransaction();                                                              \
	}); 	        	        	        	                                        \
        elem->onReturnKeyPress([this]{                                                                  \
                if (this->_palettes.empty())                                                            \
                        return;                                                                         \
	        this->commitTransaction();                                                              \
	        this->startTransaction();                                                               \
	}); 	        	        	        	                                        \
        elem->onTextChange([this](const tgui::String &s){                                               \
                if (this->_palettes.empty())                                                            \
                        return;                                                                         \
                                                                                                        \
                Color color{0, 0, 0, 255};                                                              \
                                                                                                        \
                if (s.size() < 7)                                                                       \
                        return;                                                                         \
                sscanf(s.toStdString().c_str(), "#%02hhX%02hhX%02hhX", &color.r, &color.g, &color.b);   \
                this->_colorChangeSource = 5;                                                           \
                this->updateTransaction([this, color]{                                                  \
                        return new EditColorOperation(                                                  \
                                this->localize("color.edit"),                                           \
                                this->_palettes[this->_selectedPalette],                                \
                                this->_selectedPalette,                                                 \
                                this->_selectedColor,                                                   \
                                color                                                                   \
                        );                                                                              \
                });                                                                                     \
        });                                                                                             \
        this->_containers.emplace(&container);                                                          \
} while (false)
#define PLACE_HOOK_COLOR_COMPONENT(elem, index) do {                                                                 \
        if (!elem)                                                                                                   \
                break;                                                                                               \
        elem->onFocus([this]{                                                                                        \
                if (this->_palettes.empty())                                                                         \
                        return;                                                                                      \
	        this->startTransaction();                                                                            \
	}); 	        	        	        	                                                     \
        elem->onUnfocus([this]{                                                                                      \
                if (this->_palettes.empty())                                                                         \
                        return;                                                                                      \
	        this->commitTransaction();                                                                           \
	}); 	        	        	        	                                                     \
        elem->onReturnKeyPress([this]{                                                                               \
                if (this->_palettes.empty())                                                                         \
                        return;                                                                                      \
	        this->commitTransaction();                                                                           \
	        this->startTransaction();                                                                            \
	}); 	        	        	        	                                                     \
        elem->onTextChange([this](const tgui::String &s){                                                            \
                if (this->_palettes.empty())                                                                         \
                        return;                                                                                      \
                if (s.empty())                                                                                       \
                        return;                                                                                      \
                                                                                                                     \
                auto value = std::stoul(s.toStdString());                                                            \
                auto color = this->_palettes[this->_selectedPalette].colors[this->_selectedColor];                   \
                auto converted = colorConversionsReverse[this->_selectColorMethod](color);                           \
                                                                                                                     \
                converted[index] = value;                                                                            \
                this->_colorChangeSource = index + 2;                                                                \
                this->updateTransaction([this, converted]{                                                           \
                        return new EditColorOperation(                                                               \
                                this->localize("color.edit"),                                                \
                                this->_palettes[this->_selectedPalette],                                             \
                                this->_selectedPalette,                                                              \
                                this->_selectedColor,                                                                \
                                colorConversions[this->_selectColorMethod](converted[0], converted[1], converted[2]) \
                        );                                                                                           \
                });                                                                                                  \
        });                                                                                                          \
        this->_containers.emplace(&container);                                                                       \
} while (false)
#define PLACE_HOOK_BOX(container, guiId, field, name)                                                     \
do {                                                                                                      \
        auto __elem = container.get<tgui::EditBox>(guiId);                                                \
                                                                                                          \
        if (!__elem)                                                                                      \
                break;                                                                                    \
        __elem->onFocus([this]{ this->startTransaction(); });                                             \
        __elem->onUnfocus([this]{ this->commitTransaction(); });                                          \
        __elem->onReturnKeyPress([this]{ this->commitTransaction(); this->startTransaction(); });         \
        __elem->onTextChange([this](const tgui::String &s){                                               \
                auto box = this->_preview->getSelectedBox();                                              \
                                                                                                          \
                if (box.first == BOXTYPE_NONE || s.empty()) return;                                       \
                try {                                                                                     \
                        ShadyCore::Schema::Sequence::BBox old = *this->_preview->getSelectedBoxRef();     \
                                                                                                          \
                        old.field = std::stoi(s.toStdString());                                           \
                        this->updateTransaction([&]{ return new EditBoxOperation(                         \
                                *this->_object,                                                           \
                                name,                                                                     \
                                box.first, box.second,                                                    \
                                old                                                                       \
                        ); });                                                                            \
                } catch (...) { return; }                                                                 \
        });                                                                                               \
        this->_updateFrameElements[&container].emplace_back([__elem, this]{                               \
                ShadyCore::Schema::Sequence::BBox *box = this->_preview->getSelectedBoxRef();             \
                                                                                                          \
                if (!box) return;                                                                         \
                __elem->onTextChange.setEnabled(false);                                                   \
                __elem->setText(std::to_string(box->field));                                              \
                __elem->onTextChange.setEnabled(true);                                                    \
        });                                                                                               \
        this->_containers.emplace(&container);                                                            \
} while (false)
#define PLACE_HOOK_STRUCTURE(container, guiId, field, name, operation, toString, fromStringPre, fromString, arg, reset, noEmpty) \
do {                                                                                              \
        auto __elem = container.get<tgui::EditBox>(guiId);                                        \
                                                                                                  \
        if (!__elem)                                                                              \
                break;                                                                            \
        __elem->onFocus([this]{ this->startTransaction(); });                                     \
        __elem->onUnfocus([this]{ this->commitTransaction(); });                                  \
        __elem->onReturnKeyPress([this]{ this->commitTransaction(); this->startTransaction(); }); \
        __elem->onTextChange([this](const tgui::String &s){                                       \
                if constexpr (noEmpty) if (s.empty()) return;                                     \
                try {                                                                             \
                        fromStringPre(s)                                                          \
                        this->updateTransaction([&]{ return new operation(                        \
                                *this->_object,                                                   \
                                name,                                                             \
                                &FrameData::field,                                                \
                                fromString(s, decltype(FrameData::field), arg),                   \
                                reset                                                             \
                        ); });                                                                    \
                } catch (...) { return; }                                                         \
        });                                                                                       \
        this->_updateFrameElements[&container].emplace_back([__elem, this]{                       \
                auto &data = this->_object->getFrameData();                                       \
                __elem->onTextChange.setEnabled(false);                                           \
                __elem->setText(toString(data.field, arg));                                       \
                __elem->onTextChange.setEnabled(true);                                            \
        });                                                                                       \
        this->_containers.emplace(&container);                                                    \
} while (false)
#define PLACE_HOOK_STRUCTURE_SEQ(container, guiId, field, name, toString, fromStringPre, fromString, arg, reset, noEmpty) \
do {                                                                                              \
        auto __elem = container.get<tgui::EditBox>(guiId);                                        \
                                                                                                  \
        if (!__elem)                                                                              \
                break;                                                                            \
        __elem->onFocus([this]{ this->startTransaction(); });                                     \
        __elem->onUnfocus([this]{ this->commitTransaction(); });                                  \
        __elem->onReturnKeyPress([this]{ this->commitTransaction(); this->startTransaction(); }); \
        __elem->onTextChange([this](const tgui::String &s){                                       \
                if constexpr (noEmpty) if (s.empty()) return;                                     \
                try {                                                                             \
                        fromStringPre(s)                                                          \
                        this->updateTransaction([&]{ return new BasicDataOperationSeq(            \
                                *this->_object,                                                   \
                                name,                                                             \
                                &FrameData::Sequence::field,                                      \
                                fromString(s, decltype(FrameData::Sequence::field), arg),         \
                                reset                                                             \
                        ); });                                                                    \
                } catch (...) { return; }                                                         \
        });                                                                                       \
        this->_updateFrameElements[&container].emplace_back([__elem, this]{                       \
                auto &data = this->_object->getSequence();                                        \
                __elem->onTextChange.setEnabled(false);                                           \
                __elem->setText(toString(data.field, arg));                                       \
                __elem->onTextChange.setEnabled(true);                                            \
        });                                                                                       \
        this->_containers.emplace(&container);                                                    \
} while (false)
#define PLACE_HOOK_STRUCTURE2(container, guiId, field, field2, name, toString, fromStringPre, fromString, arg, reset, noEmpty) \
do {                                                                                              \
        auto __elem = container.get<tgui::EditBox>(guiId);                                        \
                                                                                                  \
        if (!__elem)                                                                              \
                break;                                                                            \
        __elem->onFocus([this]{ this->startTransaction(); });                                     \
        __elem->onUnfocus([this]{ this->commitTransaction(); });                                  \
        __elem->onReturnKeyPress([this]{ this->commitTransaction(); this->startTransaction(); }); \
        __elem->onTextChange([this](const tgui::String &s){                                       \
                if constexpr (noEmpty) if (s.empty()) return;                                     \
                try {                                                                             \
                        fromStringPre(s)                                                          \
                        this->updateTransaction([&]{ return new BasicData2StepOperation(          \
                                *this->_object,                                                   \
                                name,                                                             \
                                &FrameData::field,                                                \
                                &decltype(FrameData::field)::field2,                              \
                                fromString(s, decltype(decltype(FrameData::field)::field2), arg), \
                                reset                                                             \
                        ); });                                                                    \
                } catch (...) { return; }                                                         \
        });                                                                                       \
        this->_updateFrameElements[&container].emplace_back([__elem, this]{                       \
                auto &data = this->_object->getFrameData();                                       \
                __elem->onTextChange.setEnabled(false);                                           \
                __elem->setText(toString(data.field.field2, arg));                                \
                __elem->onTextChange.setEnabled(true);                                            \
        });                                                                                       \
        this->_containers.emplace(&container);                                                    \
} while (false)
#define PLACE_HOOK_STRUCTURE_DOUBLE(container, guiId, field1, field2, name, toString, fromStringPre, fromString, arg, reset, noEmpty) \
do {                                                                                              \
        auto __elem = container.get<tgui::EditBox>(guiId);                                        \
                                                                                                  \
        if (!__elem)                                                                              \
                break;                                                                            \
        __elem->onFocus([this]{ this->startTransaction(); });                                     \
        __elem->onUnfocus([this]{ this->commitTransaction(); });                                  \
        __elem->onReturnKeyPress([this]{ this->commitTransaction(); this->startTransaction(); }); \
        __elem->onTextChange([this](const tgui::String &s){                                       \
                if constexpr (noEmpty) if (s.empty()) return;                                     \
                try {                                                                             \
                        fromStringPre(s)                                                          \
                        this->updateTransaction([&]{ return new BasicDataDoubleOperation(         \
                                *this->_object,                                                   \
                                name,                                                             \
                                &FrameData::field1,                                               \
                                &FrameData::field2,                                               \
                                fromString(s, decltype(FrameData::field2)),                       \
                                reset                                                             \
                        ); });                                                                    \
                } catch (...) { return; }                                                         \
        });                                                                                       \
        this->_updateFrameElements[&container].emplace_back([__elem, this]{                       \
                auto &data = this->_object->getFrameData();                                       \
                __elem->onTextChange.setEnabled(false);                                           \
                __elem->setText(toString(data.field1, data.field2, arg));                         \
                __elem->onTextChange.setEnabled(true);                                            \
        });                                                                                       \
        this->_containers.emplace(&container);                                                    \
} while (false)
#define PLACE_HOOK_STRUCTURE_DOUBLE2(container, guiId, field, field1, field2, name, toString, fromStringPre, fromString, arg, reset, noEmpty) \
do {                                                                                              \
        auto __elem = container.get<tgui::EditBox>(guiId);                                        \
                                                                                                  \
        if (!__elem)                                                                              \
                break;                                                                            \
        __elem->onFocus([this]{ this->startTransaction(); });                                     \
        __elem->onUnfocus([this]{ this->commitTransaction(); });                                  \
        __elem->onReturnKeyPress([this]{ this->commitTransaction(); this->startTransaction(); }); \
        __elem->onTextChange([this](const tgui::String &s){                                       \
                if constexpr (noEmpty) if (s.empty()) return;                                     \
                try {                                                                             \
                        fromStringPre(s)                                                          \
                        this->updateTransaction([&]{ return new BasicDataDouble2Operation(        \
                                *this->_object,                                                   \
                                name,                                                             \
                                &FrameData::field,                                                \
                                &decltype(FrameData::field)::field1,                              \
                                &decltype(FrameData::field)::field2,                              \
                                fromString(s, decltype(decltype(FrameData::field)::field2)),      \
                                reset                                                             \
                        ); });                                                                    \
                } catch (...) { return; }                                                         \
        });                                                                                       \
        this->_updateFrameElements[&container].emplace_back([__elem, this]{                       \
                auto &data = this->_object->getFrameData();                                       \
                __elem->onTextChange.setEnabled(false);                                           \
                __elem->setText(toString(data.field.field1, data.field.field2, arg));             \
                __elem->onTextChange.setEnabled(true);                                            \
        });                                                                                       \
        this->_containers.emplace(&container);                                                    \
} while (false)
#define PLACE_HOOK_STRUCTURE_QUAD(container, guiId, field1, field2, field3, field4, name, toString, fromStringPre, fromString, arg, reset, noEmpty) \
do {                                                                                                \
        auto __elem = container.get<tgui::EditBox>(guiId);                                          \
                                                                                                    \
        if (!__elem)                                                                                \
                break;                                                                              \
        __elem->onFocus([this]{ this->startTransaction(); });                                       \
        __elem->onUnfocus([this]{ this->commitTransaction(); });                                    \
        __elem->onReturnKeyPress([this]{ this->commitTransaction(); this->startTransaction(); });   \
        __elem->onTextChange([this](const tgui::String &s){                                         \
                if constexpr (noEmpty) if (s.empty()) return;                                       \
                try {                                                                               \
                        fromStringPre(s)                                                            \
                        this->updateTransaction([&]{ return new BasicDataQuadOperation(             \
                                *this->_object,                                                     \
                                name,                                                               \
                                &FrameData::field1,                                                 \
                                &FrameData::field2,                                                 \
                                &FrameData::field3,                                                 \
                                &FrameData::field4,                                                 \
                                fromString(s, decltype(FrameData::field1)),                         \
                                reset                                                               \
                        ); });                                                                      \
                } catch (...) { return; }                                                           \
        });                                                                                         \
        this->_updateFrameElements[&container].emplace_back([__elem, this]{                         \
                auto &data = this->_object->getFrameData();                                         \
                __elem->onTextChange.setEnabled(false);                                             \
                __elem->setText(toString(data.field1, data.field2, data.field3, data.field4, arg)); \
                __elem->onTextChange.setEnabled(true);                                              \
        });                                                                                         \
        this->_containers.emplace(&container);                                                      \
} while (false)

#define NO_FROMSTRING_PRE(s)

#define STRING_FROM_STRING(s, _, __) s.toStdString()
#define STRING_TO_STRING(s, _) s

#define HEXNUMBER_FROM_STRING(s, type, _) static_cast<type>(std::stoul(s.toStdString().substr(1), nullptr, 16))
#define HEXNUMBER_TO_STRING(s, p) ("#" + to_hex(s, p))

#define NUMBER_FROM_STRING(s, type, _) static_cast<type>(std::stof(s.toStdString()))
#define NUMBER_TO_STRING(s, p) to_string(s, p)

#define SOKU_FLOAT_FROM_STRING(s, type, p) static_cast<type>(std::stof(s.toStdString()) * std::pow(10, p / 10))
#define SOKU_FLOAT_TO_STRING(s, p) to_string(static_cast<float>(s / std::pow(10, p / 10)), p % 10)

#define SOKU_FLOAT_PERCENT_FROM_STRING(s, type, p) static_cast<type>(std::stof(s.toStdString()) * std::pow(10, p / 10))
#define SOKU_FLOAT_PERCENT_TO_STRING(s, p) (to_string(static_cast<float>(s / std::pow(10, p / 10)), p % 10) + "%")

#define NUMFLAGS_FROM_STRING(s, type, __) (type)std::stoul(s.toStdString(), nullptr, sizeof(type) * 2)
#define NUMFLAGS_TO_STRING(s, d) to_hex(s, d)

#define VECTOR_FROM_STRING_PRE(s)                                       \
        auto pos = s.find(',');                                         \
        auto __x = s.substr(1, pos - 1).toStdString();                  \
        auto __y = s.substr(pos + 1, s.size() - pos - 1).toStdString();
#define VECTOR_FROM_STRING(s, type) \
        (type)std::stof(__x),          \
        (type)std::stof(__y)
#define VECTOR_TO_STRING(s1, s2, p) ("(" + \
	to_string(s1, p) + "," +  \
	to_string(s2, p) +        \
")")

#define VECTOR_FLOAT_FROM_STRING(s, type)          \
        (type)(std::stof(__x) * 100), \
        (type)(std::stof(__y) * 100)
#define VECTOR_FLOAT_TO_STRING(s1, s2, p) ("(" + \
	to_string(s1 / 100.f, p) + "," +  \
	to_string(s2 / 100.f, p) +        \
")")

#define RECT_FROM_STRING_PRE(s)                                     \
        auto __pos = s.find(',');                                   \
        auto __x = s.substr(1, __pos - 1).toStdString();            \
	                                                            \
        auto __remainder = s.substr(__pos + 1);                     \
        auto __pos2 = __remainder.find(',');                        \
        auto __y = __remainder.substr(0, __pos2 + 1).toStdString(); \
	                                                            \
        auto __remainder2 = __remainder.substr(__pos2 + 1);         \
        auto __pos3 = __remainder2.find(',');                       \
        auto __w = __remainder2.substr(0, __pos3 + 1).toStdString();\
        auto __h = __remainder2.substr(__pos3 + 1, __remainder2.size() - __pos3 - 1).toStdString();
#define RECT_FROM_STRING(s, type) \
        (type)std::stof(__x),     \
        (type)std::stof(__y),     \
        (type)std::stof(__w),     \
        (type)std::stof(__h)

#define RECT_TO_STRING(s1, s2, s3, s4, p) ("(" +     \
        to_string(s1, p) + "," +  \
        to_string(s2, p) + "," +  \
        to_string(s3, p) + "," + \
        to_string(s4, p) +       \
")")

#define PLACE_HOOK_STRING(container, guiId, field, name, operation, reset) \
	PLACE_HOOK_STRUCTURE(container, guiId, field, name, operation, STRING_TO_STRING, NO_FROMSTRING_PRE, STRING_FROM_STRING, _, reset, false)
#define PLACE_HOOK_NUMBER(container, guiId, field, name, operation, reset, precision) \
	PLACE_HOOK_STRUCTURE(container, guiId, field, name, operation, NUMBER_TO_STRING, NO_FROMSTRING_PRE, NUMBER_FROM_STRING, precision, reset, true)
#define PLACE_HOOK_NUMFLAGS(container, guiId, field, name, operation, reset) \
	PLACE_HOOK_STRUCTURE(container, guiId, field, name, operation, NUMFLAGS_TO_STRING, NO_FROMSTRING_PRE, NUMFLAGS_FROM_STRING, _, reset, true)

#define PLACE_HOOK_HEXCOLOR_INT2(container, guiId, field, field2, name, digits) \
	PLACE_HOOK_STRUCTURE2(container, guiId, field, field2, name, HEXNUMBER_TO_STRING, NO_FROMSTRING_PRE, HEXNUMBER_FROM_STRING, digits, false, true)
#define PLACE_HOOK_NUMBER2(container, guiId, field, field2, name) \
	PLACE_HOOK_STRUCTURE2(container, guiId, field, field2, name, NUMBER_TO_STRING, NO_FROMSTRING_PRE, NUMBER_FROM_STRING, 0, false, true)
#define PLACE_HOOK_SOKU_FLOAT2(container, guiId, field, field2, name, precision) \
	PLACE_HOOK_STRUCTURE2(container, guiId, field, field2, name, SOKU_FLOAT_TO_STRING, NO_FROMSTRING_PRE, SOKU_FLOAT_FROM_STRING, precision, false, true)
#define PLACE_HOOK_SOKU_FLOAT_PERCENT2(container, guiId, field, field2, name, precision) \
	PLACE_HOOK_STRUCTURE2(container, guiId, field, field2, name, SOKU_FLOAT_PERCENT_TO_STRING, NO_FROMSTRING_PRE, SOKU_FLOAT_PERCENT_FROM_STRING, precision, false, true)
#define PLACE_HOOK_NUMFLAGS2(container, guiId, field, field2, name, digits) \
	PLACE_HOOK_STRUCTURE2(container, guiId, field, field2, name, NUMFLAGS_TO_STRING, NO_FROMSTRING_PRE, NUMFLAGS_FROM_STRING, digits, false, true)

#define PLACE_HOOK_VECTOR(container, guiId, field2, name) \
	PLACE_HOOK_STRUCTURE_DOUBLE(container, guiId, field2##X, field2##Y, name, VECTOR_TO_STRING, VECTOR_FROM_STRING_PRE, VECTOR_FROM_STRING, 0, false, true)
#define PLACE_HOOK_VECTOR_SOKU_FLOAT(container, guiId, field2, name, precision) \
	PLACE_HOOK_STRUCTURE_DOUBLE(container, guiId, field2##X, field2##Y, name, VECTOR_FLOAT_TO_STRING, VECTOR_FROM_STRING_PRE, VECTOR_FLOAT_FROM_STRING, precision, false, true)

#define PLACE_HOOK_VECTOR2(container, guiId, field, field2, name) \
	PLACE_HOOK_STRUCTURE_DOUBLE2(container, guiId, field, field2##X, field2##Y, name, VECTOR_TO_STRING, VECTOR_FROM_STRING_PRE, VECTOR_FROM_STRING, 0, false, true)
#define PLACE_HOOK_VECTOR_SOKU_FLOAT2(container, guiId, field, field2, name, precision) \
	PLACE_HOOK_STRUCTURE_DOUBLE2(container, guiId, field, field2##X, field2##Y, name, VECTOR_FLOAT_TO_STRING, VECTOR_FROM_STRING_PRE, VECTOR_FLOAT_FROM_STRING, precision, false, true)

#define PLACE_HOOK_RECT(container, guiId, field, name) \
	PLACE_HOOK_STRUCTURE_QUAD(container, guiId, field##OffsetX, field##OffsetY, field##Width, field##Height, name, RECT_TO_STRING, RECT_FROM_STRING_PRE, RECT_FROM_STRING, 0, false, true)

#define PLACE_HOOK_FLAG(container, guiId, field, index, name, reset)               \
do {                                                                               \
        auto __elem = container.get<tgui::CheckBox>(guiId);                        \
                                                                                   \
        if (!__elem)                                                               \
                break;                                                             \
        __elem->onChange([this, index](bool b){                                    \
                this->applyOperation(new FlagOperation(                            \
                        *this->_object,                                            \
                        name,                                                      \
                        &ShadyCore::Schema::Sequence::MoveTraits::field,           \
                        index, b, (reset)                                          \
                ));                                                                \
                this->_rePopulateFrameData();                                      \
        });                                                                        \
        this->_updateFrameElements[&container].emplace_back([__elem, this, index]{ \
                auto &data = this->_object->getFrameData();                        \
                __elem->onChange.setEnabled(false);                                \
                __elem->setChecked(data.traits.field & (1ULL << index));           \
                __elem->onChange.setEnabled(true);                                 \
        });                                                                        \
        this->_containers.emplace(&container);                                     \
} while (false)

#define PLACE_HOOK_NUMBER_SEQ(container, guiId, field, name, reset, precision) \
	PLACE_HOOK_STRUCTURE_SEQ(container, guiId, field, name, NUMBER_TO_STRING, NO_FROMSTRING_PRE, NUMBER_FROM_STRING, precision, reset, true)
#define PLACE_HOOK_BOOL_SEQ(container, guiId, field, name, reset)                  \
do {                                                                               \
        auto __elem = container.get<tgui::CheckBox>(guiId);                        \
                                                                                   \
        if (!__elem)                                                               \
                break;                                                             \
        __elem->onChange([this](bool b){                                           \
                this->updateTransaction([&]{ return new BasicDataOperationSeq(     \
                        *this->_object,                                            \
                        name,                                                      \
                        &FrameData::Sequence::field,                               \
                        b, reset                                                   \
                ); });                                                             \
                this->_rePopulateFrameData();                                      \
        });                                                                        \
        this->_updateFrameElements[&container].emplace_back([__elem, this]{        \
                auto &data = this->_object->getSequence();                         \
                __elem->onChange.setEnabled(false);                                \
                __elem->setChecked(data.field);                                    \
                __elem->onChange.setEnabled(true);                                 \
        });                                                                        \
        this->_containers.emplace(&container);                                     \
} while (false)


TGUI_RENDERER_PROPERTY_COLOR(SpiralOfFate::MainWindow::Renderer, TitleColorFocused, tgui::Color::Black)
TGUI_RENDERER_PROPERTY_COLOR(SpiralOfFate::MainWindow::Renderer, TitleBarColorFocused, tgui::Color::White)
TGUI_RENDERER_PROPERTY_TEXTURE(SpiralOfFate::MainWindow::Renderer, TextureTitleBarFocused)
TGUI_RENDERER_PROPERTY_RENDERER(SpiralOfFate::MainWindow::Renderer, CloseButtonFocused, "ChildWindowButton")
TGUI_RENDERER_PROPERTY_RENDERER(SpiralOfFate::MainWindow::Renderer, MaximizeButtonFocused, "ChildWindowButton")
TGUI_RENDERER_PROPERTY_RENDERER(SpiralOfFate::MainWindow::Renderer, MinimizeButtonFocused, "ChildWindowButton")

bool SpiralOfFate::MainWindow::_loadLabelFor(std::map<unsigned, std::string> &labels, const std::string &name)
{
	std::ifstream stream;

	stream.open("assets/gui/editor/locale/" + this->_editor.getLocale() + "/" + name + ".json");
	if (!stream)
		stream.open("assets/gui/editor/locale/en/" + name + ".json");
	if (!stream)
		return false;

	nlohmann::json j;

	stream >> j;
	for (auto &[key, value] : j.items()) {
		if (value.is_null())
			labels.erase(std::stoul(key));
		else
			labels[std::stoul(key)] = value;
	}
	return true;
}

bool SpiralOfFate::MainWindow::_loadLabelFor(std::map<unsigned, std::string> &labels, const std::string &name, const std::string &folder)
{
	auto entry = game->package.find(folder + name + "_" + this->_editor.getLocale() + "_labels.json");

	if (entry == game->package.end())
		entry = game->package.find(folder + name + "_" + this->_editor.getLocale() + "_labels.txt");
	if (entry == game->package.end())
		entry = game->package.find(folder + name + "_" + this->_editor.getLocale() + "_labels.cv0");
	if (entry == game->package.end())
		entry = game->package.find(folder + name + "_" + this->_editor.getLocale() + "_labels");
	if (entry == game->package.end())
		entry = game->package.find(folder + name + "_labels.json");
	if (entry == game->package.end())
		entry = game->package.find(folder + name + "_labels.txt");
	if (entry == game->package.end())
		entry = game->package.find(folder + name + "_labels.cv0");
	if (entry == game->package.end())
		entry = game->package.find(folder + name + "_labels");
	if (entry == game->package.end())
		return false;

	auto &stream = entry->second->open();
	nlohmann::json j;

	stream >> j;
	entry->second->close(stream);
	for (auto &[key, value] : j.items()) {
		if (value.is_null())
			labels.erase(std::stoul(key));
		else
			labels[std::stoul(key)] = value;
	}
	return true;
}

bool SpiralOfFate::MainWindow::_loadLabelFor(std::map<unsigned, std::string> &labels, const std::string &name, const std::filesystem::path &folder)
{
	std::ifstream stream;

	stream.open(folder / (name + "_" + this->_editor.getLocale() + ".json"));
	if (!stream)
		stream.open(folder / (name + "_labels.json"));
	if (!stream)
		return false;

	nlohmann::json j;

	stream >> j;
	for (auto &[key, value] : j.items()) {
		if (value.is_null())
			labels.erase(std::stoul(key));
		else
			labels[std::stoul(key)] = value;
	}
	return true;
}

void SpiralOfFate::MainWindow::_onColorHover(int oColor, int nColor, bool shouldPause)
{
	if (oColor == nColor)
		return;
	for (auto container : this->_containers) {
		auto colorPanel = container->get<tgui::ScrollablePanel>("AllColorPanel");

		if (!colorPanel)
			continue;

		if (oColor >= 0) {
			auto button = colorPanel->get<tgui::Button>("Color" + std::to_string(oColor));
			auto render = button->getRenderer();

			if (this->_selectedColor == static_cast<unsigned>(oColor)) {
				render->setBorders({2, 2, 2, 2});
				render->setBorderColor(tgui::Color{0, 0, 255});
				render->setBorderColorHover(tgui::Color{100, 100, 255});
				render->setBorderColorDisabled(tgui::Color{200, 200, 255});
				render->setBorderColorFocused(tgui::Color{0, 0, 255});
				render->setBorderColorDown(tgui::Color{0, 0, 255});
				render->setBorderColorDownDisabled(tgui::Color{0, 0, 255});
				render->setBorderColorDownFocused(tgui::Color{0, 0, 255});
				render->setBorderColorDownHover(tgui::Color{0, 0, 255});
			} else {
				render->setBorders({1, 1, 1, 1});
				render->setBorderColor(tgui::Color{0, 0, 0});
				render->setBorderColorHover(tgui::Color{100, 100, 100});
				render->setBorderColorDisabled(tgui::Color{200, 200, 200});
				render->setBorderColorFocused(tgui::Color{0, 0, 255});
				render->setBorderColorDown(tgui::Color{0, 0, 0});
				render->setBorderColorDownDisabled(tgui::Color{0, 0, 0});
				render->setBorderColorDownFocused(tgui::Color{0, 0, 0});
				render->setBorderColorDownHover(tgui::Color{0, 0, 0});
			}
		}
		if (nColor >= 0) {
			auto button = colorPanel->get<tgui::Button>("Color" + std::to_string(nColor));
			auto render = button->getRenderer();

			if (this->_selectedColor == static_cast<unsigned>(nColor)) {
				render->setBorders({2, 2, 2, 2});
				render->setBorderColor(tgui::Color{0, 0, 255});
				render->setBorderColorHover(tgui::Color{100, 100, 255});
				render->setBorderColorDisabled(tgui::Color{200, 200, 255});
				render->setBorderColorFocused(tgui::Color{0, 0, 255});
				render->setBorderColorDown(tgui::Color{0, 0, 255});
				render->setBorderColorDownDisabled(tgui::Color{0, 0, 255});
				render->setBorderColorDownFocused(tgui::Color{0, 0, 255});
				render->setBorderColorDownHover(tgui::Color{0, 0, 255});
			} else {
				render->setBorders({2, 2, 2, 2});
				render->setBorderColor(tgui::Color{255, 128, 0});
				render->setBorderColorHover(tgui::Color{255, 178, 100});
				render->setBorderColorDisabled(tgui::Color{255, 228, 200});
				render->setBorderColorFocused(tgui::Color{255, 128, 0});
				render->setBorderColorDown(tgui::Color{255, 128, 0});
				render->setBorderColorDownDisabled(tgui::Color{255, 128, 0});
				render->setBorderColorDownFocused(tgui::Color{255, 128, 0});
				render->setBorderColorDownHover(tgui::Color{255, 128, 0});
			}
		}
	}
	this->_paused2 = nColor >= 0 && shouldPause;
}

void SpiralOfFate::MainWindow::_init()
{
	this->_object->_onHoverChange = [this] (int o, int n){
		this->_onColorHover(o, n, true);
	};
	if (this->_object->_schema.isCharacterData)
		this->_effectObject = std::make_unique<EditableObject>("data/effect/", std::string("data/effect/effect.xml"), &this->_palettes[this->_selectedPalette].colors);
	this->_preview = std::make_shared<PreviewWidget>(std::ref(this->_editor), std::ref(*this), *this->_object);
	this->_preview->setPosition(0, 0);
	this->_preview->setSize("&.w", "&.h");
	this->_preview->onBoxSelect([this]{
		this->_editor.setCanDelBoxes(true);
		this->_rePopulateFrameData();
	});
	this->_preview->onBoxUnselect([this]{
		this->_editor.setCanDelBoxes(false);
		this->_rePopulateFrameData();
	});
	this->_preview->setPalette(&this->_palettes[this->_selectedPalette].colors);
	this->_pathBak = game->data / "backups" / (std::to_string(++game->lastSwap) + ".bak");

	std::filesystem::create_directories(game->data / "backups");
	this->m_renderer = aurora::makeCopied<Renderer>();
	this->loadLocalizedWidgetsFromFile("assets/gui/editor/animationWindow.gui");

	auto panel = this->get<tgui::Panel>("AnimationPanel");
	auto showBoxes = panel->get<tgui::BitmapButton>("ShowBoxes");
	auto ctrlPanel = this->get<tgui::ScrollablePanel>("ControlPanel");

	ctrlPanel->getHorizontalScrollbar()->setPolicy(tgui::Scrollbar::Policy::Never);
	if (this->_object->_schema.isCharacterData)
		ctrlPanel->loadWidgetsFromFile("assets/gui/editor/character/framedata.gui");
	else
		ctrlPanel->loadWidgetsFromFile("assets/gui/editor/character/animation.gui");
	this->_localizeWidgets(*ctrlPanel, true);
	Utils::setRenderer(this);

	this->setSize("min(1200, &.w - 20)", "min(715, &.h - 40)");
	this->setPosition(10, 30);
	this->setTitleButtons(TitleButton::Minimize | TitleButton::Maximize | TitleButton::Close);
	this->setTitle(this->_title);
	this->setResizable();
	this->setCloseBehavior(CloseBehavior::None);
	this->onClose([this]{
		std::filesystem::remove(this->_pathBak);
		this->m_parent->remove(this->shared_from_this());
		this->onRealClose.emit(this);
	});

	panel->add(this->_preview);
	this->_preview->moveToBack();

	showBoxes->onClick([this](std::weak_ptr<tgui::BitmapButton> This){
		this->_preview->displayBoxes = !this->_preview->displayBoxes;
		This.lock()->setImage(tgui::Texture("assets/gui/editor/" + std::string(this->_preview->displayBoxes ? "" : "no") + "boxes.png"));
	}, std::weak_ptr(showBoxes));

	this->onFocus([this]{
		this->m_titleText.setColor(this->_titleColorFocusedCached);
		this->m_titleBarColorCached = this->_titleBarColorFocusedCached;
		this->_updateTextureTitleBar();
		this->_updateTitleButtons();
	});
	this->onUnfocus([this]{
		this->m_titleText.setColor(this->_titleColorUnfocusedCached);
		this->m_titleBarColorCached = this->_titleBarColorUnfocusedCached;
		this->_updateTextureTitleBar();
		this->_updateTitleButtons();
	});

	this->reloadLabels();
	this->_placeUIHooks(*this);
	this->_populateData(*this);
	this->_autoSaveThread = std::thread{&MainWindow::_autoSaveLoop, this};
}

SpiralOfFate::MainWindow::MainWindow(const nlohmann::json &json, FrameDataEditor &editor) :
	LocalizedContainer<tgui::ChildWindow>(editor, MainWindow::StaticWidgetType, false),
	_editor(editor),
	_modifications(json["modifications"]),
	_title(json["title"]),
	_chrPath(json["folder"]),
	_character(json["character"]),
	_fileName(json["fileName"]),
	_path(json["path"].get<std::filesystem::path::string_type>())
{
	if (!std::filesystem::exists(game->settings.palettes / this->_character))
		std::filesystem::create_directories(game->settings.palettes / this->_character);
	this->_object = std::make_unique<EditableObject>();
	if (this->_modifications != 0)
		this->_modifications = 1;

	auto &palettes = json["palettes"];
	auto &framedata = json["framedata"];
	std::map<unsigned, unsigned> clones;

	for (auto &palj : palettes) {
		this->_palettes.emplace_back();
		auto &pal = this->_palettes.back();
		auto &colors = palj["colors"];

		for (size_t i = 0; i < pal.colors.size(); i++)
			pal.colors[i] = { colors[i][0], colors[i][1], colors[i][2] };
		pal.colors[0].a = 0;
		pal.path = palj["path"].get<std::filesystem::path::string_type>();
		pal.modifications = palj["modifications"];
		pal.name = palj["name"].get<std::string>();
	}
	for (size_t i = 0; i < this->_palettes.size(); i++)
		if (this->_palettes[i].name == "[P]palette000.pal") {
			this->_selectedPalette = i;
			break;
		}

	this->_object->_schema.isCharacterData = json["isCharacterData"];
	for (const auto &[key, actionj] : framedata.items()) {
		auto id = std::stoul(key);
		auto &action = this->_object->_schema.framedata[id];

		if (actionj.is_number()) {
			clones[id] = actionj;
			action.clonedId = actionj;
			continue;
		}
		for (auto &seqj : actionj) {
			action._sequences.emplace_back();
			auto &sequence = action._sequences.back();
			auto &frames = seqj["frames"];

			sequence.loop = seqj["loop"];
			sequence.moveLock = seqj["moveLock"];
			sequence.actionLock = seqj["actionLock"];
			for (auto &frame : frames)
				sequence.data.emplace_back(frame, this->_chrPath, &this->_palettes[this->_selectedPalette].colors);
		}
	}
	for (auto &[id, clone] : clones)
		this->_object->_schema.framedata[id].cloned = &this->_object->_schema.framedata[clone];
	this->_init();
}

SpiralOfFate::MainWindow::MainWindow(const std::string &folder, const std::string &frameDataPath, FrameDataEditor &editor) :
	LocalizedContainer<tgui::ChildWindow>(editor, MainWindow::StaticWidgetType, false),
	_editor(editor),
	_title("[P]" + frameDataPath),
	_chrPath(folder),
	_fileName(frameDataPath)
{
	if (!this->_chrPath.ends_with('/'))
		this->_chrPath.push_back('/');
	this->_character = folder;
	while (this->_character.back() == '/')
		this->_character.pop_back();
	this->_character.erase(0, this->_character.find_last_of('/') + 1);

	auto pos1 = this->_fileName.find_last_of('.');
	auto pos2 = this->_fileName.find_last_of('/');
	auto palFolder = game->settings.palettes / this->_character;

	this->_fileName = this->_fileName.substr(pos2 + 1, pos1 - pos2 - 1);
	if (!std::filesystem::exists(palFolder))
		std::filesystem::create_directories(palFolder);

	auto pos = frameDataPath.find_last_of('.');
	std::string palPackage = this->_chrPath;

	if (frameDataPath.substr(0, pos).ends_with("/stand"))
		palPackage = "data/character/" + this->_character + "/";
	this->reloadPalette(palPackage);
	this->_object = std::make_unique<EditableObject>(this->_chrPath, frameDataPath, &this->_palettes[this->_selectedPalette].colors);
	this->_init();
}

SpiralOfFate::MainWindow::MainWindow(const std::string &folder, const std::filesystem::path &frameDataPath, FrameDataEditor &editor) :
	LocalizedContainer<tgui::ChildWindow>(editor, MainWindow::StaticWidgetType, false),
	_editor(editor),
	_pathInit(true),
	_title(frameDataPath.string()),
	_chrPath(folder),
	_path(frameDataPath)
{
	if (!this->_chrPath.ends_with('/'))
		this->_chrPath.push_back('/');
	this->_character = folder;
	while (this->_character.back() == '/')
		this->_character.pop_back();
	this->_character.erase(0, this->_character.find_last_of('/') + 1);

	auto palFolder = game->settings.palettes / this->_character;

	if (!std::filesystem::exists(palFolder))
		std::filesystem::create_directories(palFolder);

	std::string palPackage = this->_chrPath;

	if (frameDataPath.filename().stem().string() == "stand")
		palPackage = "data/character/" + this->_character + "/";
	this->reloadPalette(palPackage);
	this->_object = std::make_unique<EditableObject>(this->_chrPath, frameDataPath, &this->_palettes[this->_selectedPalette].colors);
	this->_init();
}

SpiralOfFate::MainWindow::~MainWindow()
{
	this->_stopped = true;
	if (this->_autoSaveThread.joinable())
		this->_autoSaveThread.join();
}

bool SpiralOfFate::MainWindow::isFramedataModified() const noexcept
{
	return this->_modifications != 0;
}

bool SpiralOfFate::MainWindow::arePalettesModified() const noexcept
{
	return std::ranges::any_of(this->_palettes, [](const Palette &p) { return p.modifications != 0; });
}

bool SpiralOfFate::MainWindow::hasUndoData() const noexcept
{
	return this->_operationIndex || (this->_pendingTransaction && this->_pendingTransaction->hasModification());
}

bool SpiralOfFate::MainWindow::hasRedoData() const noexcept
{
	return this->_operationQueue.size() != this->_operationIndex;
}

void SpiralOfFate::MainWindow::refreshMenuItems() const
{
	auto curr = this->_object->_schema.framedata.find(this->_object->_action);
	assert_exp(curr != this->_object->_schema.framedata.end());

	auto &mov = this->_object->_schema.framedata;
	auto &act = curr->second;
	auto &blk = act[this->_object->_actionBlock];

	this->_editor.setHasRedo(this->hasRedoData());
	this->_editor.setHasUndo(this->hasUndoData());
	this->_editor.setCanDelBoxes(this->_preview->getSelectedBox().first != BOXTYPE_NONE);
	this->_editor.setCanDelAction(mov.size() > 1);
	this->_editor.setCanDelBlock(act.size() > 1);
	this->_editor.setCanDelFrame(blk.size() > 1);
	this->_editor.setCanCopyLast(this->_object->_animation > 0);
	this->_editor.setCanCopyNext(this->_object->_animation < blk.size() - 1);
	this->_editor.setHasLastFrame(this->_object->_animation > 0);
	this->_editor.setHasNextFrame(this->_object->_animation < blk.size() - 1);
	this->_editor.setHasLastBlock(this->_object->_actionBlock > 0);
	this->_editor.setHasNextBlock(this->_object->_actionBlock < act.size() - 1);
	this->_editor.setHasLastAction(curr != this->_object->_schema.framedata.begin());
	this->_editor.setHasNextAction(std::next(curr) != this->_object->_schema.framedata.end());
}

void SpiralOfFate::MainWindow::redo()
{
	assert_exp(this->_pendingTransaction || this->_operationIndex < this->_operationQueue.size());
	if (this->_pendingTransaction) {
		this->commitTransaction();
		return;
	}

	std::string title = this->_title;
	std::lock_guard guard{this->_saveMutex};

	this->_operationQueue[this->_operationIndex]->apply();
	if (this->_operationQueue[this->_operationIndex]->hasFramedataModification())
		this->_modifications++;
	else {
		this->_preview->invalidatePalette();
		this->_requireReloadPal = true;
	}
	this->_operationIndex++;
	this->_requireReload = true;
	if (this->isFramedataModified())
		title.push_back('*');
	title.push_back('~');
	this->setTitle(title);
	this->_editor.setHasUndo(true);
	this->_editor.setHasRedo(this->hasRedoData());
	this->_requireAutoSave = true;
}

void SpiralOfFate::MainWindow::undo()
{
	assert_exp(this->_pendingTransaction || this->_operationIndex > 0);
	if (this->_pendingTransaction) {
		this->cancelTransaction();
		return;
	}

	std::lock_guard guard{this->_saveMutex};
	std::string title = this->_title;

	this->_operationIndex--;
	this->_operationQueue[this->_operationIndex]->undo();
	if (this->_operationQueue[this->_operationIndex]->hasFramedataModification())
		this->_modifications--;
	else {
		this->_preview->invalidatePalette();
		this->_requireReloadPal = true;
	}
	this->_requireReload = true;
	if (this->isFramedataModified())
		title.push_back('*');
	title.push_back('~');
	this->setTitle(title);
	this->_editor.setHasUndo(this->hasUndoData());
	this->_editor.setHasRedo(true);
	this->_requireAutoSave = true;
}

void SpiralOfFate::MainWindow::copyFrame()
{
	sf::Clipboard::setString(this->_object->getFrameData().toJson().dump(4));
}

void SpiralOfFate::MainWindow::pasteFrame()
{
	auto &data = this->_object->getFrameData();

	try {
		this->applyOperation(new PasteDataOperation(
			*this->_object,
			this->localize("operation.paste"),
			{
				nlohmann::json::parse(sf::Clipboard::getString().toAnsiString()),
				data.__folder,
				data.__paletteData
			}
		));
	} catch (std::exception &e) {
		game->logger.error("Failed to deserialize frame" + std::string(e.what()));
	}
}

void SpiralOfFate::MainWindow::pasteBoxData()
{
	auto &data = this->_object->getFrameData();

	try {
		this->applyOperation(new PasteBoxDataOperation(
			*this->_object,
			this->localize("operation.paste_box"),
			{
				nlohmann::json::parse(sf::Clipboard::getString().toAnsiString()),
				data.__folder,
				data.__paletteData
			}
		));
	} catch (std::exception &e) {
		game->logger.error("Failed to deserialize frame" + std::string(e.what()));
	}
}

void SpiralOfFate::MainWindow::pasteAnimData()
{
	auto &data = this->_object->getFrameData();

	try {
		this->applyOperation(new PasteAnimDataOperation(
			*this->_object,
			this->localize("operation.paste_anim"),
			{
				nlohmann::json::parse(sf::Clipboard::getString().toAnsiString()),
				data.__folder,
				data.__paletteData
			}
		));
	} catch (std::exception &e) {
		game->logger.error("Failed to deserialize frame" + std::string(e.what()));
	}
}

void SpiralOfFate::MainWindow::startTransaction(SpiralOfFate::Operation *operation)
{
	assert_exp(!this->_pendingTransaction);

	std::lock_guard guard{this->_saveMutex};
	std::string title = this->_title;

	if (!operation)
		operation = new DummyOperation();
	this->_pendingTransaction.reset(operation);
	this->_pendingTransaction->apply();
	if (this->_pendingTransaction->hasFramedataModification()) {
		this->_preview->invalidatePalette();
		this->_requireReloadPal = true;
	}
	if (this->isFramedataModified())
		title.push_back('*');
	if (this->_requireAutoSave)
		title.push_back('~');
	this->setTitle(title);
	this->_editor.setHasUndo(this->hasUndoData());
}

void SpiralOfFate::MainWindow::updateTransaction(const std::function<Operation *()> &operation)
{
	assert_exp(this->_pendingTransaction);

	std::lock_guard guard{this->_saveMutex};
	std::string title = this->_title;

	this->_pendingTransaction->undo();
	this->_pendingTransaction.reset(operation());
	this->_pendingTransaction->apply();
	if (!this->_pendingTransaction->hasFramedataModification()) {
		this->_preview->invalidatePalette();
		this->_requireReloadPal = true;
	}
	if (this->isFramedataModified())
		title.push_back('*');
	if (this->_requireAutoSave)
		title.push_back('~');
	this->setTitle(title);
	this->_editor.setHasUndo(this->hasUndoData());
}

void SpiralOfFate::MainWindow::cancelTransaction()
{
	std::lock_guard guard{this->_saveMutex};
	std::string title = this->_title;

	assert_exp(this->_pendingTransaction);
	this->_pendingTransaction->undo();
	if (!this->_pendingTransaction->hasFramedataModification()) {
		this->_preview->invalidatePalette();
		this->_requireReloadPal = true;
	}
	this->_pendingTransaction.reset();
	if (this->isFramedataModified())
		title.push_back('*');
	title.push_back('~');
	this->setTitle(title);
	this->_editor.setHasUndo(this->hasUndoData());
}

void SpiralOfFate::MainWindow::commitTransaction()
{
	assert_exp(this->_pendingTransaction);
	if (!this->_pendingTransaction->hasModification())
		return this->_pendingTransaction.reset();

	std::lock_guard guard{this->_saveMutex};
	std::string title = this->_title;

	if (this->_pendingTransaction->hasFramedataModification())
		this->_modifications++;
	else {
		this->_preview->invalidatePalette();
		this->_requireReloadPal = true;
	}
	this->_operationQueue.erase(this->_operationQueue.begin() + this->_operationIndex, this->_operationQueue.end());
	this->_operationQueue.emplace_back(nullptr);
	this->_operationQueue.back().swap(this->_pendingTransaction);
	this->_requireReload = true;
	if (this->_operationIndex < this->_operationSaved)
		this->_operationSaved = -1;
	this->_operationIndex = this->_operationQueue.size();

	if (this->isFramedataModified())
		title.push_back('*');
	title.push_back('~');
	this->setTitle(title);
	this->_editor.setHasUndo(true);
	this->_editor.setHasRedo(false);
	this->_requireAutoSave = true;
}

void SpiralOfFate::MainWindow::applyOperation(Operation *operation)
{
	if (!operation->hasModification()) {
		delete operation;
		return;
	}

	std::lock_guard guard{this->_saveMutex};
	std::string title = this->_title;

	if (operation->hasFramedataModification())
		this->_modifications++;
	else {
		this->_preview->invalidatePalette();
		this->_requireReloadPal = true;
	}
	this->_operationQueue.erase(this->_operationQueue.begin() + this->_operationIndex, this->_operationQueue.end());
	this->_operationQueue.emplace_back(operation);
	this->_operationQueue.back()->apply();
	this->_requireReload = true;
	if (this->_operationIndex < this->_operationSaved)
		this->_operationSaved = -1;
	this->_operationIndex = this->_operationQueue.size();

	if (this->isFramedataModified())
		title.push_back('*');
	title.push_back('~');
	this->setTitle(title);
	this->_editor.setHasUndo(true);
	this->_editor.setHasRedo(false);
	this->_requireAutoSave = true;
}

bool SpiralOfFate::MainWindow::save(const std::filesystem::path &path)
{
	this->setPath(path);
	return this->save();
}

void SpiralOfFate::MainWindow::setPath(const std::filesystem::path &path)
{
	this->_path = path;
}

void SpiralOfFate::MainWindow::setBakPath(const std::filesystem::path &path)
{
	this->_pathBak = path;
}

bool SpiralOfFate::MainWindow::save()
{
	if (this->_pendingTransaction)
		this->commitTransaction();

	if (this->_pathInit && this->_path.extension() == ".json") {
		auto j = this->_asJson();
		std::ofstream stream{this->_path};

		if (stream.fail()) {
			auto err = this->localize("error.open", this->_path.string(), strerror(errno));

			// FIXME: strerror only works on Linux (err.message()?)
			// TODO: Somehow return the message box so the caller can open the save as dialog when OK is clicked
			Utils::dispMsg(game->gui, this->localize("message_box.title.save_err"), err, MB_ICONERROR);
			game->logger.error(err);
			return false;
		}
		stream << j;
		stream.close();
		this->_operationSaved = this->_operationIndex;
		this->autoSave();
		this->_modifications = 0;
		for (auto &pal : this->_palettes)
			pal.modifications = 0;
		this->_requireReloadPal = true;
		return true;
	}

	bool result = true;

	if (this->isFramedataModified()) {
		ShadyCore::FileType::Format format;
		ShadyCore::Schema schema;

		for (auto &action : this->_object->_schema.framedata) {
			if (action.second.cloned) {
				auto c = new ShadyCore::Schema::Clone(action.first);

				c->targetId = action.second.clonedId;
				schema.objects.push_back(c);
				continue;
			}
			for (auto &sequence : action.second) {
				auto c = new ShadyCore::Schema::Sequence(action.first, !this->_object->_schema.isCharacterData);

				c->loop = sequence.loop;
				c->moveLock = sequence.moveLock;
				c->actionLock = sequence.actionLock;
				for (auto &frame : sequence) {
					frame.imageIndex = getOrCreateImage(schema, frame.spritePath);
					c->frames.push_back(new ShadyCore::Schema::Sequence::MoveFrame(frame));
				}
				schema.objects.push_back(c);
			}
		}

		std::ifstream::openmode mode = std::ifstream::in;

		if (this->_path.extension() == ".xml")
			format = ShadyCore::FileType::SCHEMA_XML;
		else if (this->_object->_schema.isCharacterData) {
			format = ShadyCore::FileType::SCHEMA_GAME_PATTERN;
			mode |= std::ifstream::binary;
		} else {
			format = ShadyCore::FileType::SCHEMA_GAME_ANIM;
			mode |= std::ifstream::binary;
		}

		std::ofstream stream{this->_path, mode};

		if (stream.fail()) {
			auto err = this->localize("error.open", this->_path.string(), strerror(errno));

			// FIXME: strerror only works on Linux (err.message()?)
			// TODO: Somehow return the message box so the caller can open the save as dialog when OK is clicked
			Utils::dispMsg(game->gui, this->localize("message_box.title.save_err"), err, MB_ICONERROR);
			game->logger.error(err);
			return false;
		}
		ShadyCore::getResourceWriter({ShadyCore::FileType::TYPE_SCHEMA, format})(&schema, stream);
		stream.close();
		schema.destroy();
	}

	for (auto &pal : this->_palettes) {
		if (pal.modifications == 0)
			continue;
		try {
			// For this one since we are saving let's keep going anyway and display the error.
			assert_exp(!pal.path.empty());
		} catch (std::exception &e) {
			auto err = this->localize("error.open", pal.path.string(), e.what());

			Utils::dispMsg(game->gui, this->localize("message_box.title.save_err"), err, MB_ICONERROR);
			game->logger.error(err);
			result = false;
			continue;
		}

		ShadyCore::Palette palette;
		auto ptr = new uint32_t[256];

		palette.bitsPerPixel = 32;
		palette.data = reinterpret_cast<uint8_t *>(ptr);
		for (size_t i = 0; i < pal.colors.size(); i++)
			ptr[i] =
				(i == 0 ? 0x00000000 : 0xFF000000) |
				pal.colors[i].r << 16 |
				pal.colors[i].g << 8 |
				pal.colors[i].b << 0;
		palette.pack();

		std::ofstream pstream{pal.path, std::ios::binary};
		ShadyCore::FileType::Format format;

		if (pstream.fail()) {
			auto err = this->localize("error.open", pal.path.string(), strerror(errno));

			// FIXME: strerror only works on Linux (err.message()?)
			Utils::dispMsg(game->gui, this->localize("message_box.title.save_pal_err"), err, MB_ICONERROR);
			game->logger.error(err);
			continue;
		}
		if (pal.path.extension() == ".pal")
			format = ShadyCore::FileType::PALETTE_PAL;
		else if (pal.path.extension() == ".act")
			format = ShadyCore::FileType::PALETTE_ACT;
		else
			continue;
		ShadyCore::getResourceWriter({ShadyCore::FileType::TYPE_PALETTE, format})(&palette, pstream);
		pstream.close();
		palette.destroy();
		pal.modifications = 0;
	}

	this->_modifications = 0;
	this->_operationSaved = this->_operationIndex;
	this->autoSave();
	this->_requireReloadPal = true;
	return result;
}

void SpiralOfFate::MainWindow::exportPalette(const std::filesystem::path &path)
{
	auto &pal = this->_palettes[this->_selectedPalette];
	ShadyCore::Palette palette;
	ShadyCore::FileType::Format format;

	if (path.extension() == ".act")
		format = ShadyCore::FileType::PALETTE_ACT;
	else
		format = ShadyCore::FileType::PALETTE_PAL;
	palette.initialize(32);
	for (size_t i = 0; i < pal.colors.size(); i++)
		reinterpret_cast<uint32_t *>(palette.data)[i] =
			(i == 0 ? 0x00000000 : 0xFF000000) |
			pal.colors[i].r << 16 |
			pal.colors[i].g << 8 |
			pal.colors[i].b << 0;
	palette.pack();

	std::ofstream pstream{path, std::ios::binary};

	if (pstream.fail()) {
		auto err = this->localize("error.open", path.string(), strerror(errno));

		// FIXME: strerror only works on Linux (err.message()?)
		Utils::dispMsg(game->gui, this->localize("message_box.title.save_pal_err"), err, MB_ICONERROR);
		game->logger.error(err);
		return;
	}
	ShadyCore::getResourceWriter({ShadyCore::FileType::TYPE_PALETTE, format})(&palette, pstream);
	pstream.close();
	palette.destroy();
}

void SpiralOfFate::MainWindow::importPalette(const std::filesystem::path &path)
{
	ShadyCore::FileType::Format format;

	if (path.extension() == ".act")
		format = ShadyCore::FileType::PALETTE_ACT;
	else
		format = ShadyCore::FileType::PALETTE_PAL;

	std::ifstream stream{path, std::ifstream::binary};
	Palette pal;
	ShadyCore::Palette palette;

	if (stream.fail()) {
		auto err = this->localize("error.open", path.string(), strerror(errno));

		// FIXME: strerror only works on Linux (err.message()?)
		Utils::dispMsg(game->gui, this->localize("message_box.title.open_pal_err"), err, MB_ICONERROR);
		game->logger.error(err);
		return;
	}

	size_t size = std::filesystem::file_size(path);

	if (size != 513 && size > 1024) {
		std::array<uint32_t, 256> data;
		std::vector<char> metadata;
		struct {
			const char *character;
			const char *name;
			const char *description;
			const char *author;
		} meta;

		metadata.resize(size - 1024);
		stream.read(reinterpret_cast<char *>(data.data()), 1024);
		stream.read(metadata.data(), size - 1024);

		meta.character = metadata.data();
		meta.name = meta.character + strlen(meta.character) + 1;
		meta.description = meta.name + strlen(meta.name) + 1;
		meta.author = meta.description + strlen(meta.description) + 1;
		if (*meta.name || *meta.author || *meta.description)
			Utils::dispMsg(
				game->gui,
				this->_editor.localize("message_box.title.palette_meta"),
				this->_editor.localize("message_box.palette_meta", meta.name, meta.author, meta.description),
				MB_ICONINFORMATION
			);

		for (size_t i = 0 ; i < 256; i++) {
			pal.colors[i].r = data[i] >> 16 & 0xFF;
			pal.colors[i].g = data[i] >> 8  & 0xFF;
			pal.colors[i].b = data[i] >> 0  & 0xFF;
			pal.colors[i].a = i == 0 ? 0 : 255;
		}
	} else {
		ShadyCore::getResourceReader({ShadyCore::FileType::TYPE_PALETTE, format})(&palette, stream);
		stream.close();
		palette.unpack();

		auto ptr = reinterpret_cast<uint32_t *>(palette.data);

		for (size_t i = 0 ; i < 256; i++) {
			pal.colors[i].r = ptr[i] >> 16 & 0xFF;
			pal.colors[i].g = ptr[i] >> 8  & 0xFF;
			pal.colors[i].b = ptr[i] >> 0  & 0xFF;
			pal.colors[i].a = i == 0 ? 0 : 255;
		}
	}

	int iter = 0;
	tgui::String name = path.filename().native();
	auto pos = name.find_last_of('.');

	if (pos == tgui::String::npos) {
		pos = name.size();
		name += ".pal";
	}

	tgui::String result = name;

	while (std::ranges::any_of(this->_palettes, [&result](const Palette &p){ return p.name == result; })) {
		++iter;
		result = name.substr(0, pos) + "-" + std::to_string(iter) + name.substr(pos);
	}

	pal.name = result;
	pal.path = game->settings.palettes / this->_character / result.toWideString();
	pal.modifications = 1;
	this->applyOperation(new CreatePaletteOperation(
		this->_palettes,
		this->localize("palette.import_action"),
		pal, this->_selectedPalette,
		this->_setPaletteIndex
	));
}

void SpiralOfFate::MainWindow::_autoSaveLoop()
{
	while (!this->_stopped) {
		for (size_t i = 0; i < 100; i++) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			if (this->_stopped)
				return;
		}

		std::lock_guard guard{this->_saveMutex};

		if (this->_pendingTransaction)
			continue;
		this->autoSave();
	}
}

void SpiralOfFate::MainWindow::_onPaletteChanged()
{
	auto &pal = this->_palettes[this->_selectedPalette];

	if (!pal.path.empty())
		return;

	Palette copy;
	tgui::String name = pal.name.substr(3);
	auto pos = name.find_last_of('.');
	tgui::String result = name;
	int iter = 0;

	while (std::ranges::any_of(this->_palettes, [&result](const Palette &p){ return p.name == result; })) {
		++iter;
		result = name.substr(0, pos) + "-" + std::to_string(iter) + name.substr(pos);
	}
	copy.colors = pal.colors;
	copy.name = result;
	copy.path = game->settings.palettes / this->_character / result.toWideString();
	copy.modifications = 1;

	this->applyOperation(new CreatePaletteOperation(
		this->_palettes,
		this->localize("palette.create_action"),
		copy,
		this->_selectedPalette,
		this->_setPaletteIndex
	));

	this->_preview->setPalette(&this->_palettes[this->_selectedPalette].colors);
	this->_requireReload = true;
}

nlohmann::json SpiralOfFate::MainWindow::_asJson() const
{
	nlohmann::json json;

	json["framedata"] = nlohmann::json::object();
	json["palettes"] = nlohmann::json::array();

	auto &framedata = json["framedata"];
	auto &palettes = json["palettes"];

	json["path"] = this->_path.native();
	json["title"] = this->_title;
	json["folder"] = this->_chrPath;
	json["fileName"] = this->_fileName;
	json["character"] = this->_character;
	json["modifications"] = this->_modifications;
	json["isCharacterData"] = this->_object->_schema.isCharacterData;
	for (auto &[key, action] : this->_object->_schema.framedata) {
		auto &actionj = framedata[std::to_string(key)];

		if (action.cloned) {
			actionj = action.clonedId;
			continue;
		}
		actionj = nlohmann::json::array();
		for (auto &sequence : action) {
			actionj.push_back(nlohmann::json::object());
			auto &seqj = actionj.back();

			seqj["frames"] = nlohmann::json::array();
			auto &frames = seqj["frames"];

			seqj["loop"] = sequence.loop;
			seqj["moveLock"] = sequence.moveLock;
			seqj["actionLock"] = sequence.actionLock;
			for (auto &frame : sequence)
				frames.push_back(frame.toJson());
		}
	}
	for (auto &palette : this->_palettes) {
		palettes.push_back(nlohmann::json::object());
		auto &palj = palettes.back();

		palj["colors"] = nlohmann::json::array();
		auto &colors = palj["colors"];

		for (auto &c : palette.colors)
			colors.push_back({c.r, c.g, c.b});
		palj["path"] = palette.path.native();
		palj["modifications"] = palette.modifications;
		palj["name"] = palette.name.toStdString();
	}
	return json;
}

void SpiralOfFate::MainWindow::autoSave()
{
	std::lock_guard guard{this->_saveMutex};
	auto j = this->_asJson();
	std::ofstream stream{this->_pathBak};

	if (stream.fail()) {
		// FIXME: strerror only works on Linux (err.message()?)
		Utils::dispMsg(game->gui, "Saving failed", this->_pathBak.string() + ": " + strerror(errno), MB_ICONERROR);
		return;
	}
	stream << j;
	stream.close();
	this->_requireAutoSave = false;

	std::string title = this->_title;

	if (this->isFramedataModified())
		title.push_back('*');
	this->setTitle(title);
}

std::string SpiralOfFate::MainWindow::_localizeEffectName(unsigned id) const
{
	auto it = this->_effectLabels.find(id);

	if (it != this->_effectLabels.end())
		return it->second;
	return "Effect #" + std::to_string(id);
}

std::string SpiralOfFate::MainWindow::_localizeActionName(unsigned int id) const
{
	auto it = this->_labels.find(id);

	if (it != this->_labels.end())
		return it->second;
	return "Action #" + std::to_string(id);
}

SpiralOfFate::LocalizedContainer<tgui::ChildWindow>::Ptr SpiralOfFate::MainWindow::_createPopup(const std::string &path)
{
	auto outsidePanel = tgui::Panel::create({"100%", "100%"});
	auto contentPanel = std::make_shared<LocalizedContainer<tgui::ChildWindow>>(this->_editor);
	tgui::Vector2f size = {0, 0};

	outsidePanel->getRenderer()->setBackgroundColor({0, 0, 0, 175});
	this->add(outsidePanel);

	contentPanel->setPosition("(&.w - w) / 2", "(&.h - h) / 2");
	this->add(contentPanel);
	contentPanel->setSize(100, 100);

	auto closePopup = [this](std::weak_ptr<tgui::Panel> outsidePanel, std::weak_ptr<tgui::ChildWindow> contentPanel){
		auto content = contentPanel.lock();

		this->remove(outsidePanel.lock());
		this->remove(content);
		this->_updateFrameElements.erase(&*content);
		this->_containers.erase(&*content);
	};
	auto data = this->_object->getFrameData();

	contentPanel->localizationOverride.clear();
	if (this->_object->_action >= 800)
		contentPanel->localizationOverride["animation.player."] = "animation.object.";
	contentPanel->loadLocalizedWidgetsFromFile(path);
	for (auto &w : contentPanel->getWidgets()) {
		size.x = std::max(size.x, w->getFullSize().x + w->getPosition().x + 10);
		size.y = std::max(size.y, w->getFullSize().y + w->getPosition().y + 10);
	}
	size.y += contentPanel->getSize().y - contentPanel->getInnerSize().y;
	contentPanel->setSize(size);
	Utils::setRenderer(contentPanel->cast<tgui::Container>());
	outsidePanel->onClick(closePopup, std::weak_ptr(outsidePanel), std::weak_ptr(contentPanel));
	contentPanel->onClose(closePopup, std::weak_ptr(outsidePanel), std::weak_ptr(contentPanel));
	return contentPanel;
}

void SpiralOfFate::MainWindow::_createGenericPopup(const std::string &path)
{
	auto outsidePanel = tgui::Panel::create({"100%", "100%"});
	auto contentPanel = std::make_shared<LocalizedContainer<tgui::ScrollablePanel>>(this->_editor);
	tgui::Vector2f size = {0, 0};

	outsidePanel->getRenderer()->setBackgroundColor({0, 0, 0, 175});
	this->add(outsidePanel);

	contentPanel->setPosition("(&.w - w) / 2", "(&.h - h) / 2");
	this->add(contentPanel);

	auto closePopup = [this](std::weak_ptr<tgui::Panel> outsidePanel, std::weak_ptr<tgui::ScrollablePanel> contentPanel){
		auto content = contentPanel.lock();

		this->remove(outsidePanel.lock());
		this->remove(content);
		this->_updateFrameElements.erase(&*content);
		this->_containers.erase(&*content);
	};
	auto data = this->_object->getFrameData();

	contentPanel->loadLocalizedWidgetsFromFile(path);
	for (auto &w : contentPanel->getWidgets()) {
		size.x = std::max(size.x, w->getFullSize().x + w->getPosition().x + 10);
		size.y = std::max(size.y, w->getFullSize().y + w->getPosition().y + 10);
	}
	contentPanel->setSize(size);
	Utils::setRenderer(contentPanel->cast<tgui::Container>());
	outsidePanel->onClick(closePopup, std::weak_ptr(outsidePanel), std::weak_ptr(contentPanel));
	this->_placeUIHooks(*contentPanel);
	this->_populateData(*contentPanel);
}

void SpiralOfFate::MainWindow::_createEffectListPopup(const std::function<void(unsigned)> &onConfirm, unsigned current)
{
	assert_exp(this->_effectObject);

	auto outsidePanel = tgui::Panel::create({"100%", "100%"});
	auto contentPanel = tgui::Panel::create({756, "&.h - 100"});
	auto effectsPanel = tgui::ScrollablePanel::create({"&.w", "&.h - 42"});
	auto search = tgui::EditBox::create();
	auto scroll = 0;
	unsigned i = 0;
	std::set<unsigned> effects;
	std::unordered_map<unsigned, std::string> effectsNames;
	std::unordered_map<unsigned, std::string> effectsNamesLower;
	auto preview = std::make_shared<PreviewWidget>(std::ref(this->_editor), std::ref(*this), *this->_effectObject);
	auto previewLabel = tgui::Label::create();

	effects.insert(0);
	for (const auto &effectId: this->_effectObject->_schema.framedata | std::views::keys)
		effects.insert(effectId);

	this->_effectObject->_action = current;
	this->_effectObject->_actionBlock = 0;
	this->_effectObject->_animation = 0;
	this->_effectObject->_animationCtr = 0;

	search->setPosition({10, 10});
	search->setSize({"&.w - 20", 22});
	search->setDefaultText(this->localize("popup.search"));
	contentPanel->add(search);
	effectsPanel->setPosition(0, 42);
	effectsPanel->getRenderer()->setBorders({0, 1, 0, 0});
	contentPanel->add(effectsPanel);

	preview->_scale = 0.5;
	preview->displayBoxes = false;
	preview->_translate = {48, 44};
	preview->setSize(256, 256);
	preview->setPosition(470, 52);
	contentPanel->add(preview);

	outsidePanel->getRenderer()->setBackgroundColor({0, 0, 0, 175});
	outsidePanel->setUserData(false);
	this->add(outsidePanel);

	contentPanel->setPosition("(&.w - w) / 2", "(&.h - h) / 2");
	this->add(contentPanel);
	for (auto effectId : effects) {
		effectsNames[effectId] = this->_localizeEffectName(effectId);
		effectsNamesLower[effectId].resize(effectsNames[effectId].size());
		std::ranges::transform(effectsNames[effectId], effectsNamesLower[effectId].begin(), [](signed char c) -> char {
			// TODO: Handle non-ascii characters properly
			if (c < 0) return c;
			return std::tolower(c);
		});
		if (effectId == current)
			scroll = i * 25 + 20;
		i++;
	}

	previewLabel->getRenderer()->setTextColor(tgui::Color::Black);
	previewLabel->setText(std::to_string(current) + " - " + effectsNames[current]);
	previewLabel->setPosition(470, 52);
	contentPanel->add(previewLabel);

	auto closePopup = [this](const std::weak_ptr<tgui::Panel> &outsidePanel_w, const std::weak_ptr<tgui::Panel> &contentPanel_w){
		this->remove(outsidePanel_w.lock());
		this->remove(contentPanel_w.lock());
	};
	auto refresh = [previewLabel, effects, effectsNames, effectsNamesLower, this, onConfirm, closePopup, current](
		const std::weak_ptr<tgui::EditBox> &search_w,
		const std::weak_ptr<tgui::ScrollablePanel> &movesPanel_w,
		const std::weak_ptr<tgui::Panel> &outsidePanel_w,
		const std::weak_ptr<tgui::Panel> &contentPanel_w
	) {
		unsigned index = 0;
		std::string query = search_w.lock()->getText().toStdString();
		auto movesPanel_p = movesPanel_w.lock();

		// TODO: Handle non-ascii characters
		std::ranges::transform(query, query.begin(), [](signed char c) -> char {
			if (c < 0) return c;
			return std::tolower(c);
		});
		movesPanel_p->removeAllWidgets();
		for (auto effectId : effects) {
			if (effectsNamesLower.at(effectId).find(query) == std::string::npos)
				continue;

			const auto &name = effectsNames.at(effectId);
			auto label = tgui::Label::create(std::to_string(effectId));
			auto button = tgui::Button::create(name);

			label->setPosition(10, index * 25 + 5);
			button->setPosition(50, index * 25 + 3);
			button->setSize(410, 20);
			Utils::setRenderer(button);
			Utils::setRenderer(label);
			if (effectId == current) {
				button->getRenderer()->setTextColor(tgui::Color::Green);
				button->getRenderer()->setTextColorHover(tgui::Color{0x40, 0xFF, 0x40});
				button->getRenderer()->setTextColorDisabled(tgui::Color{0x00, 0xA0, 0x00});
				button->getRenderer()->setTextColorDown(tgui::Color{0x00, 0x80, 0x00});
				button->getRenderer()->setTextColorFocused(tgui::Color{0x20, 0x80, 0x20});
			}

			button->onClick(onConfirm, effectId);
			button->onClick(closePopup, outsidePanel_w, contentPanel_w);
			button->onMouseEnter([previewLabel, effectsNames, this, effectId] {
				try {
					previewLabel->setText(std::to_string(effectId) + " - " + effectsNames.at(effectId));
				} catch (...) {
					previewLabel->setText(std::to_string(effectId) + " - effect." + std::to_string(effectId));
				}
				this->_effectObject->_action = effectId;
				this->_effectObject->_actionBlock = 0;
				this->_effectObject->_animation = 0;
				this->_effectObject->_animationCtr = 0;
			});

			movesPanel_p->add(label);
			movesPanel_p->add(button);
			index++;
		}

		auto label = tgui::Label::create("");

		label->setPosition(10, index * 25);
		label->setSize(100, 5);
		label->setTextSize(1);
		movesPanel_p->add(label);
	};
	auto validate = [effects, effectsNamesLower, onConfirm, closePopup](
		const std::weak_ptr<tgui::EditBox> &search_w,
		const std::weak_ptr<tgui::Panel> &outsidePanel_w,
		const std::weak_ptr<tgui::Panel> &contentPanel_w
	) {
		unsigned index = 0;
		unsigned lastIndex = 0;
		unsigned move;
		std::string query = search_w.lock()->getText().toStdString();

		// TODO: Handle non-ascii characters
		std::ranges::transform(query, query.begin(), [](signed char c) -> char { if (c < 0) return c; return std::tolower(c);});
		for (auto moveId : effects) {
			auto &name = effectsNamesLower.at(moveId);

			if (name == query) {
				move = moveId;
				lastIndex = 1;
				break;
			}
			if (name.find(query) != std::string::npos) {
				if (lastIndex)
					lastIndex = UINT32_MAX;
				else
					lastIndex = index + 1;
				move = moveId;
			}
			index++;
		}
		if (lastIndex == UINT32_MAX || lastIndex == 0)
			return;
		onConfirm(move);
		closePopup(outsidePanel_w, contentPanel_w);
	};

	refresh(std::weak_ptr(search), std::weak_ptr(effectsPanel), std::weak_ptr(outsidePanel), std::weak_ptr(contentPanel));
	search->onTextChange(refresh, std::weak_ptr(search), std::weak_ptr(effectsPanel), std::weak_ptr(outsidePanel), std::weak_ptr(contentPanel));
	search->onReturnKeyPress(validate, std::weak_ptr(search), std::weak_ptr(outsidePanel), std::weak_ptr(contentPanel));
	scroll -= effectsPanel->getSize().y / 2;
	if (scroll > 0)
		effectsPanel->getVerticalScrollbar()->setValue(scroll);
	outsidePanel->onClick(closePopup, std::weak_ptr(outsidePanel), std::weak_ptr(contentPanel));
	search->setFocused(true);
}

void SpiralOfFate::MainWindow::_createSfxListPopup(const std::function<void(unsigned)> &onConfirm, unsigned current)
{
	auto outsidePanel = tgui::Panel::create({"100%", "100%"});
	auto contentPanel = tgui::Panel::create({500, "&.h - 100"});
	auto sfxsPanel = tgui::ScrollablePanel::create({"&.w", "&.h - 42"});
	auto search = tgui::EditBox::create();
	auto scroll = 0;
	unsigned i = 0;
	std::set<unsigned> sfxs;
	std::unordered_map<unsigned, std::string> sfxNames;
	std::unordered_map<unsigned, std::string> sfxNamesLower;

	sfxs.insert(0);
	for (const auto &moveId: game->soundEffects | std::views::keys)
		sfxs.insert(moveId);

	search->setPosition({10, 10});
	search->setSize({"&.w - 20", 22});
	search->setDefaultText(this->localize("popup.search"));
	contentPanel->add(search);
	sfxsPanel->setPosition(0, 42);
	sfxsPanel->getRenderer()->setBorders({0, 1, 0, 0});
	contentPanel->add(sfxsPanel);

	outsidePanel->getRenderer()->setBackgroundColor({0, 0, 0, 175});
	outsidePanel->setUserData(false);
	this->add(outsidePanel);

	contentPanel->setPosition("(&.w - w) / 2", "(&.h - h) / 2");
	this->add(contentPanel);
	for (auto sndId : sfxs) {
		sfxNames[sndId] = this->localize("sound.system." + std::to_string(sndId));
		sfxNamesLower[sndId].resize(sfxNames[sndId].size());
		std::ranges::transform(sfxNames[sndId], sfxNamesLower[sndId].begin(), [](signed char c) -> char {
			// TODO: Handle non-ascii characters properly
			if (c < 0) return c;
			return std::tolower(c);
		});
		if (sndId == current)
			scroll = i * 25 + 20;
		i++;
	}

	auto closePopup = [this](const std::weak_ptr<tgui::Panel> &outsidePanel_w, const std::weak_ptr<tgui::Panel> &contentPanel_w){
		this->remove(outsidePanel_w.lock());
		this->remove(contentPanel_w.lock());
	};
	auto refresh = [sfxs, sfxNames, sfxNamesLower, this, onConfirm, closePopup, current](
		const std::weak_ptr<tgui::EditBox> &search_w,
		const std::weak_ptr<tgui::ScrollablePanel> &movesPanel_w,
		const std::weak_ptr<tgui::Panel> &outsidePanel_w,
		const std::weak_ptr<tgui::Panel> &contentPanel_w
	) {
		unsigned index = 0;
		std::string query = search_w.lock()->getText().toStdString();
		auto movesPanel_p = movesPanel_w.lock();

		// TODO: Handle non-ascii characters
		std::ranges::transform(query, query.begin(), [](signed char c) -> char {
			if (c < 0) return c;
			return std::tolower(c);
		});
		movesPanel_p->removeAllWidgets();
		for (auto sfxId : sfxs) {
			if (sfxNamesLower.at(sfxId).find(query) == std::string::npos)
				continue;

			const auto &name = sfxNames.at(sfxId);
			auto label = tgui::Label::create(std::to_string(sfxId));
			auto button = tgui::Button::create(name);
			auto play = tgui::BitmapButton::create();

			play->setImage({ "assets/gui/editor/play.png" });
			play->setPosition(50, index * 25 + 5);
			play->setSize(20, 20);
			label->setPosition(10, index * 25 + 5);
			button->setPosition(80, index * 25 + 3);
			button->setSize(370, 20);
			Utils::setRenderer(button);
			Utils::setRenderer(label);
			if (sfxId == current) {
				button->getRenderer()->setTextColor(tgui::Color::Green);
				button->getRenderer()->setTextColorHover(tgui::Color{0x40, 0xFF, 0x40});
				button->getRenderer()->setTextColorDisabled(tgui::Color{0x00, 0xA0, 0x00});
				button->getRenderer()->setTextColorDown(tgui::Color{0x00, 0x80, 0x00});
				button->getRenderer()->setTextColorFocused(tgui::Color{0x20, 0x80, 0x20});
			}

			play->onClick([sfxId] { game->soundMgr.play(game->soundEffects[sfxId]); });
			button->onClick(onConfirm, sfxId);
			button->onClick(closePopup, outsidePanel_w, contentPanel_w);

			movesPanel_p->add(label);
			movesPanel_p->add(button);
			if (sfxId)
				movesPanel_p->add(play);
			index++;
		}

		auto label = tgui::Label::create("");

		label->setPosition(10, index * 25);
		label->setSize(100, 5);
		label->setTextSize(1);
		movesPanel_p->add(label);
	};
	auto validate = [sfxs, sfxNamesLower, onConfirm, closePopup](
		const std::weak_ptr<tgui::EditBox> &search_w,
		const std::weak_ptr<tgui::Panel> &outsidePanel_w,
		const std::weak_ptr<tgui::Panel> &contentPanel_w
	) {
		unsigned index = 0;
		unsigned lastIndex = 0;
		unsigned move;
		std::string query = search_w.lock()->getText().toStdString();

		// TODO: Handle non-ascii characters
		std::ranges::transform(query, query.begin(), [](signed char c) -> char { if (c < 0) return c; return std::tolower(c);});
		for (auto sndId : sfxs) {
			auto &name = sfxNamesLower.at(sndId);

			if (name == query) {
				move = sndId;
				lastIndex = 1;
				break;
			}
			if (name.find(query) != std::string::npos) {
				if (lastIndex)
					lastIndex = UINT32_MAX;
				else
					lastIndex = index + 1;
				move = sndId;
			}
			index++;
		}
		if (lastIndex == UINT32_MAX || lastIndex == 0)
			return;
		onConfirm(move);
		closePopup(outsidePanel_w, contentPanel_w);
	};

	refresh(std::weak_ptr(search), std::weak_ptr(sfxsPanel), std::weak_ptr(outsidePanel), std::weak_ptr(contentPanel));
	search->onTextChange(refresh, std::weak_ptr(search), std::weak_ptr(sfxsPanel), std::weak_ptr(outsidePanel), std::weak_ptr(contentPanel));
	search->onReturnKeyPress(validate, std::weak_ptr(search), std::weak_ptr(outsidePanel), std::weak_ptr(contentPanel));
	scroll -= sfxsPanel->getSize().y / 2;
	if (scroll > 0)
		sfxsPanel->getVerticalScrollbar()->setValue(scroll);
	outsidePanel->onClick(closePopup, std::weak_ptr(outsidePanel), std::weak_ptr(contentPanel));
	search->setFocused(true);
}

void SpiralOfFate::MainWindow::_createMoveListPopup(const std::function<void(unsigned)> &onConfirm, unsigned current, bool showNotAdded)
{
	auto outsidePanel = tgui::Panel::create({"100%", "100%"});
	auto contentPanel = tgui::Panel::create({500, "&.h - 100"});
	auto movesPanel = tgui::ScrollablePanel::create({"&.w", "&.h - 42"});
	auto search = tgui::EditBox::create();
	auto scroll = 0;
	unsigned i = 0;
	std::set<unsigned> moves;
	std::unordered_map<unsigned, std::string> movesNames;
	std::unordered_map<unsigned, std::string> movesNamesLower;

	for (const auto &moveId: this->_object->_schema.framedata | std::views::keys)
		moves.insert(moveId);
	if (showNotAdded) {
		// TODO:
		//for (const auto &moveId: SpiralOfFate::actionNames | std::views::keys)
		//	moves.insert(moveId);
		for (const auto &local: this->_editor.getLocalizationData() | std::views::keys) {
			try {
				if (local.rfind("action.generic.", 0) == 0)
					moves.insert(std::stoul(local.substr(strlen("action.generic."))));
				else if (local.rfind("action." + this->_character + ".", 0) == 0)
					moves.insert(std::stoul(local.substr(strlen("action..") + this->_character.size())));
			} catch (std::exception &e) {
				Utils::dispMsg(game->gui, "Error", local + ": " + e.what(), 0);
			}
		}
	}

	search->setPosition({10, 10});
	search->setSize({"&.w - 20", 22});
	search->setDefaultText(this->localize("popup.search"));
	contentPanel->add(search);
	movesPanel->setPosition(0, 42);
	movesPanel->getRenderer()->setBorders({0, 1, 0, 0});
	contentPanel->add(movesPanel);

	outsidePanel->getRenderer()->setBackgroundColor({0, 0, 0, 175});
	outsidePanel->setUserData(false);
	this->add(outsidePanel);

	contentPanel->setPosition("(&.w - w) / 2", "(&.h - h) / 2");
	this->add(contentPanel);
	for (auto moveId : moves) {
		movesNames[moveId] = this->_localizeActionName(moveId);
		movesNamesLower[moveId].resize(movesNames[moveId].size());
		std::ranges::transform(movesNames[moveId], movesNamesLower[moveId].begin(), [](signed char c) -> char {
			// TODO: Handle non-ascii characters properly
			if (c < 0) return c;
			return std::tolower(c);
		});
		if (moveId == current)
			scroll = i * 25 + 20;
		i++;
	}

	auto closePopup = [this](const std::weak_ptr<tgui::Panel> &outsidePanel_w, const std::weak_ptr<tgui::Panel> &contentPanel_w){
		this->remove(outsidePanel_w.lock());
		this->remove(contentPanel_w.lock());
	};
	auto refresh = [moves, movesNames, movesNamesLower, this, onConfirm, closePopup, current](
		const std::weak_ptr<tgui::EditBox> &search_w,
		const std::weak_ptr<tgui::ScrollablePanel> &movesPanel_w,
		const std::weak_ptr<tgui::Panel> &outsidePanel_w,
		const std::weak_ptr<tgui::Panel> &contentPanel_w
	) {
		unsigned index = 0;
		std::string query = search_w.lock()->getText().toStdString();
		auto movesPanel_p = movesPanel_w.lock();

		// TODO: Handle non-ascii characters
		std::ranges::transform(query, query.begin(), [](signed char c) -> char {
			if (c < 0) return c;
			return std::tolower(c);
		});
		movesPanel_p->removeAllWidgets();
		for (auto moveId : moves) {
			if (movesNamesLower.at(moveId).find(query) == std::string::npos)
				continue;

			auto name = movesNames.at(moveId);
			auto label = tgui::Label::create(std::to_string(moveId));
			auto button = tgui::Button::create();

			label->setPosition(10, index * 25 + 5);
			button->setPosition(50, index * 25 + 3);
			button->setSize(410, 20);
			Utils::setRenderer(button);
			Utils::setRenderer(label);
			if (name.starts_with('$')) {
				tgui::Color color{"#" + name.substr(1, 6)};

				name = name.substr(7);
				button->getRenderer()->setTextColor(color);
				button->getRenderer()->setTextColorHover(color);
				button->getRenderer()->setTextColorDisabled(color);
				button->getRenderer()->setTextColorDown(color);
				button->getRenderer()->setTextColorFocused(color);
			}
			if (moveId == current) {
				button->getRenderer()->setTextColor(tgui::Color::Green);
				button->getRenderer()->setTextColorHover(tgui::Color{0x40, 0xFF, 0x40});
				button->getRenderer()->setTextColorDisabled(tgui::Color{0x00, 0xA0, 0x00});
				button->getRenderer()->setTextColorDown(tgui::Color{0x00, 0x80, 0x00});
				button->getRenderer()->setTextColorFocused(tgui::Color{0x20, 0x80, 0x20});
			} else if (!this->_object->_schema.framedata.contains(moveId)) {
				button->getRenderer()->setTextColor(tgui::Color{0xFF, 0x00, 0x00});
				button->getRenderer()->setTextColorHover(tgui::Color{0xFF, 0x40, 0x40});
				button->getRenderer()->setTextColorDisabled(tgui::Color{0xA0, 0x00, 0});
				button->getRenderer()->setTextColorDown(tgui::Color{0x80, 0x00, 0x00});
				button->getRenderer()->setTextColorFocused(tgui::Color{0x80, 0x20, 0x20});
			} else if (name.rfind("Action #", 0) == 0) {
				button->getRenderer()->setTextColor(tgui::Color{0xFF, 0x80, 0x00});
				button->getRenderer()->setTextColorHover(tgui::Color{0xFF, 0xA0, 0x40});
				button->getRenderer()->setTextColorDisabled(tgui::Color{0xA0, 0x50, 0});
				button->getRenderer()->setTextColorDown(tgui::Color{0x80, 0x40, 0x00});
				button->getRenderer()->setTextColorFocused(tgui::Color{0x80, 0x60, 0x20});
			}

			button->setText(name);
			button->onClick(onConfirm, moveId);
			button->onClick(closePopup, outsidePanel_w, contentPanel_w);

			movesPanel_p->add(label);
			movesPanel_p->add(button);
			index++;
		}

		auto label = tgui::Label::create("");

		label->setPosition(10, index * 25);
		label->setSize(100, 5);
		label->setTextSize(1);
		movesPanel_p->add(label);
	};
	auto validate = [moves, movesNamesLower, onConfirm, closePopup](
		const std::weak_ptr<tgui::EditBox> &search_w,
		const std::weak_ptr<tgui::Panel> &outsidePanel_w,
		const std::weak_ptr<tgui::Panel> &contentPanel_w
	) {
		unsigned index = 0;
		unsigned lastIndex = 0;
		unsigned move;
		std::string query = search_w.lock()->getText().toStdString();

		// TODO: Handle non-ascii characters
		std::ranges::transform(query, query.begin(), [](signed char c) -> char { if (c < 0) return c; return std::tolower(c);});
		for (auto moveId : moves) {
			auto &name = movesNamesLower.at(moveId);

			if (name == query) {
				move = moveId;
				lastIndex = 1;
				break;
			}
			if (name.find(query) != std::string::npos) {
				if (lastIndex)
					lastIndex = UINT32_MAX;
				else
					lastIndex = index + 1;
				move = moveId;
			}
			index++;
		}
		if (lastIndex == UINT32_MAX || lastIndex == 0)
			return;
		onConfirm(move);
		closePopup(outsidePanel_w, contentPanel_w);
	};

	refresh(std::weak_ptr(search), std::weak_ptr(movesPanel), std::weak_ptr(outsidePanel), std::weak_ptr(contentPanel));
	search->onTextChange(refresh, std::weak_ptr(search), std::weak_ptr(movesPanel), std::weak_ptr(outsidePanel), std::weak_ptr(contentPanel));
	search->onReturnKeyPress(validate, std::weak_ptr(search), std::weak_ptr(outsidePanel), std::weak_ptr(contentPanel));
	scroll -= movesPanel->getSize().y / 2;
	if (scroll > 0)
		movesPanel->getVerticalScrollbar()->setValue(scroll);
	outsidePanel->onClick(closePopup, std::weak_ptr(outsidePanel), std::weak_ptr(contentPanel));
	search->setFocused(true);
}

void SpiralOfFate::MainWindow::_placeUIHooks(tgui::Container &container)
{
	auto clearHit = container.get<tgui::Button>("ClearHit");
	auto clearBlock = container.get<tgui::Button>("ClearBlock");
	auto action = container.get<tgui::EditBox>("ActionID");
	auto actionSelect = container.get<tgui::Button>("ActionSelect");
	auto prevAction = container.get<tgui::Button>("PrevAction");
	auto nextAction = container.get<tgui::Button>("NextAction");
	auto play = container.get<tgui::Button>("Play");
	auto step = container.get<tgui::Button>("Step");
	auto frame = container.get<tgui::Slider>("Frame");
	auto blockSpin = container.get<tgui::SpinButton>("BlockSpin");
	auto frameSpin = container.get<tgui::SpinButton>("FrameSpin");
	auto aFlags = container.get<tgui::Button>("AFlagsButton");
	auto dFlags = container.get<tgui::Button>("DFlagsButton");
	auto hitEdit = container.get<tgui::Button>("HitEdit");
	auto blendEdit = container.get<tgui::Button>("BlendEdit");
	auto generalEdit = container.get<tgui::Button>("GeneralEdit");
	auto ctrl = container.get<tgui::Tabs>("ControlTabs");
	auto ctrlPanel = container.get<tgui::Panel>("ControlPanel");
	auto boxes = container.get<tgui::Panel>("SelectedBoxPanel");
	auto boxLabel = container.get<tgui::Label>("SelectedBoxName");
	auto plane = container.get<ColorPlaneWidget>("ColorPlane");
	auto slider = container.get<ColorSliderWidget>("ColorSlider");
	auto comp1 = container.get<tgui::EditBox>("ColorComponent1");
	auto comp2 = container.get<tgui::EditBox>("ColorComponent2");
	auto comp3 = container.get<tgui::EditBox>("ColorComponent3");
	auto hexcode = container.get<tgui::EditBox>("RGBColor");
	auto addPal = container.get<tgui::Button>("AddPalette");
	auto removePal = container.get<tgui::Button>("RemovePalette");
	auto preview = container.get<tgui::Label>("ColorPreview");
	auto paletteList = container.get<tgui::ComboBox>("PaletteList");
	auto colorPanel = container.get<tgui::ScrollablePanel>("AllColorPanel");
	auto mode = container.get<tgui::Tabs>("ColorModes");
	auto boundsButton = container.get<tgui::Button>("BoundsButton");
	auto gndSeq = container.get<tgui::Button>("GndHitSeq");
	auto airSeq = container.get<tgui::Button>("AirHitSeq");
	auto atkLevel = container.get<tgui::ComboBox>("AttackLevel");
	auto selectSnd = container.get<tgui::Button>("SelectSound");
	auto selectEff = container.get<tgui::Button>("SelectEffect");
	tgui::Container &sidePanel = ctrlPanel ? *ctrlPanel : container;

	if (mode)
		mode->onTabSelect([this, &sidePanel](std::weak_ptr<tgui::Tabs> This){
			this->_selectColorMethod = This.lock()->getSelectedIndex();
			this->_reinitSidePanel(sidePanel);
			this->_rePopulateColorData();
		}, std::weak_ptr(mode));
	if (colorPanel) {
		float stepX = (colorPanel->getSize().x - 24) / 8.f;
		float stepY = stepX - 8;
		float extra = this->getSize().y - this->getInnerSize().y;

		colorPanel->setSize("&.w - 20", tgui::Layout("(&.h - y) - 10 - " + std::to_string(extra)));
		colorPanel->removeAllWidgets();
		for (size_t i = 0; i < 256; i++) {
			auto button = tgui::Button::create();

			button->setSize({24, 16});
			button->setPosition({(i % 8) * stepX, (i / 8) * stepY});
			colorPanel->add(button, "Color" + std::to_string(i));

			button->onClick([this, i, &container]{
				this->_selectedColor = i;
				this->_preview->setSelectedColor(i);
				this->_populateColorData(container);
			});
			button->onMouseEnter([this, i]{
				this->_onColorHover(this->_object->_paletteIndex, i, false);
				this->_object->_paletteIndex = i;
				this->_object->_needGenerate = true;
			});
			button->onMouseLeave([this]{
				this->_onColorHover(this->_object->_paletteIndex, -1, false);
				this->_object->_paletteIndex = -1;
			});
		}
	}
	this->_preview->onColorSelect.disconnectAll();
	this->_preview->onColorSelect([this, &container](unsigned colorIndex){
		this->_selectedColor = colorIndex;
		this->_populateColorData(container);
	});
	if (paletteList)
		paletteList->onItemSelect([this, &container](unsigned index){
			if (this->_palettes.empty())
				return;
			this->_selectedPalette = index;
			this->_populateColorData(container);
			this->_preview->setPalette(&this->_palettes[this->_selectedPalette].colors);
		});

	PLACE_HOOK_WIDGET(plane, 0);
	PLACE_HOOK_WIDGET(slider, 1);
	PLACE_HOOK_COLOR_COMPONENT(comp1, 0);
	PLACE_HOOK_COLOR_COMPONENT(comp2, 1);
	PLACE_HOOK_COLOR_COMPONENT(comp3, 2);
	PLACE_HOOK_HEXCOLOR(hexcode);

	if (addPal)
		addPal->onClick([this]{
			auto window = this->_createPopup("assets/gui/editor/palette/add.gui");
			auto create = window->get<tgui::Button>("CreatePalette");
			auto cancel = window->get<tgui::Button>("CancelPalette");
			auto name = window->get<tgui::EditBox>("PaletteName");
			auto list = window->get<tgui::ComboBox>("PaletteList");

			for (auto &pal : this->_palettes) {
				if (pal.modifications != 0)
					list->addItem(pal.name + "*");
				else
					list->addItem(pal.name);
			}
			if (this->_palettes.empty())
				list->setEnabled(false);
			list->setSelectedItemByIndex(this->_selectedPalette);
			create->setEnabled(false);

			name->onTextChange([this](std::weak_ptr<tgui::Button> ptr, const tgui::String &s){
				auto button = ptr.lock();

				if (s.empty()) {
					button->setEnabled(false);
					return;
				}
				for (auto &pal : this->_palettes)
					if (pal.path == s.toStdString()) {
						button->setEnabled(false);
						return;
					}
				button->setEnabled(true);
			}, std::weak_ptr(create));
			create->onClick([this, name, list](std::weak_ptr<LocalizedContainer<tgui::ChildWindow>> ptr){
				Palette copy;
				tgui::String _name = name->getText();
				auto pos = _name.find_last_of('.');
				int iter = 0;

				if (pos == tgui::String::npos) {
					pos = _name.size();
					_name += ".pal";
				}

				tgui::String result = _name;

				while (std::ranges::any_of(this->_palettes, [&result](const Palette &p){ return p.name == result; })) {
					++iter;
					result = _name.substr(0, pos) + "-" + std::to_string(iter) + _name.substr(pos);
				}
				if (this->_palettes.empty()) {
					copy.colors.fill(Color::Black);
					copy.colors[0] = Color::Transparent;
				} else
					copy.colors = this->_palettes[list->getSelectedItemIndex()].colors;
				copy.name = result;
				copy.path = game->settings.palettes / this->_character / result.toWideString();
				copy.modifications = 1;

				this->applyOperation(new CreatePaletteOperation(
					this->_palettes,
					this->localize("palette.create_action"),
					copy,
					this->_selectedPalette,
					this->_setPaletteIndex
				));
				ptr.lock()->close();
			}, std::weak_ptr(window));
			cancel->onClick([](std::weak_ptr<LocalizedContainer<tgui::ChildWindow>> ptr){
				ptr.lock()->close();
			}, std::weak_ptr(window));
		});
	if (removePal)
		removePal->onClick([this]{
			if (this->_palettes[this->_selectedPalette].path.empty())
				return;
			this->applyOperation(new RemovePaletteOperation(
				this->_palettes,
				this->localize("palette.remove_action"),
				this->_selectedPalette,
				this->_setPaletteIndex
			));
		});

	if (ctrl)
		ctrl->onTabSelect([this, &sidePanel](std::weak_ptr<tgui::Tabs> This){
			auto selected = This.lock()->getSelectedIndex();

			if (selected == this->_showingPalette)
				return;

			this->_planes.clear();
			this->_sliders.clear();
			this->_showingPalette = selected;
			if (selected == 1)
				sidePanel.loadWidgetsFromFile("assets/gui/editor/palette/palette.gui");
			else if (this->_object->_schema.isCharacterData)
				sidePanel.loadWidgetsFromFile("assets/gui/editor/character/framedata.gui");
			else
				sidePanel.loadWidgetsFromFile("assets/gui/editor/character/animation.gui");
			this->_initSidePanel(sidePanel);
			this->_localizeWidgets(sidePanel, true);
			Utils::setRenderer(&sidePanel);
			this->_placeUIHooks(sidePanel);
			this->_populateData(sidePanel);
			this->_preview->showingPalette = this->_showingPalette;
		}, std::weak_ptr(ctrl));
	if (action)
		action->onReturnOrUnfocus([this](std::weak_ptr<tgui::EditBox> This, const tgui::String &s){
			if (std::to_string(this->_object->_action) == s)
				return;
			if (s.empty()) {
				This.lock()->setText(std::to_string(this->_object->_action));
				return;
			}

			unsigned action = std::stoul(s.toStdString());

			if (this->_object->_schema.framedata.count(action) == 0) {
				This.lock()->setText(std::to_string(this->_object->_action));
				return;
			}
			this->localizationOverride.clear();
			if (action >= 800)
				this->localizationOverride["animation.player."] = "animation.object.";
			this->_object->_action = action;
			this->_object->_actionBlock = 0;
			this->_object->_animation = 0;
			this->_object->_animationCtr = 0;
			this->_requireReload = true;
		}, std::weak_ptr(action));
	if (clearHit)
		clearHit->onClick([this]{
			this->applyOperation(new ClearAttackOperation(*this->_object, this->_editor));
		});
	if (aFlags)
		aFlags->onClick(&MainWindow::_createGenericPopup, this, "assets/gui/editor/character/attackFlags.gui");
	if (dFlags)
		dFlags->onClick(&MainWindow::_createGenericPopup, this, "assets/gui/editor/character/defenseFlags.gui");
	if (hitEdit)
		hitEdit->onClick(&MainWindow::_createGenericPopup, this, "assets/gui/editor/character/hitProperties.gui");
	if (generalEdit)
		generalEdit->onClick(&MainWindow::_createGenericPopup, this, "assets/gui/editor/character/generalProperties.gui");
	if (blendEdit)
		blendEdit->onClick(&MainWindow::_createGenericPopup, this, "assets/gui/editor/character/blendProperties.gui");
	if (actionSelect)
		actionSelect->onClick(&MainWindow::navGoTo, this);
	if (prevAction)
		prevAction->onClick(&MainWindow::navToPrevAction, this);
	if (nextAction)
		nextAction->onClick(&MainWindow::navToNextAction, this);
	if (play)
		play->onPress([this]{
			this->_paused = false;
		});
	if (step)
		step->onPress([this]{
			this->_paused = false;
			this->tick();
			this->_paused = true;
		});
	if (frame)
		frame->onValueChange([this](float value){
			auto &blk = this->_object->_schema.framedata[this->_object->_action][this->_object->_actionBlock];

			this->_paused = true;
			this->_object->_animation = value;
			this->_object->resetState();
			this->_preview->frameChanged();
			this->_rePopulateFrameData();
			this->_editor.setCanCopyLast(this->_object->_animation > 0);
			this->_editor.setCanCopyNext(this->_object->_animation < blk.size() - 1);
			this->_editor.setHasLastFrame(this->_object->_animation > 0);
			this->_editor.setHasNextFrame(this->_object->_animation < blk.size() - 1);
		});
	if (frameSpin)
		frameSpin->onValueChange([this](float value){
			auto &blk = this->_object->_schema.framedata[this->_object->_action][this->_object->_actionBlock];

			this->_paused = true;
			this->_object->_animation = value;
			this->_object->resetState();
			this->_preview->frameChanged();
			this->_rePopulateFrameData();
			this->_editor.setCanCopyLast(this->_object->_animation > 0);
			this->_editor.setCanCopyNext(this->_object->_animation < blk.size() - 1);
			this->_editor.setHasLastFrame(this->_object->_animation > 0);
			this->_editor.setHasNextFrame(this->_object->_animation < blk.size() - 1);
		});
	if (blockSpin)
		blockSpin->onValueChange([this](float value){
			auto &act = this->_object->_schema.framedata[this->_object->_action];
			auto &blk = act[value];

			this->_object->_actionBlock = value;
			this->_object->_animation = 0;
			this->_requireReload = true;
			this->_editor.setHasLastBlock(this->_object->_actionBlock > 0);
			this->_editor.setHasNextBlock(this->_object->_actionBlock < act.size() - 1);
			this->_editor.setHasLastFrame(false);
			this->_editor.setHasNextFrame(blk.size() != 1);
			this->_editor.setCanCopyLast(false);
			this->_editor.setCanCopyNext(blk.size() != 1);
		});
	if (boundsButton)
		boundsButton->onClick([] {
			// TODO: Not implemented
			Utils::dispMsg(game->gui, "Error", "Not implemented", MB_ICONERROR);
		});
	if (gndSeq) {
		gndSeq->onClick([this]{
			this->_createMoveListPopup([this](unsigned move){
				this->applyOperation(new BasicData2StepOperation(
					*this->_object,
					this->localize("animation.hit.gseq"),
					&FrameData::traits,
					&ShadyCore::Schema::Sequence::MoveTraits::onGroundHitSetSequence,
					static_cast<uint16_t>(move), false
				));
			}, this->_object->getFrameData().traits.onGroundHitSetSequence, true);
		});

		this->_updateFrameElements[&container].emplace_back([gndSeq, this]{
			auto &data = this->_object->getFrameData();
			std::string name;

			if (data.traits.onGroundHitSetSequence == 0)
				name = this->localize("animation.hit.seq.default");
			else
				name = this->_localizeActionName(data.traits.onGroundHitSetSequence);
			if (name.starts_with('$')) {
				tgui::Color color{"#" + name.substr(1, 6)};

				name = name.substr(7);
				gndSeq->getRenderer()->setTextColor(color);
				gndSeq->getRenderer()->setTextColorHover(color);
				gndSeq->getRenderer()->setTextColorDisabled(color);
				gndSeq->getRenderer()->setTextColorDown(color);
				gndSeq->getRenderer()->setTextColorFocused(color);
			} else
				Utils::setRenderer(gndSeq);
			gndSeq->setText(name + " (" + std::to_string(data.traits.onGroundHitSetSequence) + ")");
		});
	}
	if (airSeq) {
		airSeq->onClick([this]{
			this->_createMoveListPopup([this](unsigned move){
				this->applyOperation(new BasicData2StepOperation(
					*this->_object,
					this->localize("animation.hit.aseq"),
					&FrameData::traits,
					&ShadyCore::Schema::Sequence::MoveTraits::onAirHitSetSequence,
					static_cast<uint16_t>(move), false
				));
			}, this->_object->getFrameData().traits.onAirHitSetSequence, true);
		});
		this->_updateFrameElements[&container].emplace_back([airSeq, this]{
			auto &data = this->_object->getFrameData();
			std::string name;

			if (data.traits.onAirHitSetSequence == 0)
				name = this->localize("animation.hit.seq.default");
			else
				name = this->_localizeActionName(data.traits.onAirHitSetSequence);
			if (name.starts_with('$')) {
				tgui::Color color{"#" + name.substr(1, 6)};

				name = name.substr(7);
				airSeq->getRenderer()->setTextColor(color);
				airSeq->getRenderer()->setTextColorHover(color);
				airSeq->getRenderer()->setTextColorDisabled(color);
				airSeq->getRenderer()->setTextColorDown(color);
				airSeq->getRenderer()->setTextColorFocused(color);
			} else
				Utils::setRenderer(airSeq);
			airSeq->setText(name + " (" + std::to_string(data.traits.onAirHitSetSequence) + ")");
		});
	}
	if (atkLevel) {
		atkLevel->onItemSelect([this](unsigned id){
			this->applyOperation(new BasicData2StepOperation(
				*this->_object,
				this->localize("animation.hit.aseq"),
				&FrameData::traits,
				&ShadyCore::Schema::Sequence::MoveTraits::attackLevel,
				static_cast<uint16_t>(id), false
			));
		});
		this->_updateFrameElements[&container].emplace_back([atkLevel, this]{
			auto &data = this->_object->getFrameData();

			atkLevel->setSelectedItemByIndex(data.traits.attackLevel);
		});
	}
	if (selectSnd) {
		selectSnd->onClick([this]{
			this->_createSfxListPopup([this](unsigned move){
				this->applyOperation(new BasicData2StepOperation(
					*this->_object,
					this->localize("animation.hit.sound"),
					&FrameData::traits,
					&ShadyCore::Schema::Sequence::MoveTraits::onHitSfx,
					static_cast<uint16_t>(move), false
				));
			}, this->_object->getFrameData().traits.onHitSfx);
		});
		this->_updateFrameElements[&container].emplace_back([selectSnd, this]{
			auto &data = this->_object->getFrameData();

			selectSnd->setText(this->localize("sound.system." + std::to_string(data.traits.onHitSfx)) + " (" + std::to_string(data.traits.onHitSfx) + ")");
		});
	}
	if (selectEff) {
		selectEff->onClick([this]{
			this->_createEffectListPopup([this](unsigned move){
				this->applyOperation(new BasicData2StepOperation(
					*this->_object,
					this->localize("animation.hit.effect"),
					&FrameData::traits,
					&ShadyCore::Schema::Sequence::MoveTraits::onHitEffect,
					static_cast<uint16_t>(move), false
				));
			}, this->_object->getFrameData().traits.onHitEffect);
		});
		this->_updateFrameElements[&container].emplace_back([selectEff, this]{
			auto &data = this->_object->getFrameData();

			selectEff->setText(this->_localizeEffectName(data.traits.onHitEffect) + " (" + std::to_string(data.traits.onHitEffect) + ")");
		});
	}

	PLACE_HOOK_BOOL_SEQ(container,   "SeqLoop",  loop,       this->localize("animation.loop"), false);
	PLACE_HOOK_NUMBER_SEQ(container, "SeqALock", actionLock, this->localize("animation.player.action_lock"), false, 0);
	PLACE_HOOK_NUMBER_SEQ(container, "SeqMLock", moveLock,   this->localize("animation.player.move_lock"), false, 0);

	PLACE_HOOK_STRING(container, "Sprite",      spritePath,    this->localize("animation.sprite"),   EditSpriteOperation, false);
	PLACE_HOOK_NUMBER(container, "Duration",    duration,      this->localize("animation.duration"), BasicDataOperation, false, 0);
	PLACE_HOOK_VECTOR(container, "Offset",      offset,        this->localize("animation.offset"));
	PLACE_HOOK_RECT(container,   "Bounds",      tex,           this->localize("animation.bounds"));
	PLACE_HOOK_NUMBER(container, "RenderGroup", renderGroup,   this->localize("animation.rendergrp"), BasicDataOperation, false, 0);
	PLACE_HOOK_NUMBER(container, "Unknown",     unknown,       this->localize("animation.unknown"), BasicDataOperation, false, 0);

	PLACE_HOOK_NUMBER2(container,             "Damage",       traits, damage,            this->localize("animation.hit.damage"));
	PLACE_HOOK_SOKU_FLOAT_PERCENT2(container, "Rate",         traits, proration,         this->localize("animation.hit.rate"), 11);
	PLACE_HOOK_NUMBER2(container,             "ChipDamage",   traits, chipDamage,        this->localize("animation.block.chip"));
	PLACE_HOOK_NUMBER2(container,             "SpiritDamage", traits, spiritDamage,      this->localize("animation.block.damage"));
	PLACE_HOOK_NUMBER2(container,             "Untech",       traits, untech,            this->localize("animation.hit.untech"));
	PLACE_HOOK_NUMBER2(container,             "Power",        traits, power,             this->localize("animation.hit.power"));
	PLACE_HOOK_SOKU_FLOAT_PERCENT2(container, "Limit",        traits, limit,             this->localize("animation.hit.limit"), 0);
	PLACE_HOOK_NUMBER2(container,             "PHitStop",     traits, onHitPlayerStun,   this->localize("animation.hit.pstop"));
	PLACE_HOOK_NUMBER2(container,             "OHitStop",     traits, onHitEnemyStun,    this->localize("animation.hit.ostop"));
	PLACE_HOOK_NUMBER2(container,             "PBlockStop",   traits, onBlockPlayerStun, this->localize("animation.block.pstop"));
	PLACE_HOOK_NUMBER2(container,             "OBlockStop",   traits, onBlockEnemyStun,  this->localize("animation.block.ostop"));
	PLACE_HOOK_NUMBER2(container,             "HMeter",       traits, onHitCardGain,     this->localize("animation.hit.meter"));
	PLACE_HOOK_NUMBER2(container,             "BMeter",       traits, onBlockCardGain,   this->localize("animation.block.meter"));
	PLACE_HOOK_VECTOR_SOKU_FLOAT2(container,  "HitSpeed",     traits, speed,             this->localize("animation.hit.speed"),  2);
	PLACE_HOOK_NUMFLAGS2(container,           "cFlags",       traits, comboModifier,     this->localize("animation.hit.cflags"), 2);
	PLACE_HOOK_NUMFLAGS2(container,           "AFlags",       traits, attackFlags,       this->localize("animation.hit.aflags"), 8);
	PLACE_HOOK_NUMFLAGS2(container,           "DFlags",       traits, frameFlags,        this->localize("animation.hit.dflags"), 8);
	for (size_t i = 0; i < 8; i++)
		PLACE_HOOK_FLAG(container, "cFlag" + std::to_string(i), comboModifier, i,    this->localize("animation.hit.cflag" + std::to_string(i)), false);
	for (size_t i = 0; i < 32; i++)
		PLACE_HOOK_FLAG(container, "aFlag" + std::to_string(i), attackFlags,   i,    this->localize("animation.aflag" + std::to_string(i)), false);
	for (size_t i = 0; i < 32; i++)
		PLACE_HOOK_FLAG(container, "dFlag" + std::to_string(i), frameFlags,    i,    this->localize("animation.dflag" + std::to_string(i)), false);

	PLACE_HOOK_NUMBER2(container, "SpeedX",        effect, speedX,              this->localize("animation.extra.speedX"));
	PLACE_HOOK_NUMBER2(container, "SpeedY",        effect, speedY,              this->localize("animation.extra.speedY"));
	PLACE_HOOK_NUMBER2(container, "Unknown02",     effect, unknown02RESETSTATE, this->localize("animation.extra.unknown"));
	PLACE_HOOK_VECTOR2(container, "Pivot",         effect, pivot,               this->localize("animation.extra.pivot"));
	PLACE_HOOK_VECTOR2(container, "PositionExtra", effect, positionExtra,       this->localize("animation.extra.positionextra"));
	PLACE_HOOK_VECTOR2(container, "Position",      effect, position,            this->localize("animation.extra.position"));

	PLACE_HOOK_NUMBER2(container,            "Mode",     blendOptions, mode,     this->localize("animation.blend.mode"));
	PLACE_HOOK_HEXCOLOR_INT2(container,      "Color",    blendOptions, color,    this->localize("animation.blend.color"), 8);
	PLACE_HOOK_VECTOR_SOKU_FLOAT2(container, "Scale",    blendOptions, scale,    this->localize("animation.blend.scale"), 2);
	PLACE_HOOK_NUMBER2(container,            "FlipVert", blendOptions, flipVert, this->localize("animation.blend.flipVert"));
	PLACE_HOOK_NUMBER2(container,            "FlipHorz", blendOptions, flipHorz, this->localize("animation.blend.flipHorz"));
	PLACE_HOOK_NUMBER2(container,            "Angle",    blendOptions, angle,    this->localize("animation.blend.angle"));

	PLACE_HOOK_BOX(container, "SelectedBoxLeft",  left,  this->localize("animation.selectedbox.left"));
	PLACE_HOOK_BOX(container, "SelectedBoxRight", right, this->localize("animation.selectedbox.right"));
	PLACE_HOOK_BOX(container, "SelectedBoxUp",    up,    this->localize("animation.selectedbox.up"));
	PLACE_HOOK_BOX(container, "SelectedBoxDown",  down,  this->localize("animation.selectedbox.down"));
	if (boxes) {
		this->_updateFrameElements[&container].emplace_back([boxes, this]{
			bool hasBox = this->_preview->getSelectedBox().first != BOXTYPE_NONE;

			boxes->setVisible(hasBox);
			if (hasBox)
				boxes->setSize({"&.w", 150});
			else
				boxes->setSize({0, 0});
			for (auto &fct : this->_updateFrameElements[&*boxes])
				fct();
		});
		this->_containers.emplace(&container);
	}
	if (boxLabel) {
		this->_updateFrameElements[&container].emplace_back([boxLabel, this] {
			auto box = this->_preview->getSelectedBox();

			switch (box.first) {
			case BOXTYPE_HITBOX:
				boxLabel->setText(this->localize("animation.selectedbox.hitbox", std::to_string(box.second)));
				break;
			case BOXTYPE_HURTBOX:
				boxLabel->setText(this->localize("animation.selectedbox.hurtbox", std::to_string(box.second)));
				break;
			case BOXTYPE_COLLISIONBOX:
				boxLabel->setText(this->localize("animation.selectedbox.collision"));
				break;
			default:
				boxLabel->setText("");
			}
		});
		this->_containers.emplace(&container);
	}
}


void SpiralOfFate::MainWindow::newFrame()
{
	this->applyOperation(new CreateFrameOperation(
		*this->_object,
		this->localize("operation.create_frame"),
		this->_object->_animation,
		this->_object->getFrameData()
	));
}

void SpiralOfFate::MainWindow::newEndFrame()
{
	this->applyOperation(new CreateFrameOperation(
		*this->_object,
		this->localize("operation.create_frame_end"),
		this->_object->_schema.framedata[this->_object->_action][this->_object->_actionBlock].size(),
		this->_object->getFrameData()
	));
}

void SpiralOfFate::MainWindow::newAnimationBlock()
{
	auto &seq = this->_object->_schema.framedata[this->_object->_action][this->_object->_actionBlock];

	this->applyOperation(new CreateBlockOperation(
		*this->_object,
		this->localize("operation.create_block"),
		this->_object->_actionBlock + 1,
		FrameData::Sequence{
			.data = {this->_object->getFrameData()},
			.moveLock = seq.moveLock,
			.actionLock = seq.actionLock,
			.loop = seq.loop
		}
	));
}

void SpiralOfFate::MainWindow::newAction()
{
	auto window = this->_createPopup("assets/gui/editor/character/actionCreate.gui");
	auto idBox = window->get<tgui::EditBox>("ActionID");
	auto action = window->get<tgui::Button>("Action");
	auto create = window->get<tgui::Button>("Create");
	auto actionW = std::weak_ptr(action);
	auto createW = std::weak_ptr(create);
	auto idBoxW  = std::weak_ptr(idBox);
	auto windowW = std::weak_ptr(window);

	window->setTitle(this->localize("message_box.title.create_action"));
	create->setEnabled(false);
	idBox->onTextChange([createW, actionW, idBoxW, this](const tgui::String &t){
		if (t.empty()) {
			actionW.lock()->setText("");
			createW.lock()->setEnabled(false);
		} else {
			std::string name = this->_localizeActionName(std::stoul(t.toStdString()));
			auto action = actionW.lock();

			if (name.starts_with('$')) {
				tgui::Color color{"#" + name.substr(1, 6)};

				name = name.substr(7);
				action->getRenderer()->setTextColor(color);
				action->getRenderer()->setTextColorHover(color);
				action->getRenderer()->setTextColorDisabled(color);
				action->getRenderer()->setTextColorDown(color);
				action->getRenderer()->setTextColorFocused(color);
			} else
				Utils::setRenderer(action);
			action->setText(name);
			createW.lock()->setEnabled(true);
		}
	});
	action->onClick(&MainWindow::_createMoveListPopup, this, [idBoxW](unsigned move){
		idBoxW.lock()->setText(std::to_string(move));
	}, this->_object->_action, true);
	create->onClick([windowW, idBox, this]{
		auto &data = this->_object->getFrameData();
		auto &seq = this->_object->_schema.framedata[this->_object->_action][this->_object->_actionBlock];
		auto action = std::stoul(idBox->getText().toStdString());

		if (!this->_object->_schema.framedata.contains(action)) {
			this->applyOperation(new CreateMoveOperation(
				*this->_object,
				this->localize("operation.create_move"),
				action,
				FrameData::Action{
					._sequences = {
						FrameData::Sequence{
							.data = { data },
							.moveLock = seq.moveLock,
							.actionLock = seq.actionLock,
							.loop = seq.loop
						}
					},
					// TODO: Allow to create clones
					.cloned = nullptr,
					.clonedId = 0
				}
			));
			windowW.lock()->close();
			this->_rePopulateData();
		} else
			Utils::dispMsg(game->gui, this->localize("message_box.title.already_exists"), this->localize("message_box.action_exists"), MB_ICONERROR);
	});
}

void SpiralOfFate::MainWindow::newHurtBox()
{
	this->applyOperation(new CreateBoxOperation(
		*this->_object,
		this->localize("operation.create_hurtbox"),
		true
	));
}

void SpiralOfFate::MainWindow::newHitBox()
{
	this->applyOperation(new CreateBoxOperation(
		*this->_object,
		this->localize("operation.create_hitbox"),
		false
	));
}

void SpiralOfFate::MainWindow::removeFrame()
{
	if (this->_object->_schema.framedata[this->_object->_action][this->_object->_actionBlock].size() == 1)
		return;
	this->applyOperation(new RemoveFrameOperation(
		*this->_object,
		this->localize("operation.remove_frame")
	));
}

void SpiralOfFate::MainWindow::removeAnimationBlock()
{
	assert_exp(this->_object->_schema.framedata[this->_object->_action].size() > 1);
	this->applyOperation(new RemoveBlockOperation(
		*this->_object,
		this->localize("operation.remove_block"),
		this->_object->_actionBlock
	));
}

void SpiralOfFate::MainWindow::removeAction()
{
	assert_exp(this->_object->_schema.framedata.size() > 1);
	this->applyOperation(new RemoveMoveOperation(
		*this->_object,
		this->localize("operation.remove_action"),
		this->_object->_action
	));
}

void SpiralOfFate::MainWindow::removeBox()
{
	auto b = this->_preview->getSelectedBox();

	assert_exp(b.first != BOXTYPE_NONE);
	this->applyOperation(new RemoveBoxOperation(
		*this->_object,
		this->localize("operation.remove_box"),
		b.first, b.second,
		[this](BoxType t, unsigned i) { this->_preview->setSelectedBox(t, i); }
	));
}

void SpiralOfFate::MainWindow::copyBoxesFromLastFrame()
{
	auto &blk = this->_object->_schema.framedata[this->_object->_action][this->_object->_actionBlock];

	assert_exp(this->_object->_animation > 0);
	this->applyOperation(new PasteBoxDataOperation(
		*this->_object,
		this->localize("operation.copy_box_last"),
		blk[this->_object->_animation - 1]
	));
}

void SpiralOfFate::MainWindow::copyBoxesFromNextFrame()
{
	auto &blk = this->_object->_schema.framedata[this->_object->_action][this->_object->_actionBlock];

	assert_exp(this->_object->_animation < blk.size() - 1);
	this->applyOperation(new PasteBoxDataOperation(
		*this->_object,
		this->localize("operation.copy_box_last"),
		blk[this->_object->_animation + 1]
	));
}

void SpiralOfFate::MainWindow::flattenThisMoveCollisionBoxes()
{
	auto &f = this->_object->_schema.framedata[this->_object->_action][this->_object->_actionBlock][this->_object->_animation];

	this->applyOperation(new FlattenCollisionBoxesOperation(
		*this->_object,
		this->localize("operation.flatten_collision"),
		f.cBoxes.empty() ? std::optional<ShadyCore::Schema::Sequence::BBox>() : f.cBoxes.front()
	));
}

void SpiralOfFate::MainWindow::flattenThisMoveProperties()
{
	this->applyOperation(new FlattenPropretiesOperation(
		*this->_object,
		this->localize("operation.flatten_properties"),
		this->_object->_schema.framedata[this->_object->_action][this->_object->_actionBlock][this->_object->_animation]
	));
}

void SpiralOfFate::MainWindow::cleanThisCharacterProperties()
{
	this->applyOperation(new CleanPropertiesOperation(
		*this->_object,
		this->localize("operation.flatten_properties")
	));
}

void SpiralOfFate::MainWindow::reloadTextures()
{
	game->textureMgr.reloadEverything();
}

void SpiralOfFate::MainWindow::invertColors()
{
	auto &pal = this->_palettes[this->_selectedPalette];
	std::array<Color, 256> palette = pal.colors;

	for (size_t i = 1; i < 251; i++) {
		palette[i].r = ~palette[i].r;
		palette[i].g = ~palette[i].g;
		palette[i].b = ~palette[i].b;
	}
	this->applyOperation(new EditColorsOperation(
		this->localize("operation.invert_colors"),
		pal, this->_selectedPalette,
		palette
	));
}

void SpiralOfFate::MainWindow::reversePalette()
{
	auto &pal = this->_palettes[this->_selectedPalette];
	std::array<Color, 256> palette;

	palette[0] = pal.colors[0];
	palette[251] = pal.colors[251];
	palette[252] = pal.colors[252];
	palette[253] = pal.colors[253];
	palette[254] = pal.colors[254];
	palette[255] = pal.colors[255];
	for (size_t i = 1; i < 251; i++)
		palette[i] = pal.colors[255 - i];
	this->applyOperation(new EditColorsOperation(
		this->localize("operation.reverse_palette"),
		pal, this->_selectedPalette,
		palette
	));
}

void SpiralOfFate::MainWindow::_rePopulateData()
{
	for (auto key : this->_containers) {
		this->_localizeWidgets(*key, false);
		this->_populateData(*key);
	}
	this->_object->resetState();
	this->_preview->frameChanged();
}

void SpiralOfFate::MainWindow::_rePopulateFrameData()
{
	for (auto key: this->_containers)
		this->_populateFrameData(*key);
}

void SpiralOfFate::MainWindow::_rePopulateColorData()
{
	for (auto key: this->_containers)
		this->_populateColorData(*key);
}

void SpiralOfFate::MainWindow::_populateData(const tgui::Container &container)
{
	auto action = container.get<tgui::EditBox>("ActionID");
	auto actionSelect = container.get<tgui::Button>("ActionSelect");
	auto prevAction = container.get<tgui::Button>("PrevAction");
	auto nextAction = container.get<tgui::Button>("NextAction");
	auto frameSpin = container.get<tgui::SpinButton>("FrameSpin");
	auto frame = container.get<tgui::Slider>("Frame");
	auto ctrl = container.get<tgui::Tabs>("ControlTabs");

	if (action)
		action->setText(std::to_string(this->_object->_action));
	if (actionSelect) {
		std::string name = this->_localizeActionName(this->_object->_action);

		if (name.starts_with('$')) {
			tgui::Color color{"#" + name.substr(1, 6)};

			name = name.substr(7);
			actionSelect->getRenderer()->setTextColor(color);
			actionSelect->getRenderer()->setTextColorHover(color);
			actionSelect->getRenderer()->setTextColorDisabled(color);
			actionSelect->getRenderer()->setTextColorDown(color);
			actionSelect->getRenderer()->setTextColorFocused(color);
		} else
			Utils::setRenderer(actionSelect);
		actionSelect->setText(name + " (" + std::to_string(this->_object->_action) + ")");
	}
	if (prevAction) {
		auto it = this->_object->_schema.framedata.find(this->_object->_action);

		if (it != this->_object->_schema.framedata.begin()) {
			auto prev = std::prev(it)->first;
			std::string name = this->_localizeActionName(prev);

			if (name.starts_with('$')) {
				tgui::Color color{"#" + name.substr(1, 6)};

				name = name.substr(7);
				prevAction->getRenderer()->setTextColor(color);
				prevAction->getRenderer()->setTextColorHover(color);
				prevAction->getRenderer()->setTextColorDisabled(color);
				prevAction->getRenderer()->setTextColorDown(color);
				prevAction->getRenderer()->setTextColorFocused(color);
			} else
				Utils::setRenderer(prevAction);
			prevAction->setText(name + " (" + std::to_string(prev) + ")");
			prevAction->setEnabled(true);
		} else {
			prevAction->setText(this->localize("animation.no_prev_action"));
			prevAction->setEnabled(false);
		}
	}
	if (nextAction) {
		auto it = this->_object->_schema.framedata.find(this->_object->_action);
		assert_exp(it != this->_object->_schema.framedata.end());
		auto next = std::next(it);

		if (next != this->_object->_schema.framedata.end()) {
			std::string name = this->_localizeActionName(next->first);

			if (name.starts_with('$')) {
				tgui::Color color{"#" + name.substr(1, 6)};

				name = name.substr(7);
				nextAction->getRenderer()->setTextColor(color);
				nextAction->getRenderer()->setTextColorHover(color);
				nextAction->getRenderer()->setTextColorDisabled(color);
				nextAction->getRenderer()->setTextColorDown(color);
				nextAction->getRenderer()->setTextColorFocused(color);
			} else
				Utils::setRenderer(nextAction);
			nextAction->setText(name + " (" + std::to_string(next->first) + ")");
			nextAction->setEnabled(true);
		} else {
			nextAction->setText(this->localize("animation.no_next_action"));
			nextAction->setEnabled(false);
		}
	}
	if (frameSpin) {
		frameSpin->onValueChange.setEnabled(false);
		frameSpin->setMinimum(0);
		frameSpin->setMaximum(this->_object->_schema.framedata.at(this->_object->_action).at(this->_object->_actionBlock).size() - 1);
		frameSpin->onValueChange.setEnabled(true);
	}
	if (frame) {
		frame->onValueChange.setEnabled(false);
		frame->setMinimum(0);
		frame->setMaximum(this->_object->_schema.framedata.at(this->_object->_action).at(this->_object->_actionBlock).size() - 1);
		frame->onValueChange.setEnabled(true);
	}
	if (ctrl) {
		ctrl->onTabSelect.setEnabled(false);
		ctrl->select(this->_showingPalette);
		ctrl->onTabSelect.setEnabled(true);
	}

	this->_populateFrameData(container);
	this->_populateColorData(container);
}

void SpiralOfFate::MainWindow::_populateColorData(const tgui::Container &container)
{
	if (auto removePal = container.get<tgui::Button>("RemovePalette"))
		removePal->setEnabled(this->_palettes.size() > 1 && !this->_palettes[this->_selectedPalette].path.empty());

	if (this->_palettes.empty())
		return;

	auto mode = container.get<tgui::Tabs>("ColorModes");
	auto plane = container.get<ColorPlaneWidget>("ColorPlane");
	auto slider = container.get<ColorSliderWidget>("ColorSlider");
	auto comp1 = container.get<tgui::EditBox>("ColorComponent1");
	auto comp2 = container.get<tgui::EditBox>("ColorComponent2");
	auto comp3 = container.get<tgui::EditBox>("ColorComponent3");
	auto hexcode = container.get<tgui::EditBox>("RGBColor");
	auto paletteList = container.get<tgui::ComboBox>("PaletteList");
	auto preview = container.get<tgui::Label>("ColorPreview");
	auto colorIndex = container.get<tgui::Label>("ColorIndex");
	auto colorPanel = container.get<tgui::ScrollablePanel>("AllColorPanel");
	auto comp1lab = container.get<tgui::Label>("ColorComponentName1");
	auto comp2lab = container.get<tgui::Label>("ColorComponentName2");
	auto comp3lab = container.get<tgui::Label>("ColorComponentName3");
	auto color = this->_palettes[this->_selectedPalette].colors[this->_selectedColor];
	auto colorConv = colorConversionsReverse[this->_selectColorMethod](color);

	if (plane && this->_colorChangeSource == 0)
		colorConv = plane->getColor();
	if (slider && this->_colorChangeSource == 1)
		colorConv = slider->getColor();
	if (comp1 && this->_colorChangeSource == 2)
		colorConv[0] = std::stoul(comp1->getText().toStdString());
	if (comp2 && this->_colorChangeSource == 3)
		colorConv[1] = std::stoul(comp2->getText().toStdString());
	if (comp3 && this->_colorChangeSource == 4)
		colorConv[2] = std::stoul(comp3->getText().toStdString());
	if (mode) {
		mode->onTabSelect.setEnabled(false);
		mode->removeAll();
		for (size_t i = 0; i < std::size(colorConversions); i++)
			mode->add(this->localize("color.type" + std::to_string(i) + ".name"), this->_selectColorMethod == i);
		mode->onTabSelect.setEnabled(true);
	}
	if (colorPanel) {
		float extra = this->getSize().y - this->getInnerSize().y;
		auto lay = colorPanel->getSizeLayout();
		auto data = colorPanel->getUserData<tgui::String>();

		colorPanel->setSize(lay.x, tgui::Layout("max(" + data + " - " + std::to_string(extra) + ")"));
		for (size_t i = 0; i < 256; i++) {
			auto button = colorPanel->get<tgui::Button>("Color" + std::to_string(i));
			auto render = button->getRenderer();
			auto ccolor = this->_palettes[this->_selectedPalette].colors[i];

			if (this->_selectedColor == i) {
				render->setBorders({2, 2, 2, 2});
				render->setBorderColor(tgui::Color{0, 0, 255});
				render->setBorderColorHover(tgui::Color{100, 100, 255});
				render->setBorderColorDisabled(tgui::Color{200, 200, 255});
				render->setBorderColorFocused(tgui::Color{0, 0, 255});
				render->setBorderColorDown(tgui::Color{0, 0, 255});
				render->setBorderColorDownDisabled(tgui::Color{0, 0, 255});
				render->setBorderColorDownFocused(tgui::Color{0, 0, 255});
				render->setBorderColorDownHover(tgui::Color{0, 0, 255});
			} else if (this->_object->_paletteIndex == static_cast<int>(i)) {
				render->setBorders({2, 2, 2, 2});
				render->setBorderColor(tgui::Color{0, 255, 255});
				render->setBorderColorHover(tgui::Color{100, 255, 255});
				render->setBorderColorDisabled(tgui::Color{200, 255, 255});
				render->setBorderColorFocused(tgui::Color{0, 255, 255});
				render->setBorderColorDown(tgui::Color{0, 255, 255});
				render->setBorderColorDownDisabled(tgui::Color{0, 255, 255});
				render->setBorderColorDownFocused(tgui::Color{0, 255, 255});
				render->setBorderColorDownHover(tgui::Color{0, 255, 255});
			} else {
				render->setBorders({1, 1, 1, 1});
				render->setBorderColor(tgui::Color{0, 0, 0});
				render->setBorderColorHover(tgui::Color{100, 100, 100});
				render->setBorderColorDisabled(tgui::Color{200, 200, 200});
				render->setBorderColorFocused(tgui::Color{0, 0, 255});
				render->setBorderColorDown(tgui::Color{0, 0, 0});
				render->setBorderColorDownDisabled(tgui::Color{0, 0, 0});
				render->setBorderColorDownFocused(tgui::Color{0, 0, 0});
				render->setBorderColorDownHover(tgui::Color{0, 0, 0});
			}

			render->setBackgroundColor(tgui::Color{ccolor.r, ccolor.g, ccolor.b, ccolor.a});
			render->setBackgroundColorHover(tgui::Color{ccolor.r, ccolor.g, ccolor.b, ccolor.a});
			render->setBackgroundColorDisabled(tgui::Color{ccolor.r, ccolor.g, ccolor.b, ccolor.a});
			render->setBackgroundColorFocused(tgui::Color{ccolor.r, ccolor.g, ccolor.b, ccolor.a});
			render->setBackgroundColorDown(tgui::Color{ccolor.r, ccolor.g, ccolor.b, ccolor.a});
			render->setBackgroundColorDownDisabled(tgui::Color{ccolor.r, ccolor.g, ccolor.b, ccolor.a});
			render->setBackgroundColorDownFocused(tgui::Color{ccolor.r, ccolor.g, ccolor.b, ccolor.a});
			render->setBackgroundColorDownHover(tgui::Color{ccolor.r, ccolor.g, ccolor.b, ccolor.a});

			render->setTexture(tgui::Texture{});
			render->setTextureHover(tgui::Texture{});
			render->setTextureDisabled(tgui::Texture{});
			render->setTextureFocused(tgui::Texture{});
			render->setTextureDown(tgui::Texture{});
			render->setTextureDownDisabled(tgui::Texture{});
			render->setTextureDownFocused(tgui::Texture{});
			render->setTextureDownHover(tgui::Texture{});
		}
	}
	if (colorIndex)
		colorIndex->setText("#" + std::to_string(this->_selectedColor));
	if (paletteList) {
		paletteList->onItemSelect.setEnabled(false);
		paletteList->removeAllItems();
		for (auto &s : this->_palettes)
			if (s.modifications != 0)
				paletteList->addItem(s.name + "*");
			else
				paletteList->addItem(s.name);
		paletteList->setSelectedItemByIndex(this->_selectedPalette);
		paletteList->onItemSelect.setEnabled(true);
	}
	if (preview) {
		auto r = preview->getRenderer();

		r->setBorders({1, 1, 1, 1});
		r->setBackgroundColor(sf::Color(color));
	}
	if (plane && this->_colorChangeSource != 0) {
		plane->onChanged.setEnabled(false);
		plane->setColor(colorConv);
		plane->onChanged.setEnabled(true);
	}
	if (slider && this->_colorChangeSource != 1) {
		slider->onChanged.setEnabled(false);
		slider->setColor(colorConv);
		slider->onChanged.setEnabled(true);
	}
	if (comp1 && this->_colorChangeSource != 2) {
		comp1->onTextChange.setEnabled(false);
		comp1->setText(std::to_string(colorConv[0]));
		comp1->onTextChange.setEnabled(true);
	}
	if (comp2 && this->_colorChangeSource != 3) {
		comp2->onTextChange.setEnabled(false);
		comp2->setText(std::to_string(colorConv[1]));
		comp2->onTextChange.setEnabled(true);
	}
	if (comp3 && this->_colorChangeSource != 4) {
		comp3->onTextChange.setEnabled(false);
		comp3->setText(std::to_string(colorConv[2]));
		comp3->onTextChange.setEnabled(true);
	}
	if (comp1lab && this->_colorChangeSource == 255)
		comp1lab->setText(this->localize("color.type" + std::to_string(this->_selectColorMethod) + ".component1"));
	if (comp2lab && this->_colorChangeSource == 255)
		comp2lab->setText(this->localize("color.type" + std::to_string(this->_selectColorMethod) + ".component2"));
	if (comp3lab && this->_colorChangeSource == 255)
		comp3lab->setText(this->localize("color.type" + std::to_string(this->_selectColorMethod) + ".component3"));
	if (hexcode && this->_colorChangeSource != 5) {
		char buffer[8];

		sprintf(buffer, "#%02X%02X%02X", color.r, color.g, color.b);
		hexcode->onTextChange.setEnabled(false);
		hexcode->setText(buffer);
		hexcode->onTextChange.setEnabled(true);
	}
}

void SpiralOfFate::MainWindow::_populateFrameData(const tgui::Container &container)
{
	for (const auto &fct : this->_updateFrameElements[&container])
		fct();

	auto frameLabel = container.get<tgui::Label>("FrameLabel");
	auto frameSpin = container.get<tgui::SpinButton>("FrameSpin");
	auto frame = container.get<tgui::Slider>("Frame");
	auto blockLabel = container.get<tgui::Label>("BlockLabel");
	auto blockSpin = container.get<tgui::SpinButton>("BlockSpin");

	if (blockLabel)
		blockLabel->setText(this->localize(
			blockLabel->getUserData<std::string>(),
			std::to_string(this->_object->_actionBlock),
			std::to_string(this->_object->_schema.framedata.at(this->_object->_action).size() - 1)
		));
	if (blockSpin) {
		blockSpin->onValueChange.setEnabled(false);
		blockSpin->setMinimum(0);
		blockSpin->setMaximum(this->_object->_schema.framedata.at(this->_object->_action).size() - 1);
		blockSpin->setValue(this->_object->_actionBlock);
		blockSpin->onValueChange.setEnabled(true);
	}
	if (frameLabel)
		frameLabel->setText(this->localize(
			frameLabel->getUserData<std::string>(),
			std::to_string(this->_object->_animation),
			std::to_string(this->_object->_schema.framedata.at(this->_object->_action).at(this->_object->_actionBlock).size() - 1)
		));
	if (frame) {
		frame->onValueChange.setEnabled(false);
		frame->setValue(this->_object->_animation);
		frame->onValueChange.setEnabled(true);
	}
	if (frameSpin) {
		frameSpin->onValueChange.setEnabled(false);
		frameSpin->setValue(this->_object->_animation);
		frameSpin->onValueChange.setEnabled(true);
	}
}

void SpiralOfFate::MainWindow::tick()
{
	auto animation = this->_object->_animation;

	if (this->_requireReloadPal) {
		this->_rePopulateColorData();
		this->_requireReloadPal = false;
	}
	if (this->_requireReload) {
		this->_rePopulateData();
		this->_preview->invalidatePalette();
		this->refreshMenuItems();
		this->_requireReload = false;
	}
	this->_timer++;
	if (this->_timer % 2 == 0) {
		if (this->_colorChangeSource != 255) {
			for (auto container : this->_containers)
				this->_populateColorData(*container);
			this->_colorChangeSource = 255;
			this->_preview->invalidatePalette();
		}
	}
	this->_object->tick();
	if (!this->_paused && !this->_paused2)
		this->_object->update();
	if (this->_effectObject) {
		this->_effectObject->tick();
		this->_effectObject->update();
	}
	if (animation != this->_object->_animation) {
		this->_preview->frameChanged();
		for (auto key : this->_containers)
			this->_populateFrameData(*key);

		auto &blk = this->_object->_schema.framedata[this->_object->_action][this->_object->_actionBlock];

		this->_editor.setCanCopyLast(this->_object->_animation > 0);
		this->_editor.setCanCopyNext(this->_object->_animation < blk.size() - 1);
		this->_editor.setHasLastFrame(this->_object->_animation > 0);
		this->_editor.setHasNextFrame(this->_object->_animation < blk.size() - 1);
	}
}

void SpiralOfFate::MainWindow::rendererChanged(const tgui::String &property)
{
	if (property == U"TitleColor") {
		this->_titleColorUnfocusedCached = this->getSharedRenderer()->getTitleColor();
		if (!this->isFocused())
			this->m_titleText.setColor(this->_titleColorUnfocusedCached);
	} else if (property == U"TitleColorFocused") {
		this->_titleColorFocusedCached = this->getSharedRenderer()->getTitleColorFocused();
		if (this->isFocused())
			this->m_titleText.setColor(this->_titleColorFocusedCached);

	} else if (property == U"TitleBarColor") {
		this->_titleBarColorUnfocusedCached = this->getSharedRenderer()->getTitleBarColor();
		if (!this->isFocused())
			this->m_titleBarColorCached = this->_titleBarColorUnfocusedCached;
	} else if (property == U"TitleBarColorFocused") {
		this->_titleBarColorFocusedCached = this->getSharedRenderer()->getTitleBarColorFocused();
		if (this->isFocused())
			this->m_titleBarColorCached = this->_titleBarColorFocusedCached;

	} else if (property == U"CloseButton") {
		this->_closeButtonRendererUnFocusedCached = this->getSharedRenderer()->getCloseButton();
		this->_updateTitleButtons();
	} else if (property == U"MaximizeButton") {
		this->_maximizeButtonRendererUnFocusedCached = this->getSharedRenderer()->getMaximizeButton();
		this->_updateTitleButtons();
	} else if (property == U"MinimizeButton") {
		this->_minimizeButtonRendererUnFocusedCached = this->getSharedRenderer()->getMinimizeButton();
		this->_updateTitleButtons();

	} else if (property == U"CloseButtonFocused") {
		this->_closeButtonRendererFocusedCached = this->getSharedRenderer()->getCloseButtonFocused();
		this->_updateTitleButtons();
	} else if (property == U"MaximizeButtonFocused") {
		this->_maximizeButtonRendererFocusedCached = this->getSharedRenderer()->getMaximizeButtonFocused();
		this->_updateTitleButtons();
	} else if (property == U"MinimizeButtonFocused") {
		this->_minimizeButtonRendererFocusedCached = this->getSharedRenderer()->getMinimizeButtonFocused();
		this->_updateTitleButtons();

	} else if (property == U"TextureTitleBar" || property == U"TextureTitleBarFocused") {
		this->_updateTextureTitleBar();
	} else
		tgui::ChildWindow::rendererChanged(property);
}

void SpiralOfFate::MainWindow::_updateTitleButtons()
{
	std::shared_ptr<tgui::RendererData> closeButtonRenderer;
	std::shared_ptr<tgui::RendererData> maximizeButtonRenderer;
	std::shared_ptr<tgui::RendererData> minimizeButtonRenderer;

	if (this->isFocused()) {
		closeButtonRenderer = this->_closeButtonRendererFocusedCached;
		maximizeButtonRenderer = this->_maximizeButtonRendererFocusedCached;
		minimizeButtonRenderer = this->_minimizeButtonRendererFocusedCached;
	} else {
		closeButtonRenderer = this->_closeButtonRendererUnFocusedCached;
		maximizeButtonRenderer = this->_maximizeButtonRendererUnFocusedCached;
		minimizeButtonRenderer = this->_minimizeButtonRendererUnFocusedCached;
	}

	if (!closeButtonRenderer || (closeButtonRenderer->propertyValuePairs.empty() && !closeButtonRenderer->connectedTheme))
		closeButtonRenderer = tgui::Theme::getDefault()->getRendererNoThrow("Button");
	if (!maximizeButtonRenderer || (maximizeButtonRenderer->propertyValuePairs.empty() && !maximizeButtonRenderer->connectedTheme))
		maximizeButtonRenderer = closeButtonRenderer;
	if (!minimizeButtonRenderer || (minimizeButtonRenderer->propertyValuePairs.empty() && !minimizeButtonRenderer->connectedTheme))
		minimizeButtonRenderer = closeButtonRenderer;
	this->m_closeButton->setRenderer(closeButtonRenderer);
	this->m_maximizeButton->setRenderer(maximizeButtonRenderer);
	this->m_minimizeButton->setRenderer(minimizeButtonRenderer);
	this->updateTitleBarHeight();
}

void SpiralOfFate::MainWindow::_updateTextureTitleBar()
{
	const float oldTitleBarHeight = this->m_titleBarHeightCached;

	if (this->isFocused())
		this->m_spriteTitleBar.setTexture(getSharedRenderer()->getTextureTitleBarFocused());
	else
		this->m_spriteTitleBar.setTexture(getSharedRenderer()->getTextureTitleBar());

	this->m_titleBarHeightCached = getSharedRenderer()->getTitleBarHeight();
	this->updateTitleBarHeight();
	if (oldTitleBarHeight != this->m_titleBarHeightCached) {
		if (this->m_decorationLayoutY && (this->m_decorationLayoutY == this->m_size.y.getRightOperand()))
			this->m_decorationLayoutY->replaceValue(
				this->m_bordersCached.getTop() +
				this->m_bordersCached.getBottom() +
				this->m_titleBarHeightCached +
				this->m_borderBelowTitleBarCached
			);

		// If the title bar changes in height then the inner size will also change
		this->recalculateBoundSizeLayouts();
	}
}

SpiralOfFate::MainWindow::Renderer *SpiralOfFate::MainWindow::getSharedRenderer()
{
	return aurora::downcast<Renderer *>(Widget::getSharedRenderer());
}

const SpiralOfFate::MainWindow::Renderer *SpiralOfFate::MainWindow::getSharedRenderer() const
{
	return aurora::downcast<const Renderer *>(Widget::getSharedRenderer());
}

SpiralOfFate::MainWindow::Renderer *SpiralOfFate::MainWindow::getRenderer()
{
	return aurora::downcast<Renderer *>(Widget::getRenderer());
}

void SpiralOfFate::MainWindow::reloadLabels()
{
	this->_labels.clear();
	this->_effectLabels.clear();
	this->_loadLabelFor(this->_effectLabels, "effect");
	if (this->_object->_schema.isCharacterData) {
		this->_loadLabelFor(this->_labels, "_common");
		if (this->_pathInit && this->_loadLabelFor(this->_labels, this->_character, this->_path.parent_path()))
			return;
		if (this->_loadLabelFor(this->_labels, this->_character, this->_chrPath))
			return;
		this->_loadLabelFor(this->_labels, this->_character);
	} else if (this->_fileName == "stand")
		this->_loadLabelFor(this->_labels, "stand");
	else
		this->_loadLabelFor(this->_labels, this->_character);
}

bool SpiralOfFate::MainWindow::hasPath() const
{
	return this->_pathInit;
}

std::filesystem::path SpiralOfFate::MainWindow::getPath() const
{
	if (!this->_pathInit)
		return ".";
	return this->_path.parent_path();
}

bool SpiralOfFate::MainWindow::hasPaletteChanges() const
{
	return std::ranges::any_of(this->_palettes, [](const Palette &pal) { return pal.modifications == 0; });
}

void SpiralOfFate::MainWindow::reloadPalette(const std::string &folder)
{
	typedef std::tuple<std::string, ShadyCore::FileType, ShadyCore::BasePackageEntry *> Key;
	std::vector<Key> pals;

	this->_palettes.clear();
	for (auto &entry : game->package) {
		if (entry.first.fileType.type != ShadyCore::FileType::TYPE_PALETTE)
			continue;
		if (!entry.first.actualName.starts_with(folder))
			continue;
		if (strchr(entry.first.actualName.data() + folder.size(), '/'))
			continue;
		pals.emplace_back(entry.first.actualName, entry.first.fileType, entry.second);
	}
	std::ranges::sort(pals, [](const Key &a, const Key &b) {
		return std::get<0>(a) < std::get<0>(b);
	});
	for (auto &entry : pals) {
		ShadyCore::Palette pal;
		auto &stream = std::get<2>(entry)->open();

		ShadyCore::getResourceReader(std::get<1>(entry))(&pal, stream);
		std::get<2>(entry)->close(stream);
		this->_palettes.emplace_back();

		auto &p = this->_palettes.back();
		auto &name = std::get<0>(entry);

		p.name = "[P]" + name.substr(name.find_last_of('/') + 1);
		p.modifications = 0;
		pal.unpack();

		auto ptr = reinterpret_cast<uint32_t *>(pal.data);

		for (size_t i = 0 ; i < 256; i++) {
			p.colors[i].r = ptr[i] >> 16 & 0xFF;
			p.colors[i].g = ptr[i] >> 8  & 0xFF;
			p.colors[i].b = ptr[i] >> 0  & 0xFF;
			p.colors[i].a = i == 0 ? 0 : 255;
		}
	}

	for (auto &file : std::filesystem::directory_iterator(game->settings.palettes / this->_character)) {
		auto &path = file.path();
		ShadyCore::Palette pal;
		ShadyCore::FileType::Format format;

		if (path.extension() == ".pal")
			format = ShadyCore::FileType::PALETTE_PAL;
		else if (path.extension() == ".act")
			format = ShadyCore::FileType::PALETTE_ACT;
		else
			continue;

		std::ifstream stream{path, std::ifstream::binary};

		if (!stream)
			continue;
		ShadyCore::getResourceReader({ShadyCore::FileType::TYPE_PALETTE, format})(&pal, stream);
		stream.close();
		this->_palettes.emplace_back();

		auto &p = this->_palettes.back();

		p.name = path.filename().native();
		p.path = path;
		p.modifications = 0;
		pal.unpack();

		auto ptr = reinterpret_cast<uint32_t *>(pal.data);

		for (size_t i = 0 ; i < 256; i++) {
			p.colors[i].r = ptr[i] >> 16 & 0xFF;
			p.colors[i].g = ptr[i] >> 8  & 0xFF;
			p.colors[i].b = ptr[i] >> 0  & 0xFF;
			p.colors[i].a = i == 0 ? 0 : 255;
		}
	}

	if (this->_palettes.empty()) {
		this->_palettes.emplace_back();

		auto &p = this->_palettes.back();

		p.name = "black";
		p.modifications = 0;
		p.colors.fill(Color::Black);
	}
	for (size_t i = 0; i < this->_palettes.size(); i++)
		if (this->_palettes[i].name == "[P]palette000.pal") {
			this->_selectedPalette = i;
			break;
		}
	if (this->_preview)
		this->_preview->setPalette(&this->_palettes[this->_selectedPalette].colors);
}

void SpiralOfFate::MainWindow::keyPressed(const tgui::Event::KeyEvent &event)
{
	if (!event.alt && !event.control && !event.shift && !event.system && event.code == tgui::Event::KeyboardKey::Escape) {
		if (this->_pendingTransaction) {
			this->cancelTransaction();
			this->_requireReload = true;
			// FIXME: Special case for when you are dragging a box.
			//        Right now it will start a transaction that will never be removed
			//        and starting a new one will crash because of the assertion
			this->startTransaction();
		}
	} else
		ChildWindow::keyPressed(event);
}

bool SpiralOfFate::MainWindow::canHandleKeyPress(const tgui::Event::KeyEvent &event)
{
	if (!event.alt && !event.control && !event.shift && !event.system && event.code == tgui::Event::KeyboardKey::Escape)
		return true;
	return ChildWindow::canHandleKeyPress(event);
}

void SpiralOfFate::MainWindow::navToNextFrame()
{
	auto &blk = this->_object->_schema.framedata[this->_object->_action][this->_object->_actionBlock];

	assert_exp(this->_object->_animation < blk.size() - 1);
	this->_object->_animation++;

	this->_paused = true;
	this->_object->resetState();
	this->_preview->frameChanged();
	this->_rePopulateFrameData();
	this->_editor.setCanCopyLast(this->_object->_animation > 0);
	this->_editor.setCanCopyNext(this->_object->_animation < blk.size() - 1);
	this->_editor.setHasLastFrame(this->_object->_animation > 0);
	this->_editor.setHasNextFrame(this->_object->_animation < blk.size() - 1);
}

void SpiralOfFate::MainWindow::navToPrevFrame()
{
	assert_exp(this->_object->_animation > 0);
	this->_object->_animation--;

	auto &blk = this->_object->_schema.framedata[this->_object->_action][this->_object->_actionBlock];

	this->_paused = true;
	this->_object->resetState();
	this->_preview->frameChanged();
	this->_rePopulateFrameData();
	this->_editor.setCanCopyLast(this->_object->_animation > 0);
	this->_editor.setCanCopyNext(this->_object->_animation < blk.size() - 1);
	this->_editor.setHasLastFrame(this->_object->_animation > 0);
	this->_editor.setHasNextFrame(this->_object->_animation < blk.size() - 1);
}

void SpiralOfFate::MainWindow::navToNextBlock()
{
	auto &act = this->_object->_schema.framedata[this->_object->_action];

	assert_exp(this->_object->_actionBlock < act.size() - 1);
	this->_object->_actionBlock++;

	auto &blk = act[this->_object->_actionBlock];

	this->_object->_animation = 0;
	this->_requireReload = true;
	this->_editor.setHasLastBlock(this->_object->_actionBlock > 0);
	this->_editor.setHasNextBlock(this->_object->_actionBlock < act.size() - 1);
	this->_editor.setHasLastFrame(false);
	this->_editor.setHasNextFrame(blk.size() != 1);
	this->_editor.setCanCopyLast(false);
	this->_editor.setCanCopyNext(blk.size() != 1);
}

void SpiralOfFate::MainWindow::navToPrevBlock()
{
	assert_exp(this->_object->_actionBlock > 0);
	this->_object->_actionBlock--;

	auto &act = this->_object->_schema.framedata[this->_object->_action];
	auto &blk = act[this->_object->_actionBlock];

	this->_object->_animation = 0;
	this->_requireReload = true;
	this->_editor.setHasLastBlock(this->_object->_actionBlock > 0);
	this->_editor.setHasNextBlock(this->_object->_actionBlock < act.size() - 1);
	this->_editor.setHasLastFrame(false);
	this->_editor.setHasNextFrame(blk.size() != 1);
	this->_editor.setCanCopyLast(false);
	this->_editor.setCanCopyNext(blk.size() != 1);
}

void SpiralOfFate::MainWindow::navToNextAction()
{
	auto curr = this->_object->_schema.framedata.find(this->_object->_action);
	assert_exp(curr != this->_object->_schema.framedata.end());
	auto it = std::next(curr);
	assert_exp(it != this->_object->_schema.framedata.end());
	auto move = it->first;
	auto &act = this->_object->_schema.framedata[move];
	auto &blk = act.sequences().front();

	curr = this->_object->_schema.framedata.find(move);
	assert_exp(curr != this->_object->_schema.framedata.end());

	this->localizationOverride.clear();
	if (move >= 800)
		this->localizationOverride["animation.player."] = "animation.object.";
	this->_object->_action = move;
	this->_object->_actionBlock = 0;
	this->_object->_animation = 0;
	this->_object->_animationCtr = 0;
	this->_requireReload = true;
	this->_editor.setHasLastAction(curr != this->_object->_schema.framedata.begin());
	this->_editor.setHasNextAction(std::next(curr) != this->_object->_schema.framedata.end());
	this->_editor.setHasLastBlock(false);
	this->_editor.setHasNextBlock(act.size() != 1);
	this->_editor.setHasLastFrame(false);
	this->_editor.setHasNextFrame(blk.size() != 1);
	this->_editor.setCanCopyLast(false);
	this->_editor.setCanCopyNext(blk.size() != 1);
}

void SpiralOfFate::MainWindow::navToPrevAction()
{
	auto it = this->_object->_schema.framedata.find(this->_object->_action);
	assert_exp(it != this->_object->_schema.framedata.begin());
	auto move = std::prev(it)->first;
	auto &act = this->_object->_schema.framedata[move];
	auto &blk = act.sequences().front();

	auto curr = this->_object->_schema.framedata.find(move);
	assert_exp(curr != this->_object->_schema.framedata.end());

	this->localizationOverride.clear();
	if (move >= 800)
		this->localizationOverride["animation.player."] = "animation.object.";
	this->_object->_action = move;
	this->_object->_actionBlock = 0;
	this->_object->_animation = 0;
	this->_object->_animationCtr = 0;
	this->_requireReload = true;
	this->_editor.setHasLastAction(curr != this->_object->_schema.framedata.begin());
	this->_editor.setHasNextAction(std::next(curr) != this->_object->_schema.framedata.end());
	this->_editor.setHasLastBlock(false);
	this->_editor.setHasNextBlock(act.size() != 1);
	this->_editor.setHasLastFrame(false);
	this->_editor.setHasNextFrame(blk.size() != 1);
	this->_editor.setCanCopyLast(false);
	this->_editor.setCanCopyNext(blk.size() != 1);
}

void SpiralOfFate::MainWindow::navGoTo()
{
	this->_createMoveListPopup([this](unsigned move){
		auto curr = this->_object->_schema.framedata.find(move);
		assert_exp(curr != this->_object->_schema.framedata.end());

		auto &act = this->_object->_schema.framedata[move];
		auto &blk = act.sequences().front();

		this->localizationOverride.clear();
		if (move >= 800)
			this->localizationOverride["animation.player."] = "animation.object.";
		this->_object->_action = move;
		this->_object->_actionBlock = 0;
		this->_object->_animation = 0;
		this->_object->_animationCtr = 0;
		this->_requireReload = true;
		this->_editor.setHasLastAction(curr != this->_object->_schema.framedata.begin());
		this->_editor.setHasNextAction(std::next(curr) != this->_object->_schema.framedata.end());
		this->_editor.setHasLastBlock(false);
		this->_editor.setHasNextBlock(act.size() != 1);
		this->_editor.setHasLastFrame(false);
		this->_editor.setHasNextFrame(blk.size() != 1);
		this->_editor.setCanCopyLast(false);
		this->_editor.setCanCopyNext(blk.size() != 1);
	}, this->_object->_action, false);
}

void SpiralOfFate::MainWindow::_reinitSidePanel(tgui::Container &panel)
{
	if (!this->_showingPalette)
		return;

	auto rect = panel.get<tgui::Panel>("ColorSpaceRectPanel");

	if (rect) {
		auto colorPlane = rect->get<ColorPlaneWidget>("ColorPlane");
		auto colorSlider = rect->get<ColorSliderWidget>("ColorSlider");

		colorPlane->setColorSpace(colorConversions[this->_selectColorMethod], colorSpaces[this->_selectColorMethod]);
		colorSlider->setColorSpace(colorConversions[this->_selectColorMethod], colorSpaces[this->_selectColorMethod]);
	}
}

void SpiralOfFate::MainWindow::_initSidePanel(tgui::Container &panel)
{
	if (!this->_showingPalette)
		return;

	auto rect = panel.get<tgui::Panel>("ColorSpaceRectPanel");

	if (rect) {
		rect->removeAllWidgets();

		auto colorPlane = ColorPlaneWidget::create(colorConversions[this->_selectColorMethod], colorSpaces[this->_selectColorMethod], {0, 1}, {"&.w - 30", "&.w - 30"});
		auto colorSlider= ColorSliderWidget::create(colorConversions[this->_selectColorMethod], colorSpaces[this->_selectColorMethod], 2, true, {"20", "&.w - 30"});

		rect->add(colorPlane, "ColorPlane");
		rect->add(colorSlider, "ColorSlider");
		colorPlane->setPosition(0, 0);
		colorSlider->setPosition("ColorPlane.x + ColorPlane.w", 0);
		this->_planes.push_back(colorPlane);
		this->_sliders.push_back(colorSlider);
	}
}

void SpiralOfFate::MainWindow::refreshInterface()
{
	this->_populateData(*this);
}

void SpiralOfFate::MainWindow::mouseMovedAbsolute(tgui::Vector2f pos)
{
	for (auto &widget : this->_planes)
		widget->mouseMovedAbs(pos - widget->getAbsolutePosition());
	for (auto &widget : this->_sliders)
		widget->mouseMovedAbs(pos - widget->getAbsolutePosition());
}

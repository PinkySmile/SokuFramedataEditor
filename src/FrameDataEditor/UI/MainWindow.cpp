//
// Created by PinkySmile on 05/05/25.
//

#include <TGUI/RendererDefines.hpp>
#include "MainWindow.hpp"
#include "PreviewWidget.hpp"
#include "../Operations/FlagOperation.hpp"
#include "../Operations/BasicDataOperation.hpp"
#include "../Operations/SpriteChangeOperation.hpp"
#include "../Operations/SoundChangeOperation.hpp"

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

std::string to_hex(unsigned long long value)
{
	char buffer[17];

	sprintf(buffer, "%016llX", value);
	return buffer;
}

#define PLACE_HOOK_STRUCTURE(container, guiId, field, name, operation, toString, fromStringPre, fromString, arg) \
do {                                                                                                             \
        auto __elem = container.get<tgui::EditBox>(guiId);                                                       \
                                                                                                                 \
        if (!__elem)                                                                                             \
                break;                                                                                           \
        __elem->onTextChange.connect([this](const tgui::String &s){                                              \
                if (s.empty()) return;                                                                           \
                try {                                                                                            \
                        fromStringPre(s)                                                                         \
                        this->applyOperation(new operation(                                                      \
                                *this->_object,                                                                  \
                                name,                                                                            \
                                &FrameData::field,                                                               \
                                fromString(s, decltype(FrameData::field))                                        \
                        ));                                                                                      \
                } catch (...) { return; }                                                                        \
        });                                                                                                      \
        this->_updateFrameElements[&container].emplace_back([__elem, this]{                                      \
                auto &data = this->_object->getFrameData();                                                      \
                __elem->onTextChange.setEnabled(false);                                                          \
                __elem->setText(toString(data.field, arg));                                                      \
                __elem->onTextChange.setEnabled(true);                                                           \
        });                                                                                                      \
} while (false)
#define PLACE_HOOK_OPTIONAL_STRUCTURE(container, guiId, field, name, operation, toString, fromStringPre, fromString, arg) \
do {                                                                                                                      \
        auto __elem = container.get<tgui::EditBox>(guiId);                                                                \
                                                                                                                          \
        if (!__elem)                                                                                                      \
                break;                                                                                                    \
        __elem->onTextChange.connect([this](const tgui::String &s){                                                       \
                if (s.empty()) {                                                                                          \
                        this->applyOperation(new operation(                                                               \
                                *this->_object,                                                                           \
                                name,                                                                                     \
                                &FrameData::field,                                                                        \
                                std::make_optional<decltype(FrameData::field)::value_type>()                              \
                        ));                                                                                               \
                        return;                                                                                           \
                }                                                                                                         \
                try {                                                                                                     \
                        fromStringPre(s)                                                                                  \
                        this->applyOperation(new operation(                                                               \
                                *this->_object,                                                                           \
                                name,                                                                                     \
                                &FrameData::field,                                                                        \
                                std::make_optional<decltype(FrameData::field)::value_type>(                               \
                                        fromString(s, decltype(FrameData::field)::value_type)                             \
                                )                                                                                         \
                        ));                                                                                               \
                } catch (...) { return; }                                                                                 \
        });                                                                                                               \
        this->_updateFrameElements[&container].emplace_back([__elem, this]{                                               \
                auto &data = this->_object->getFrameData();                                                               \
                __elem->onTextChange.setEnabled(false);                                                                   \
                if (data.field)                                                                                           \
                        __elem->setText(toString(*data.field, arg));                                                      \
                else                                                                                                      \
                        __elem->setText("");                                                                              \
                __elem->onTextChange.setEnabled(true);                                                                    \
        });                                                                                                               \
} while (false)

#define NO_FROMSTRING_PRE(s)

#define STRING_FROM_STRING(s, _) s.toStdString()
#define STRING_TO_STRING(s, _) s

#define NUMBER_FROM_STRING(s, type) static_cast<type>(std::stof(s.toStdString()))
#define NUMBER_TO_STRING(s, p) to_string(s, p)

#define NUMFLAGS_FROM_STRING(s, _) { (unsigned int)std::stoul(s.toStdString(), nullptr, 16) }
#define NUMFLAGS_TO_STRING(s, _) to_hex(s.flags)

#define VECTOR_FROM_STRING_PRE(s)                                       \
        auto pos = s.find(',');                                         \
        auto __x = s.substr(1, pos - 1).toStdString();                  \
        auto __y = s.substr(pos + 1, s.size() - pos - 1).toStdString();
#define VECTOR_FROM_STRING(s, type) {       \
        (decltype(type::x))std::stof(__x), \
        (decltype(type::y))std::stof(__y)  \
}
#define VECTOR_TO_STRING(s, p) "(" + \
	to_string((s).x, p) + "," +  \
	to_string((s).y, p) +        \
")"

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
#define RECT_FROM_STRING(s, type) {                                                       \
        {(decltype(type::pos.x))std::stof(__x),  (decltype(type::pos.y))std::stof(__y) }, \
        {(decltype(type::size.x))std::stof(__w), (decltype(type::size.y))std::stof(__h) } \
}
#define RECT_TO_STRING(s, p) "(" +     \
        to_string(s.pos.x, p) + "," +  \
        to_string(s.pos.y, p) + "," +  \
        to_string(s.size.x, p) + "," + \
        to_string(s.size.y, p) +       \
")"

#define PLACE_HOOK_STRING(container, guiId, field, name, operation) \
	PLACE_HOOK_STRUCTURE(container, guiId, field, name, operation, STRING_TO_STRING, NO_FROMSTRING_PRE, STRING_FROM_STRING, _)
#define PLACE_HOOK_NUMBER(container, guiId, field, name, operation, precision) \
	PLACE_HOOK_STRUCTURE(container, guiId, field, name, operation, NUMBER_TO_STRING, NO_FROMSTRING_PRE, NUMBER_FROM_STRING, precision)
#define PLACE_HOOK_OPTIONAL_NUMBER(container, guiId, field, name, operation, precision) \
	PLACE_HOOK_OPTIONAL_STRUCTURE(container, guiId, field, name, operation, NUMBER_TO_STRING, NO_FROMSTRING_PRE, NUMBER_FROM_STRING, precision)
#define PLACE_HOOK_NUMFLAGS(container, guiId, field, name, operation) \
	PLACE_HOOK_STRUCTURE(container, guiId, field, name, operation, NUMFLAGS_TO_STRING, NO_FROMSTRING_PRE, NUMFLAGS_FROM_STRING, _)
#define PLACE_HOOK_VECTOR(container, guiId, field, name, operation, precision) \
	PLACE_HOOK_STRUCTURE(container, guiId, field, name, operation, VECTOR_TO_STRING, VECTOR_FROM_STRING_PRE, VECTOR_FROM_STRING, precision)
#define PLACE_HOOK_OPTIONAL_VECTOR(container, guiId, field, name, operation, precision) \
	PLACE_HOOK_OPTIONAL_STRUCTURE(container, guiId, field, name, operation, VECTOR_TO_STRING, VECTOR_FROM_STRING_PRE, VECTOR_FROM_STRING, precision)
#define PLACE_HOOK_RECT(container, guiId, field, name, operation) \
	PLACE_HOOK_STRUCTURE(container, guiId, field, name, operation, RECT_TO_STRING, RECT_FROM_STRING_PRE, RECT_FROM_STRING, 0)
#define PLACE_HOOK_FLAG(container, guiId, field, index, name)                      \
do {                                                                               \
        auto __elem = container.get<tgui::CheckBox>(guiId);                        \
                                                                                   \
        if (!__elem)                                                               \
                break;                                                             \
        __elem->onChange.connect([this, index](bool b){                            \
                this->applyOperation(new FlagOperation(                            \
                        *this->_object,                                            \
                        name,                                                      \
                        &FrameData::field,                                         \
                        index, b                                                   \
                ));                                                                \
                for (auto &[key, _] : this->_updateFrameElements)                  \
                        this->_populateFrameData(*key);                            \
        });                                                                        \
        this->_updateFrameElements[&container].emplace_back([__elem, this, index]{ \
                auto &data = this->_object->getFrameData();                        \
                __elem->onChange.setEnabled(false);                                \
                __elem->setChecked(data.field.flags & (1ULL << index));            \
                __elem->onChange.setEnabled(true);                                 \
        });                                                                        \
} while (false)


TGUI_RENDERER_PROPERTY_COLOR(SpiralOfFate::MainWindow::Renderer, TitleColorFocused, tgui::Color::Black)
TGUI_RENDERER_PROPERTY_COLOR(SpiralOfFate::MainWindow::Renderer, TitleBarColorFocused, tgui::Color::White)
TGUI_RENDERER_PROPERTY_TEXTURE(SpiralOfFate::MainWindow::Renderer, TextureTitleBarFocused)
TGUI_RENDERER_PROPERTY_RENDERER(SpiralOfFate::MainWindow::Renderer, CloseButtonFocused, "ChildWindowButton")
TGUI_RENDERER_PROPERTY_RENDERER(SpiralOfFate::MainWindow::Renderer, MaximizeButtonFocused, "ChildWindowButton")
TGUI_RENDERER_PROPERTY_RENDERER(SpiralOfFate::MainWindow::Renderer, MinimizeButtonFocused, "ChildWindowButton")

SpiralOfFate::MainWindow::MainWindow(const std::string &frameDataPath, const FrameDataEditor &editor) :
	LocalizedContainer<tgui::ChildWindow>(editor, MainWindow::StaticWidgetType, false),
	_path(frameDataPath),
	_character(std::filesystem::path(frameDataPath).parent_path().filename().string()),
	_object(new EditableObject(frameDataPath))
{
	auto preview = std::make_shared<PreviewWidget>(*this->_object);

	preview->setPosition(0, 0);
	preview->setSize("&.w", "&.h");

	this->m_renderer = aurora::makeCopied<Renderer>();
	this->loadLocalizedWidgetsFromFile("assets/gui/editor/character/animationWindow.gui");
	Utils::setRenderer(this);
	this->setSize(1200, 600);
	this->setPosition(10, 30);
	this->setTitleButtons(TitleButton::Minimize | TitleButton::Maximize | TitleButton::Close);
	this->setTitle(frameDataPath);
	this->setResizable();
	this->get<tgui::Panel>("AnimationPanel")->add(preview);

	this->onFocus.connect([this]{
		this->m_titleText.setColor(this->_titleColorFocusedCached);
		this->m_titleBarColorCached = this->_titleBarColorFocusedCached;
		this->_updateTextureTitleBar();
		this->_updateTitleButtons();
	});
	this->onUnfocus.connect([this]{
		this->m_titleText.setColor(this->_titleColorUnfocusedCached);
		this->m_titleBarColorCached = this->_titleBarColorUnfocusedCached;
		this->_updateTextureTitleBar();
		this->_updateTitleButtons();
	});

	this->_placeUIHooks(*this);
	this->_populateData(*this);
}

bool SpiralOfFate::MainWindow::isModified() const noexcept
{
	return this->_operationIndex != this->_operationSaved;
}

void SpiralOfFate::MainWindow::redo()
{
	if (this->_operationIndex == this->_operationQueue.size())
		return;
	this->_operationQueue[this->_operationIndex]->apply();
	this->_operationIndex++;
	for (auto &[key, _] : this->_updateFrameElements)
		this->_populateData(*key);
	if (this->isModified())
		this->setTitle(this->_path + "*");
	else
		this->setTitle(this->_path);
}

void SpiralOfFate::MainWindow::undo()
{
	if (this->_operationIndex == 0)
		return;
	this->_operationIndex--;
	this->_operationQueue[this->_operationIndex]->undo();
	for (auto &[key, _] : this->_updateFrameElements)
		this->_populateData(*key);
	if (this->isModified())
		this->setTitle(this->_path + "*");
	else
		this->setTitle(this->_path);
}

void SpiralOfFate::MainWindow::applyOperation(IOperation *operation)
{
	this->_operationQueue.erase(this->_operationQueue.begin() + this->_operationIndex, this->_operationQueue.end());
	this->_operationQueue.emplace_back(operation);
	this->_operationQueue.back()->apply();
	if (this->_operationIndex < this->_operationSaved)
		this->_operationSaved = -1;
	this->_operationIndex = this->_operationQueue.size();
	this->setTitle(this->_path + "*");
}

void SpiralOfFate::MainWindow::save(const std::string &path)
{
	this->_path = path;
	this->save();
}

void SpiralOfFate::MainWindow::save()
{
	nlohmann::json j = nlohmann::json::array();

	for (auto &[key, value] : this->_object->_moves) {
		j.push_back({
			{"action", key},
			{"framedata", value}
		});
	}

	std::ofstream stream{this->_path};

	if (stream.fail()) {
		SpiralOfFate::Utils::dispMsg(game->gui, "Saving failed", this->_path + ": " + strerror(errno), MB_ICONERROR);
		return;
	}
	stream << j.dump(2);
	this->_operationSaved = this->_operationIndex;
	this->setTitle(this->_path);
}

std::string SpiralOfFate::MainWindow::_localizeActionName(unsigned int id)
{
	if (this->_editor.hasLocalization("action." + this->_character + "." + std::to_string(id)))
		return this->_editor.localize("action." + this->_character + "." + std::to_string(id));
	else if (this->_editor.hasLocalization("action.generic." + std::to_string(id)))
		return this->_editor.localize("action.generic." + std::to_string(id));
	return Character::actionToString(id);
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
	};
	auto data = this->_object->getFrameData();

	contentPanel->loadLocalizedWidgetsFromFile(path);
	for (auto &w : contentPanel->getWidgets()) {
		size.x = std::max(size.x, w->getFullSize().x + w->getPosition().x + 20);
		size.y = std::max(size.y, w->getFullSize().y + w->getPosition().y + 20);
	}
	contentPanel->setSize(size);
	Utils::setRenderer(contentPanel->cast<tgui::Container>());
	outsidePanel->onClick.connect(closePopup, std::weak_ptr(outsidePanel), std::weak_ptr(contentPanel));
	this->_placeUIHooks(*contentPanel);
	this->_populateData(*contentPanel);
}

void SpiralOfFate::MainWindow::_createMoveListPopup()
{
	auto outsidePanel = tgui::Panel::create({"100%", "100%"});
	auto contentPanel = tgui::ScrollablePanel::create({500, "&.h - 100"});
	unsigned i = 0;
	auto scroll = 0;

	outsidePanel->getRenderer()->setBackgroundColor({0, 0, 0, 175});
	outsidePanel->setUserData(false);
	this->add(outsidePanel);

	contentPanel->setPosition("(&.w - w) / 2", "(&.h - h) / 2");
	this->add(contentPanel);

	auto closePopup = [this](std::weak_ptr<tgui::Panel> outsidePanel, std::weak_ptr<tgui::ScrollablePanel> contentPanel){
		this->remove(outsidePanel.lock());
		this->remove(contentPanel.lock());
	};

	outsidePanel->onClick.connect(closePopup, std::weak_ptr(outsidePanel), std::weak_ptr(contentPanel));

	for (auto &[moveId, _] : this->_object->_moves) {
		auto name = this->_localizeActionName(moveId);
		auto label = tgui::Label::create(std::to_string(moveId));
		auto button = tgui::Button::create(name);

		label->setPosition(10, i * 25 + 12);
		button->setPosition(50, i * 25 + 10);
		button->setSize(410, 20);
		Utils::setRenderer(button);
		Utils::setRenderer(label);
		if (moveId == this->_object->_action) {
			scroll = i * 25 + 20;
			button->getRenderer()->setTextColor(tgui::Color::Green);
			button->getRenderer()->setTextColorHover(tgui::Color{0x40, 0xFF, 0x40});
			button->getRenderer()->setTextColorDisabled(tgui::Color{0x00, 0xA0, 0x00});
			button->getRenderer()->setTextColorDown(tgui::Color{0x00, 0x80, 0x00});
			button->getRenderer()->setTextColorFocused(tgui::Color{0x20, 0x80, 0x20});
		} else if (name.rfind("Action #", 0) == 0) {
			button->getRenderer()->setTextColor(tgui::Color{0xFF, 0x80, 0x00});
			button->getRenderer()->setTextColorHover(tgui::Color{0xFF, 0xA0, 0x40});
			button->getRenderer()->setTextColorDisabled(tgui::Color{0xA0, 0x50, 0});
			button->getRenderer()->setTextColorDown(tgui::Color{0x80, 0x40, 0x00});
			button->getRenderer()->setTextColorFocused(tgui::Color{0x80, 0x60, 0x20});
		}

		button->onClick.connect([this](unsigned move){
			this->_object->_action = move;
			this->_object->_actionBlock = 0;
			this->_object->_animation = 0;
			this->_object->_animationCtr = 0;
			for (auto &[key, _] : this->_updateFrameElements)
				this->_populateData(*key);
		}, moveId);
		button->onClick.connect(closePopup, std::weak_ptr(outsidePanel), std::weak_ptr(contentPanel));

		contentPanel->add(label);
		contentPanel->add(button);
		i++;
	}
	scroll -= contentPanel->getSize().y / 2;

	auto label = tgui::Label::create("");

	label->setPosition(10, i * 25 + 10);
	label->setSize(100, 10);
	label->setTextSize(1);
	contentPanel->add(label);
	if (scroll > 0)
		contentPanel->getVerticalScrollbar()->setValue(scroll);
}

void SpiralOfFate::MainWindow::_placeUIHooks(const tgui::Container &container)
{
	auto action = container.get<tgui::EditBox>("ActionID");
	auto actionSelect = container.get<tgui::Button>("ActionSelect");
	auto play = container.get<tgui::Button>("Play");
	auto pause = container.get<tgui::Button>("Pause");
	auto frame = container.get<tgui::Slider>("Frame");
	auto blockSpin = container.get<tgui::SpinButton>("BlockSpin");
	auto frameSpin = container.get<tgui::SpinButton>("FrameSpin");
	auto aFlags = container.get<tgui::Button>("AFlagsButton");
	auto dFlags = container.get<tgui::Button>("DFlagsButton");
	auto hitEdit = container.get<tgui::Button>("HitEdit");
	auto blockEdit = container.get<tgui::Button>("BlockEdit");
	auto generalEdit = container.get<tgui::Button>("GeneralEdit");

	if (action)
		action->onReturnOrUnfocus.connect([this](std::weak_ptr<tgui::EditBox> This, const tgui::String &s){
			if (std::to_string(this->_object->_action) == s)
				return;
			if (s.empty()) {
				This.lock()->setText(std::to_string(this->_object->_action));
				return;
			}

			unsigned action = std::stoul(s.toStdString());

			if (this->_object->_moves.count(action) == 0) {
				This.lock()->setText(std::to_string(this->_object->_action));
				return;
			}
			this->_object->_action = action;
			this->_object->_actionBlock = 0;
			this->_object->_animation = 0;
			this->_object->_animationCtr = 0;
			for (auto &[key, _] : this->_updateFrameElements)
				this->_populateData(*key);
		}, std::weak_ptr(action));
	if (aFlags)
		// TODO: character/subobject
		aFlags->onClick.connect(&MainWindow::_createGenericPopup, this, "assets/gui/editor/character/attackFlags.gui");
	if (dFlags)
		// TODO: character/subobject
		dFlags->onClick.connect(&MainWindow::_createGenericPopup, this, "assets/gui/editor/character/defenseFlags.gui");
	if (hitEdit)
		// TODO: character/subobject
		hitEdit->onClick.connect(&MainWindow::_createGenericPopup, this, "assets/gui/editor/character/hitProperties.gui");
	if (blockEdit)
		// TODO: character/subobject
		blockEdit->onClick.connect(&MainWindow::_createGenericPopup, this, "assets/gui/editor/character/blockProperties.gui");
	if (generalEdit)
		// TODO: character/subobject
		generalEdit->onClick.connect(&MainWindow::_createGenericPopup, this, "assets/gui/editor/character/generalProperties.gui");
	if (actionSelect)
		actionSelect->onClick.connect(&MainWindow::_createMoveListPopup, this);
	if (play)
		play->onPress.connect([this]{ this->_paused = false; });
	if (pause)
		pause->onPress.connect([this]{ this->_paused = true; });
	if (frame)
		frame->onValueChange.connect([this](float value){
			this->_paused = true;
			this->_object->_animation = value;
			for (auto &[key, _] : this->_updateFrameElements)
				this->_populateFrameData(*key);
		});
	if (frameSpin)
		frameSpin->onValueChange.connect([this](float value){
			this->_paused = true;
			this->_object->_animation = value;
			for (auto &[key, _] : this->_updateFrameElements)
				this->_populateFrameData(*key);
		});
	if (blockSpin)
		blockSpin->onValueChange.connect([this](float value){
			this->_object->_actionBlock = value;
			this->_object->_animation = 0;
			for (auto &[key, _] : this->_updateFrameElements)
				this->_populateData(*key);
		});

	PLACE_HOOK_STRING(container,   "Sprite",   spritePath,    this->_editor.localize("animation.sprite"),   SpriteChangeOperation);
	PLACE_HOOK_NUMBER(container,   "Duration", duration,      this->_editor.localize("animation.duration"), BasicDataOperation, 0);
	PLACE_HOOK_VECTOR(container,   "Offset",   offset,        this->_editor.localize("animation.offset"),   BasicDataOperation, 0);
	PLACE_HOOK_VECTOR(container,   "Scale",    scale,         this->_editor.localize("animation.scale"),    BasicDataOperation, 2);
	PLACE_HOOK_RECT(container,     "Bounds",   textureBounds, this->_editor.localize("animation.bounds"),   BasicDataOperation);
	PLACE_HOOK_NUMFLAGS(container, "AFlags",   oFlag,         this->_editor.localize("animation.aflags"),   BasicDataOperation);
	PLACE_HOOK_NUMFLAGS(container, "DFlags",   dFlag,         this->_editor.localize("animation.dflags"),   BasicDataOperation);
	PLACE_HOOK_NUMBER(container,   "Duration", duration,      this->_editor.localize("animation.duration"), BasicDataOperation, 0);

	PLACE_HOOK_NUMBER(container,          "PGen",      particleGenerator, this->_editor.localize("animation.general.partgenerator"), BasicDataOperation, 0);
	PLACE_HOOK_NUMBER(container,          "SubObj",    subObjectSpawn,    this->_editor.localize("animation.general.subobj"),        BasicDataOperation, 0);
	PLACE_HOOK_NUMBER(container,          "Marker",    specialMarker,     this->_editor.localize("animation.general.marker"),        BasicDataOperation, 0);
	PLACE_HOOK_OPTIONAL_NUMBER(container, "Tier",      priority,          this->_editor.localize("animation.general.tier"),          BasicDataOperation, 0);
	PLACE_HOOK_VECTOR(container,          "MoveSpeed", speed,             this->_editor.localize("animation.general.speed"),         BasicDataOperation, 2);
	PLACE_HOOK_OPTIONAL_VECTOR(container, "Gravity",   gravity,           this->_editor.localize("animation.general.gravity"),       BasicDataOperation, 2);
	PLACE_HOOK_STRING(container,          "Sound",     soundPath,         this->_editor.localize("animation.general.sound"),         SoundChangeOperation);
	PLACE_HOOK_NUMBER(container,          "FadeTime",  fadeTime,          this->_editor.localize("animation.general.fadetime"),      BasicDataOperation, 0);

	for (size_t i = 0; i < 64; i++)
		PLACE_HOOK_FLAG(container, "aFlag" + std::to_string(i), oFlag, i, this->_editor.localize("animation.aflags.flag" + std::to_string(i)));
	for (size_t i = 0; i < 64; i++)
		PLACE_HOOK_FLAG(container, "dFlag" + std::to_string(i), dFlag, i, this->_editor.localize("animation.dflags.flag" + std::to_string(i)));
}

void SpiralOfFate::MainWindow::_populateData(const tgui::Container &container)
{
	auto action = container.get<tgui::EditBox>("ActionID");
	auto actionSelect = container.get<tgui::Button>("ActionSelect");
	auto block = container.get<tgui::Label>("BlockLabel");
	auto blockSpin = container.get<tgui::SpinButton>("BlockSpin");
	auto frameSpin = container.get<tgui::SpinButton>("FrameSpin");
	auto frame = container.get<tgui::Slider>("Frame");

	if (action)
		action->setText(std::to_string(this->_object->_action));
	if (actionSelect)
		actionSelect->setText(this->_localizeActionName(this->_object->_action));
	if (block)
		block->setText(this->_editor.localize(
			block->getUserData<std::string>(),
			std::to_string(this->_object->_actionBlock),
			std::to_string(this->_object->_moves.at(this->_object->_action).size() - 1)
		));
	if (blockSpin) {
		blockSpin->onValueChange.setEnabled(false);
		blockSpin->setMinimum(0);
		blockSpin->setMaximum(this->_object->_moves.at(this->_object->_action).size() - 1);
		blockSpin->setValue(this->_object->_actionBlock);
		blockSpin->onValueChange.setEnabled(true);
	}
	if (frameSpin) {
		frameSpin->onValueChange.setEnabled(false);
		frameSpin->setMinimum(0);
		frameSpin->setMaximum(this->_object->_moves.at(this->_object->_action).at(this->_object->_actionBlock).size() - 1);
		frameSpin->onValueChange.setEnabled(true);
	}
	if (frame) {
		frame->onValueChange.setEnabled(false);
		frame->setMinimum(0);
		frame->setMaximum(this->_object->_moves.at(this->_object->_action).at(this->_object->_actionBlock).size() - 1);
		frame->onValueChange.setEnabled(true);
	}

	this->_populateFrameData(container);
}

void SpiralOfFate::MainWindow::_populateFrameData(const tgui::Container &container)
{
	for (const auto &pair : this->_updateFrameElements[&container])
		pair();

	auto frameLabel = container.get<tgui::Label>("FrameLabel");
	auto frameSpin = container.get<tgui::SpinButton>("FrameSpin");
	auto frame = container.get<tgui::Slider>("Frame");

	if (frameLabel)
		frameLabel->setText(this->_editor.localize(
			frameLabel->getUserData<std::string>(),
			std::to_string(this->_object->_animation),
			std::to_string(this->_object->_moves.at(this->_object->_action).at(this->_object->_actionBlock).size() - 1)
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

	if (!this->_paused)
		this->_object->update();
	if (animation != this->_object->_animation)
		for (auto &[key, _] : this->_updateFrameElements)
			this->_populateFrameData(*key);
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
	if (oldTitleBarHeight != m_titleBarHeightCached) {
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
	return aurora::downcast<SpiralOfFate::MainWindow::Renderer*>(Widget::getSharedRenderer());
}

const SpiralOfFate::MainWindow::Renderer *SpiralOfFate::MainWindow::getSharedRenderer() const
{
	return aurora::downcast<const SpiralOfFate::MainWindow::Renderer*>(Widget::getSharedRenderer());
}

SpiralOfFate::MainWindow::Renderer *SpiralOfFate::MainWindow::getRenderer()
{
	return aurora::downcast<SpiralOfFate::MainWindow::Renderer*>(Widget::getRenderer());
}

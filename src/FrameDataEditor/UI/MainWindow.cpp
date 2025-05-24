//
// Created by PinkySmile on 05/05/25.
//

#include <TGUI/RendererDefines.hpp>
#include "MainWindow.hpp"
#include "../Operations/DummyOperation.hpp"
#include "../Operations/FlagOperation.hpp"
#include "../Operations/BasicDataOperation.hpp"
#include "../Operations/SpriteChangeOperation.hpp"
#include "../Operations/SoundChangeOperation.hpp"
#include "../Operations/ClearBlockOperation.hpp"
#include "../Operations/ClearHitOperation.hpp"
#include "../Operations/CreateMoveOperation.hpp"
#include "../Operations/CreateFrameOperation.hpp"
#include "../Operations/CreateBlockOperation.hpp"

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

#define PLACE_HOOK_STRUCTURE(container, guiId, field, name, operation, toString, fromStringPre, fromString, arg, reset) \
do {                                                                                                             \
        auto __elem = container.get<tgui::EditBox>(guiId);                                                       \
                                                                                                                 \
        if (!__elem)                                                                                             \
                break;                                                                                           \
        __elem->onFocus.connect([this]{ this->startTransaction(); });                                            \
        __elem->onUnfocus.connect([this]{ this->commitTransaction(); });                                         \
        __elem->onReturnKeyPress.connect([this]{ this->commitTransaction(); this->startTransaction(); });        \
        __elem->onTextChange.connect([this](const tgui::String &s){                                              \
                if (s.empty()) return;                                                                           \
                try {                                                                                            \
                        fromStringPre(s)                                                                         \
                        this->updateTransaction([&]{ return new operation(                                       \
                                *this->_object,                                                                  \
                                name,                                                                            \
                                &FrameData::field,                                                               \
                                fromString(s, decltype(FrameData::field)),                                       \
                                reset                                                                            \
                        ); });                                                                                   \
                } catch (...) { return; }                                                                        \
        });                                                                                                      \
        this->_updateFrameElements[&container].emplace_back([__elem, this]{                                      \
                auto &data = this->_object->getFrameData();                                                      \
                __elem->onTextChange.setEnabled(false);                                                          \
                __elem->setText(toString(data.field, arg));                                                      \
                __elem->onTextChange.setEnabled(true);                                                           \
        });                                                                                                      \
} while (false)
#define PLACE_HOOK_OPTIONAL_STRUCTURE(container, guiId, field, name, operation, toString, fromStringPre, fromString, arg, reset) \
do {                                                                                                                             \
        auto __elem = container.get<tgui::EditBox>(guiId);                                                                       \
                                                                                                                                 \
        if (!__elem)                                                                                                             \
                break;                                                                                                           \
        __elem->onFocus.connect([this]{ this->startTransaction(); });                                                            \
        __elem->onUnfocus.connect([this]{ this->commitTransaction(); });                                                         \
        __elem->onReturnKeyPress.connect([this]{ this->commitTransaction(); this->startTransaction(); });                        \
        __elem->onTextChange.connect([this](const tgui::String &s){                                                              \
                if (s.empty()) {                                                                                                 \
                        this->updateTransaction([&]{ return new operation(                                                       \
                                *this->_object,                                                                                  \
                                name,                                                                                            \
                                &FrameData::field,                                                                               \
                                std::make_optional<decltype(FrameData::field)::value_type>(),                                    \
                                (reset)                                                                                          \
                        ); });                                                                                                   \
                        return;                                                                                                  \
                }                                                                                                                \
                try {                                                                                                            \
                        fromStringPre(s)                                                                                         \
                        this->updateTransaction([&]{ return new operation(                                                       \
                                *this->_object,                                                                                  \
                                name,                                                                                            \
                                &FrameData::field,                                                                               \
                                std::make_optional<decltype(FrameData::field)::value_type>(                                      \
                                        fromString(s, decltype(FrameData::field)::value_type)                                    \
                                ),                                                                                               \
                                (reset)                                                                                          \
                        ); });                                                                                                   \
                } catch (...) { return; }                                                                                        \
        });                                                                                                                      \
        this->_updateFrameElements[&container].emplace_back([__elem, this]{                                                      \
                auto &data = this->_object->getFrameData();                                                                      \
                __elem->onTextChange.setEnabled(false);                                                                          \
                if (data.field)                                                                                                  \
                        __elem->setText(toString(*data.field, arg));                                                             \
                else                                                                                                             \
                        __elem->setText("");                                                                                     \
                __elem->onTextChange.setEnabled(true);                                                                           \
        });                                                                                                                      \
} while (false)

#define NO_FROMSTRING_PRE(s)

#define STRING_FROM_STRING(s, _) s.toStdString()
#define STRING_TO_STRING(s, _) s

#define NUMBER_FROM_STRING(s, type) static_cast<type>(std::stof(s.toStdString()))
#define NUMBER_TO_STRING(s, p) to_string(s, p)

#define NUMBER_RAD_FROM_STRING(s, type) static_cast<type>(std::stof(s.toStdString()) * M_PI / 180)
#define NUMBER_RAD_TO_STRING(s, p) to_string(s * 180 / M_PI, p)

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

#define SNAP_FROM_STRING_PRE(s)                          \
        auto pos = s.find(',');                          \
        auto s2 = s.substr(pos + 1, s.size() - pos - 2); \
        auto pos2 = s2.find(',');                        \
        auto __x = s.substr(1, pos - 1).toStdString();   \
        auto __y = s2.substr(0, pos2 - 1).toStdString(); \
        auto __r = s2.substr(pos2 + 1).toStdString();
#define SNAP_FROM_STRING(s, type) {                                                           \
        { (decltype(type::first.x))std::stod(__x), (decltype(type::first.y))std::stod(__y) }, \
        (decltype(type::second))std::stof(__r) * (float)M_PI / 180.f                          \
}
#define SNAP_TO_STRING(s, _) "(" +                     \
	to_string((s).first.x, 2) + "," +              \
	to_string((s).first.y, 2) + "," +              \
	to_string((int)((s).second * 180 / M_PI), 0) + \
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

#define PLACE_HOOK_STRING(container, guiId, field, name, operation, reset) \
	PLACE_HOOK_STRUCTURE(container, guiId, field, name, operation, STRING_TO_STRING, NO_FROMSTRING_PRE, STRING_FROM_STRING, _, reset)
#define PLACE_HOOK_NUMBER(container, guiId, field, name, operation, reset, precision) \
	PLACE_HOOK_STRUCTURE(container, guiId, field, name, operation, NUMBER_TO_STRING, NO_FROMSTRING_PRE, NUMBER_FROM_STRING, precision, reset)
#define PLACE_HOOK_NUMBER_DEG(container, guiId, field, name, operation, reset, precision) \
	PLACE_HOOK_STRUCTURE(container, guiId, field, name, operation, NUMBER_RAD_TO_STRING, NO_FROMSTRING_PRE, NUMBER_RAD_FROM_STRING, precision, reset)
#define PLACE_HOOK_OPTIONAL_NUMBER(container, guiId, field, name, operation, reset, precision) \
	PLACE_HOOK_OPTIONAL_STRUCTURE(container, guiId, field, name, operation, NUMBER_TO_STRING, NO_FROMSTRING_PRE, NUMBER_FROM_STRING, precision, reset)
#define PLACE_HOOK_NUMFLAGS(container, guiId, field, name, operation, reset) \
	PLACE_HOOK_STRUCTURE(container, guiId, field, name, operation, NUMFLAGS_TO_STRING, NO_FROMSTRING_PRE, NUMFLAGS_FROM_STRING, _, reset)
#define PLACE_HOOK_VECTOR(container, guiId, field, name, operation, reset, precision) \
	PLACE_HOOK_STRUCTURE(container, guiId, field, name, operation, VECTOR_TO_STRING, VECTOR_FROM_STRING_PRE, VECTOR_FROM_STRING, precision, reset)
#define PLACE_HOOK_OPTIONAL_VECTOR(container, guiId, field, name, operation, reset, precision) \
	PLACE_HOOK_OPTIONAL_STRUCTURE(container, guiId, field, name, operation, VECTOR_TO_STRING, VECTOR_FROM_STRING_PRE, VECTOR_FROM_STRING, precision, reset)
#define PLACE_HOOK_OPTIONAL_SNAP(container, guiId, field, name, operation, reset) \
	PLACE_HOOK_OPTIONAL_STRUCTURE(container, guiId, field, name, operation, SNAP_TO_STRING, SNAP_FROM_STRING_PRE, SNAP_FROM_STRING, _, reset)
#define PLACE_HOOK_RECT(container, guiId, field, name, operation, reset) \
	PLACE_HOOK_STRUCTURE(container, guiId, field, name, operation, RECT_TO_STRING, RECT_FROM_STRING_PRE, RECT_FROM_STRING, 0, reset)
#define PLACE_HOOK_FLAG(container, guiId, field, index, name, reset)               \
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
                        index, b, (reset)                                          \
                ));                                                                \
                this->_rePopulateFrameData();                                      \
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

SpiralOfFate::MainWindow::MainWindow(const std::string &frameDataPath, FrameDataEditor &editor) :
	LocalizedContainer<tgui::ChildWindow>(editor, MainWindow::StaticWidgetType, false),
	_editor(editor),
	_path(frameDataPath),
	_character(std::filesystem::path(frameDataPath).parent_path().filename().string()),
	_object(new EditableObject(frameDataPath))
{
	this->_preview = std::make_shared<PreviewWidget>(std::ref(editor), std::ref(*this), *this->_object);
	this->_preview->setPosition(0, 0);
	this->_preview->setSize("&.w", "&.h");

	this->m_renderer = aurora::makeCopied<Renderer>();
	this->loadLocalizedWidgetsFromFile("assets/gui/editor/character/animationWindow.gui");

	auto panel = this->get<tgui::Panel>("AnimationPanel");
	auto showBoxes = panel->get<tgui::BitmapButton>("ShowBoxes");
	auto displace = panel->get<tgui::BitmapButton>("Displace");

	Utils::setRenderer(this);
	this->setSize(1200, 600);
	this->setPosition(10, 30);
	this->setTitleButtons(TitleButton::Minimize | TitleButton::Maximize | TitleButton::Close);
	this->setTitle(frameDataPath);
	this->setResizable();
	this->setCloseBehavior(CloseBehavior::None);
	this->onClose.connect([this]{
		// TODO: Check if needs to be saved
		std::error_code err;

		std::filesystem::remove(this->_path + ".bak");
		this->m_parent->remove(this->shared_from_this());
		this->onRealClose.emit(this);
	});

	panel->add(this->_preview);
	this->_preview->moveToBack();

	showBoxes->onClick.connect([this](std::weak_ptr<tgui::BitmapButton> This){
		this->_preview->displayBoxes = !this->_preview->displayBoxes;
		This.lock()->setImage(tgui::Texture("assets/gui/editor/" + std::string(this->_preview->displayBoxes ? "" : "no") + "boxes.png"));
	}, std::weak_ptr(showBoxes));
	displace->onClick.connect([this](std::weak_ptr<tgui::BitmapButton> This){
		this->_preview->displaceObject = !this->_preview->displaceObject;
		This.lock()->setImage(tgui::Texture("assets/gui/editor/" + std::string(this->_preview->displaceObject ? "" : "no") + "dispose.png"));
	}, std::weak_ptr(displace));

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
	return
		this->_operationIndex != this->_operationSaved ||
		(this->_pendingTransaction && this->_pendingTransaction->hasModification());
}

bool SpiralOfFate::MainWindow::hasUndoData() const noexcept
{
	return this->_operationIndex || (this->_pendingTransaction && this->_pendingTransaction->hasModification());
}

bool SpiralOfFate::MainWindow::hasRedoData() const noexcept
{
	return this->_operationQueue.size() != this->_operationIndex;
}

void SpiralOfFate::MainWindow::redo()
{
	if (this->_pendingTransaction)
		this->commitTransaction();
	if (this->_operationIndex == this->_operationQueue.size())
		return;
	this->_operationQueue[this->_operationIndex]->apply();
	this->_operationIndex++;
	this->_rePopulateData();
	if (this->isModified())
		this->setTitle(this->_path + "*");
	else
		this->setTitle(this->_path);
	this->_editor.setHasUndo(true);
	this->_editor.setHasRedo(this->hasRedoData());
}

void SpiralOfFate::MainWindow::undo()
{
	if (this->_pendingTransaction) {
		this->cancelTransaction();
		return;
	}
	if (this->_operationIndex == 0)
		return;
	this->_operationIndex--;
	this->_operationQueue[this->_operationIndex]->undo();
	this->_rePopulateData();
	if (this->isModified())
		this->setTitle(this->_path + "*");
	else
		this->setTitle(this->_path);
	this->_editor.setHasUndo(this->hasUndoData());
	this->_editor.setHasRedo(true);
}

void SpiralOfFate::MainWindow::startTransaction(SpiralOfFate::Operation *operation)
{
	assert_exp(!this->_pendingTransaction);
	if (!operation)
		operation = new DummyOperation();
	this->_pendingTransaction.reset(operation);
	this->_pendingTransaction->apply();
	if (this->_pendingTransaction->hasModification())
		this->setTitle(this->_path + "*");
	this->_editor.setHasUndo(this->hasUndoData());
}

void SpiralOfFate::MainWindow::updateTransaction(const std::function<Operation *()> &operation)
{
	assert_exp(this->_pendingTransaction);
	this->_pendingTransaction->undo();
	this->_pendingTransaction.reset(operation());
	this->_pendingTransaction->apply();
	if (this->_pendingTransaction->hasModification())
		this->setTitle(this->_path + "*");
	this->_editor.setHasUndo(this->hasUndoData());
}

void SpiralOfFate::MainWindow::cancelTransaction()
{
	assert_exp(this->_pendingTransaction);
	this->_pendingTransaction->undo();
	this->_pendingTransaction.reset();
	if (this->isModified())
		this->setTitle(this->_path + "*");
	else
		this->setTitle(this->_path);
	this->_editor.setHasUndo(this->hasUndoData());
}

void SpiralOfFate::MainWindow::commitTransaction()
{
	assert_exp(this->_pendingTransaction);
	if (!this->_pendingTransaction->hasModification())
		return this->_pendingTransaction.reset();
	this->_operationQueue.erase(this->_operationQueue.begin() + this->_operationIndex, this->_operationQueue.end());
	this->_operationQueue.emplace_back(nullptr);
	this->_operationQueue.back().swap(this->_pendingTransaction);
	if (this->_operationIndex < this->_operationSaved)
		this->_operationSaved = -1;
	this->_operationIndex = this->_operationQueue.size();
	this->setTitle(this->_path + "*");
	this->autoSave();
	this->_editor.setHasUndo(true);
	this->_editor.setHasRedo(false);
}

void SpiralOfFate::MainWindow::applyOperation(Operation *operation)
{
	if (!operation->hasModification()) {
		delete operation;
		return;
	}
	this->_operationQueue.erase(this->_operationQueue.begin() + this->_operationIndex, this->_operationQueue.end());
	this->_operationQueue.emplace_back(operation);
	this->_operationQueue.back()->apply();
	this->_rePopulateData();
	if (this->_operationIndex < this->_operationSaved)
		this->_operationSaved = -1;
	this->_operationIndex = this->_operationQueue.size();
	this->setTitle(this->_path + "*");
	this->_editor.setHasUndo(true);
	this->_editor.setHasRedo(false);
	this->autoSave();
}

void SpiralOfFate::MainWindow::save(const std::string &path)
{
	std::error_code err;

	std::filesystem::rename(this->_path + ".bak", path + ".bak", err);
	if (err) {
		// TODO: Hardcoded string
		// FIXME: strerror only works on Linux (err.message()?)
		SpiralOfFate::Utils::dispMsg(game->gui, "Saving failed", "Cannot rename " + this->_path + ".bak to " + path + ".bak: " + strerror(errno), MB_ICONERROR);
		return;
	}
	this->setPath(path);
	this->save();
}

void SpiralOfFate::MainWindow::setPath(const std::string &path)
{
	this->_path = path;
}

void SpiralOfFate::MainWindow::save()
{
	nlohmann::json j = nlohmann::json::array();

	if (this->_pendingTransaction)
		this->commitTransaction();
	for (auto &[key, value] : this->_object->_moves) {
		j.push_back({
			{"action", key},
			{"framedata", value}
		});
	}

	std::ofstream stream{this->_path};

	if (stream.fail()) {
		// TODO: Hardcoded string
		// FIXME: strerror only works on Linux (err.message()?)
		SpiralOfFate::Utils::dispMsg(game->gui, "Saving failed", "Cannot open " + this->_path + ": " + strerror(errno), MB_ICONERROR);
		return;
	}
	stream << j.dump(2);
	this->_operationSaved = this->_operationIndex;
	this->setTitle(this->_path);
}

void SpiralOfFate::MainWindow::autoSave()
{
	nlohmann::json j = nlohmann::json::array();

	for (auto &[key, value] : this->_object->_moves) {
		j.push_back({
			{"action", key},
			{"framedata", value}
		});
	}

	std::ofstream stream{this->_path + ".bak"};

	if (stream.fail()) {
		SpiralOfFate::Utils::dispMsg(game->gui, "Saving failed", this->_path + ".bak: " + strerror(errno), MB_ICONERROR);
		return;
	}
	stream << j.dump(2);
}

std::string SpiralOfFate::MainWindow::_localizeActionName(unsigned int id)
{
	if (this->_editor.hasLocalization("action." + this->_character + "." + std::to_string(id)))
		return this->_editor.localize("action." + this->_character + "." + std::to_string(id));
	else if (this->_editor.hasLocalization("action.generic." + std::to_string(id)))
		return this->_editor.localize("action.generic." + std::to_string(id));
	return Character::actionToString(id);
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
	};
	auto data = this->_object->getFrameData();

	contentPanel->loadLocalizedWidgetsFromFile(path);
	for (auto &w : contentPanel->getWidgets()) {
		size.x = std::max(size.x, w->getFullSize().x + w->getPosition().x + 20);
		size.y = std::max(size.y, w->getFullSize().y + w->getPosition().y + 20);
	}
	size.y += contentPanel->getSize().y - contentPanel->getInnerSize().y;
	contentPanel->setSize(size);
	Utils::setRenderer(contentPanel->cast<tgui::Container>());
	outsidePanel->onClick.connect(closePopup, std::weak_ptr(outsidePanel), std::weak_ptr(contentPanel));
	contentPanel->onClose.connect(closePopup, std::weak_ptr(outsidePanel), std::weak_ptr(contentPanel));
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

void SpiralOfFate::MainWindow::_createMoveListPopup(const std::function<void(unsigned)> &onConfirm, bool showNotAdded)
{
	auto outsidePanel = tgui::Panel::create({"100%", "100%"});
	auto contentPanel = tgui::ScrollablePanel::create({500, "&.h - 100"});
	unsigned i = 0;
	auto scroll = 0;
	std::set<unsigned> moves;

	for (auto &[moveId, _] : this->_object->_moves)
		moves.insert(moveId);
	if (showNotAdded) {
		for (auto &[moveId, _]: SpiralOfFate::actionNames)
			moves.insert(moveId);
		for (auto &[local, _] : this->_editor.getLocalizationData()) {
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

	for (auto moveId : moves) {
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
		} else if (!this->_object->_moves.contains(moveId)) {
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

		button->onClick.connect(onConfirm, moveId);
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
	auto clearHit = container.get<tgui::Button>("ClearHit");
	auto clearBlock = container.get<tgui::Button>("ClearBlock");
	auto action = container.get<tgui::EditBox>("ActionID");
	auto actionSelect = container.get<tgui::Button>("ActionSelect");
	auto play = container.get<tgui::Button>("Play");
	auto step = container.get<tgui::Button>("Step");
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
			this->_rePopulateData();
		}, std::weak_ptr(action));
	if (clearHit)
		clearHit->onClick.connect([this]{
			this->applyOperation(new ClearHitOperation(*this->_object, this->_editor));
		});
	if (clearBlock)
		clearBlock->onClick.connect([this]{
			this->applyOperation(new ClearBlockOperation(*this->_object, this->_editor));
		});
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
		actionSelect->onClick.connect(&MainWindow::_createMoveListPopup, this, [this](unsigned move){
			this->_object->_action = move;
			this->_object->_actionBlock = 0;
			this->_object->_animation = 0;
			this->_object->_animationCtr = 0;
			this->_rePopulateData();
		}, false);
	if (play)
		play->onPress.connect([this]{
			this->_paused = false;
		});
	if (step)
		step->onPress.connect([this]{
			this->_paused = false;
			this->tick();
			this->_paused = true;
		});
	if (frame)
		frame->onValueChange.connect([this](float value){
			this->_paused = true;
			this->_object->_animation = value;
			this->_object->resetState();
			this->_preview->frameChanged();
			this->_rePopulateFrameData();
		});
	if (frameSpin)
		frameSpin->onValueChange.connect([this](float value){
			this->_paused = true;
			this->_object->_animation = value;
			this->_object->resetState();
			this->_preview->frameChanged();
			this->_rePopulateFrameData();
		});
	if (blockSpin)
		blockSpin->onValueChange.connect([this](float value){
			this->_object->_actionBlock = value;
			this->_object->_animation = 0;
			this->_rePopulateData();
		});

	PLACE_HOOK_STRING(container,   "Sprite",   spritePath,    this->_editor.localize("animation.sprite"),   SpriteChangeOperation, false);
	PLACE_HOOK_NUMBER(container,   "Duration", duration,      this->_editor.localize("animation.duration"), BasicDataOperation, false, 0);
	PLACE_HOOK_VECTOR(container,   "Offset",   offset,        this->_editor.localize("animation.offset"),   BasicDataOperation, false, 0);
	PLACE_HOOK_VECTOR(container,   "Scale",    scale,         this->_editor.localize("animation.scale"),    BasicDataOperation, false, 2);
	PLACE_HOOK_RECT(container,     "Bounds",   textureBounds, this->_editor.localize("animation.bounds"),   BasicDataOperation, false);
	PLACE_HOOK_NUMFLAGS(container, "AFlags",   oFlag,         this->_editor.localize("animation.aflags"),   BasicDataOperation, false);
	PLACE_HOOK_NUMFLAGS(container, "DFlags",   dFlag,         this->_editor.localize("animation.dflags"),   BasicDataOperation, false);

	PLACE_HOOK_NUMBER(container,          "PGen",      particleGenerator, this->_editor.localize("animation.general.partgenerator"), BasicDataOperation, false, 0);
	PLACE_HOOK_NUMBER(container,          "SubObj",    subObjectSpawn,    this->_editor.localize("animation.general.subobj"),        BasicDataOperation, false, 0);
	PLACE_HOOK_NUMBER(container,          "Marker",    specialMarker,     this->_editor.localize("animation.general.marker"),        BasicDataOperation, false, 0);
	PLACE_HOOK_OPTIONAL_NUMBER(container, "Tier",      priority,          this->_editor.localize("animation.general.tier"),          BasicDataOperation, false, 0);
	PLACE_HOOK_VECTOR(container,          "MoveSpeed", speed,             this->_editor.localize("animation.general.speed"),         BasicDataOperation, true,  2);
	PLACE_HOOK_OPTIONAL_VECTOR(container, "Gravity",   gravity,           this->_editor.localize("animation.general.gravity"),       BasicDataOperation, true,  2);
	PLACE_HOOK_STRING(container,          "Sound",     soundPath,         this->_editor.localize("animation.general.sound"),         SoundChangeOperation, false);
	PLACE_HOOK_NUMBER(container,          "FadeTime",  fadeTime,          this->_editor.localize("animation.general.fadetime"),      BasicDataOperation, false, 0);
	PLACE_HOOK_NUMBER(container,          "ManaCost",  manaCost,          this->_editor.localize("animation.general.manacost"),      BasicDataOperation, false, 0);
	PLACE_HOOK_NUMBER_DEG(container,      "Rotation",  rotation,          this->_editor.localize("animation.general.rotation"),      BasicDataOperation, true, 2);

	PLACE_HOOK_NUMBER(container,        "PHitStop",  hitPlayerHitStop,  this->_editor.localize("animation.hit.pstop"),   BasicDataOperation, false, 0);
	PLACE_HOOK_NUMBER(container,        "OHitStop",  hitOpponentHitStop,this->_editor.localize("animation.hit.ostop"),   BasicDataOperation, false, 0);
	PLACE_HOOK_VECTOR(container,        "HitSpeed",  hitSpeed,          this->_editor.localize("animation.hit.speed"),   BasicDataOperation, false, 0);
	PLACE_HOOK_VECTOR(container,        "CHitSpeed", counterHitSpeed,   this->_editor.localize("animation.hit.chspeed"), BasicDataOperation, false, 0);
	PLACE_HOOK_NUMBER(container,        "NLimit",    neutralLimit,      this->_editor.localize("animation.hit.nlimit"),  BasicDataOperation, false, 0);
	PLACE_HOOK_NUMBER(container,        "SLimit",    spiritLimit,       this->_editor.localize("animation.hit.slimit"),  BasicDataOperation, false, 0);
	PLACE_HOOK_NUMBER(container,        "MLimit",    matterLimit,       this->_editor.localize("animation.hit.mlimit"),  BasicDataOperation, false, 0);
	PLACE_HOOK_NUMBER(container,        "VLimit",    voidLimit,         this->_editor.localize("animation.hit.vlimit"),  BasicDataOperation, false, 0);
	PLACE_HOOK_NUMBER(container,        "Rate",      prorate,           this->_editor.localize("animation.hit.rate"),    BasicDataOperation, false, 2);
	PLACE_HOOK_NUMBER(container,        "MinRate",   minProrate,        this->_editor.localize("animation.hit.minrate"), BasicDataOperation, false, 2);
	PLACE_HOOK_NUMBER(container,        "Untech",    untech,            this->_editor.localize("animation.hit.untech"),  BasicDataOperation, false, 0);
	PLACE_HOOK_NUMBER(container,        "Damage",    damage,            this->_editor.localize("animation.hit.damage"),  BasicDataOperation, false, 0);
	PLACE_HOOK_NUMBER(container,        "HitStun",   hitStun,           this->_editor.localize("animation.hit.stun"),    BasicDataOperation, false, 0);
	PLACE_HOOK_STRING(container,        "HitSound",  hitSoundPath,      this->_editor.localize("animation.hit.sound"),   SoundChangeOperation, false);
	PLACE_HOOK_OPTIONAL_SNAP(container, "HitSnap",   snap,              this->_editor.localize("animation.hit.snap"),    BasicDataOperation, false);

	PLACE_HOOK_NUMBER(container, "PBlockStop", blockPlayerHitStop,  this->_editor.localize("animation.block.pstop"),   BasicDataOperation, false, 0);
	PLACE_HOOK_NUMBER(container, "OBlockStop", blockOpponentHitStop,this->_editor.localize("animation.block.ostop"),   BasicDataOperation, false, 0);
	PLACE_HOOK_NUMBER(container, "PPush",      pushBack,            this->_editor.localize("animation.block.ppush"),   BasicDataOperation, false, 0);
	PLACE_HOOK_NUMBER(container, "OPush",      pushBlock,           this->_editor.localize("animation.block.opush"),   BasicDataOperation, false, 0);
	PLACE_HOOK_NUMBER(container, "GuardDmg",   guardDmg,            this->_editor.localize("animation.block.guarddmg"),BasicDataOperation, false, 0);
	PLACE_HOOK_NUMBER(container, "Chip",       chipDamage,          this->_editor.localize("animation.block.chip"),    BasicDataOperation, false, 0);
	PLACE_HOOK_NUMBER(container, "BlockStun",  blockStun,           this->_editor.localize("animation.block.stun"),    BasicDataOperation, false, 0);
	PLACE_HOOK_NUMBER(container, "WBlockStun", wrongBlockStun,      this->_editor.localize("animation.block.wstun"),   BasicDataOperation, false, 0);

	for (size_t i = 0; i < 64; i++)
		PLACE_HOOK_FLAG(container, "aFlag" + std::to_string(i), oFlag, i, this->_editor.localize("animation.aflags.flag" + std::to_string(i)), false);
	for (size_t i = 0; i < 64; i++)
		PLACE_HOOK_FLAG(container, "dFlag" + std::to_string(i), dFlag, i, this->_editor.localize("animation.dflags.flag" + std::to_string(i)), i == 8 || i == 13 || i == 21);
}


void SpiralOfFate::MainWindow::newFrame()
{
	this->applyOperation(new CreateFrameOperation(
		*this->_object,
		this->_editor.localize("operation.create_frame"),
		this->_object->_action,
		this->_object->_actionBlock,
		this->_object->_animation,
		this->_object->getFrameData()
	));
}

void SpiralOfFate::MainWindow::newEndFrame()
{
	this->applyOperation(new CreateFrameOperation(
		*this->_object,
		this->_editor.localize("operation.create_frame_end"),
		this->_object->_action,
		this->_object->_actionBlock,
		this->_object->_moves[this->_object->_action][this->_object->_actionBlock].size(),
		this->_object->getFrameData()
	));
}

void SpiralOfFate::MainWindow::newAnimationBlock()
{
	this->applyOperation(new CreateBlockOperation(
		*this->_object,
		this->_editor.localize("operation.create_block"),
		this->_object->_action,
		this->_object->_actionBlock + 1,
		{ this->_object->getFrameData() }
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

	window->setTitle(this->_editor.localize("create_action.title"));
	create->setEnabled(false);
	idBox->onTextChange.connect([createW, actionW, idBoxW, this](const tgui::String &t){
		if (t.empty()) {
			actionW.lock()->setText("");
			createW.lock()->setEnabled(false);
		} else {
			actionW.lock()->setText(this->_localizeActionName(std::stoul(t.toStdString())));
			createW.lock()->setEnabled(true);
		}
	});
	action->onClick.connect(&MainWindow::_createMoveListPopup, this, [idBoxW](unsigned move){
		idBoxW.lock()->setText(std::to_string(move));
	}, true);
	create->onClick.connect([windowW, idBox, this]{
		auto &data = this->_object->getFrameData();
		auto action = std::stoul(idBox->getText().toStdString());

		if (this->_object->_moves.contains(action)) {
			this->_object->_action = action;
			this->_object->_actionBlock = 0;
			this->_object->_animation = 0;
			this->_rePopulateData();
		} else
			this->applyOperation(new CreateMoveOperation(
				*this->_object,
				this->_editor.localize("operation.create_move"),
				action, {{data}}
			));
		windowW.lock()->close();
	});
}

void SpiralOfFate::MainWindow::newHurtBox()
{

}

void SpiralOfFate::MainWindow::newHitBox()
{

}

void SpiralOfFate::MainWindow::removeFrame()
{

}

void SpiralOfFate::MainWindow::removeAnimationBlock()
{

}

void SpiralOfFate::MainWindow::removeAction()
{

}

void SpiralOfFate::MainWindow::copyBoxesFromLastFrame()
{

}

void SpiralOfFate::MainWindow::copyBoxesFromNextFrame()
{

}

void SpiralOfFate::MainWindow::flattenThisMoveCollisionBoxes()
{

}

void SpiralOfFate::MainWindow::reloadTextures()
{

}


void SpiralOfFate::MainWindow::_rePopulateData()
{
	for (auto &[key, _] : this->_updateFrameElements)
		this->_populateData(*key);
	this->_object->resetState();
	this->_preview->frameChanged();
}

void SpiralOfFate::MainWindow::_rePopulateFrameData()
{
	for (auto &[key, _] : this->_updateFrameElements)
		this->_populateFrameData(*key);
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
	if (animation != this->_object->_animation) {
		this->_preview->frameChanged();
		for (auto &[key, _]: this->_updateFrameElements)
			this->_populateFrameData(*key);
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

void SpiralOfFate::MainWindow::keyPressed(const tgui::Event::KeyEvent &event)
{
	if (!event.alt && !event.control && !event.shift && !event.system && event.code == tgui::Event::KeyboardKey::Escape) {
		if (this->_pendingTransaction) {
			this->cancelTransaction();
			this->_rePopulateData();
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

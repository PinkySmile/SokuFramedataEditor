//
// Created by PinkySmile on 03/01/2026.
//

#include "ShortcutsWindow.hpp"

SpiralOfFate::ShortcutsWindow::ShortcutsWindow(FrameDataEditor &editor) :
	LocalizedContainer<tgui::ChildWindow>(editor)
{
	this->loadLocalizedWidgetsFromFile("assets/gui/editor/shortcuts.gui");
	Utils::setRenderer(*this);
	this->_shortcuts = editor.getShortcuts();
	this->_poplulateData();

	auto defaut = this->get<tgui::Button>("Default");
	auto cancel = this->get<tgui::Button>("Cancel");
	auto save = this->get<tgui::Button>("Save");

	defaut->onClick([this] {
		this->_editor.restoreDefaultShortcuts(this->_shortcuts);
		this->_poplulateData();
	});
	cancel->onClick([this]{
		this->close();
	});
	save->onClick([this, &editor]{
		editor.setShortcuts(this->_shortcuts);
		editor.saveSettings();
		this->close();
	});
}

void SpiralOfFate::ShortcutsWindow::_poplulateData()
{
	auto panel = this->get<tgui::ScrollablePanel>("Shortcuts");
	size_t i = 0;

	for (auto &name : this->_editor.getShortcutsNames()) {
		auto npanel = panel->get<LocalizedContainer<tgui::Panel>>(name);
		tgui::Button::Ptr key;

		if (!npanel) {
			npanel = std::make_shared<LocalizedContainer<tgui::Panel>>(this->_editor);
			npanel->loadLocalizedWidgetsFromFile("assets/gui/editor/shortcut.gui");
			npanel->setSize({"100%", 35});
			npanel->setPosition({0, i++ * 35});
			Utils::setRenderer(static_cast<tgui::Container::Ptr>(npanel));
			panel->add(npanel);

			auto label = npanel->get<tgui::Label>("Shortcut");
			auto rm = npanel->get<tgui::Button>("Remove");

			key = npanel->get<tgui::Button>("Key");
			label->setText(this->_editor.localize("shortcut." + name));
			key->onClick([name, this](std::weak_ptr<tgui::Button> me) {
				if (this->_changingShortcut.second) {
					this->_changingShortcut.second->setText(this->_editor.shortcutToString(this->_shortcuts[this->_changingShortcut.first]));
					Utils::setRenderer(this->_changingShortcut.second);
				}
				this->_changingShortcut = {name, me.lock()};
				this->_changingShortcut.second->setText(this->_editor.localize("shortcut.press"));
				this->_changingShortcut.second->getRenderer()->setTextColor(tgui::Color{0xFF, 0x80, 0x00, 0xFF});
				this->_tmp.code = sf::Keyboard::Key::Unknown;
				this->_tmp.alt = false;
				this->_tmp.shift = false;
				this->_tmp.control = false;
				this->_tmp.meta = false;
			}, std::weak_ptr(key));
			rm->onClick([name, key, this] {
				if (this->_changingShortcut.second) {
					this->_changingShortcut.second->setText(this->_editor.shortcutToString(this->_shortcuts[this->_changingShortcut.first]));
					Utils::setRenderer(this->_changingShortcut.second);
					this->_changingShortcut.second.reset();
					return;
				}
				auto it = this->_shortcuts.find(name);

				if (it == this->_shortcuts.end())
					return;
				this->_shortcuts.erase(it);
				key->setText(this->_editor.localize("shortcut.none"));
			});
		} else
			key = npanel->get<tgui::Button>("Key");

		auto shortcut = this->_shortcuts.find(name);

		if (shortcut == this->_shortcuts.end())
			key->setText(this->_editor.localize("shortcut.none"));
		else
			key->setText(this->_editor.shortcutToString(shortcut->second));
	}
}

static_assert(static_cast<int>(sf::Keyboard::Key::Unknown) == static_cast<int>(tgui::Event::KeyboardKey::Unknown));
static_assert(static_cast<int>(sf::Keyboard::Key::A) == static_cast<int>(tgui::Event::KeyboardKey::A));
static_assert(static_cast<int>(sf::Keyboard::Key::B) == static_cast<int>(tgui::Event::KeyboardKey::B));
static_assert(static_cast<int>(sf::Keyboard::Key::C) == static_cast<int>(tgui::Event::KeyboardKey::C));
static_assert(static_cast<int>(sf::Keyboard::Key::D) == static_cast<int>(tgui::Event::KeyboardKey::D));
static_assert(static_cast<int>(sf::Keyboard::Key::E) == static_cast<int>(tgui::Event::KeyboardKey::E));
static_assert(static_cast<int>(sf::Keyboard::Key::F) == static_cast<int>(tgui::Event::KeyboardKey::F));
static_assert(static_cast<int>(sf::Keyboard::Key::G) == static_cast<int>(tgui::Event::KeyboardKey::G));
static_assert(static_cast<int>(sf::Keyboard::Key::H) == static_cast<int>(tgui::Event::KeyboardKey::H));
static_assert(static_cast<int>(sf::Keyboard::Key::I) == static_cast<int>(tgui::Event::KeyboardKey::I));
static_assert(static_cast<int>(sf::Keyboard::Key::J) == static_cast<int>(tgui::Event::KeyboardKey::J));
static_assert(static_cast<int>(sf::Keyboard::Key::K) == static_cast<int>(tgui::Event::KeyboardKey::K));
static_assert(static_cast<int>(sf::Keyboard::Key::L) == static_cast<int>(tgui::Event::KeyboardKey::L));
static_assert(static_cast<int>(sf::Keyboard::Key::M) == static_cast<int>(tgui::Event::KeyboardKey::M));
static_assert(static_cast<int>(sf::Keyboard::Key::N) == static_cast<int>(tgui::Event::KeyboardKey::N));
static_assert(static_cast<int>(sf::Keyboard::Key::O) == static_cast<int>(tgui::Event::KeyboardKey::O));
static_assert(static_cast<int>(sf::Keyboard::Key::P) == static_cast<int>(tgui::Event::KeyboardKey::P));
static_assert(static_cast<int>(sf::Keyboard::Key::Q) == static_cast<int>(tgui::Event::KeyboardKey::Q));
static_assert(static_cast<int>(sf::Keyboard::Key::R) == static_cast<int>(tgui::Event::KeyboardKey::R));
static_assert(static_cast<int>(sf::Keyboard::Key::S) == static_cast<int>(tgui::Event::KeyboardKey::S));
static_assert(static_cast<int>(sf::Keyboard::Key::T) == static_cast<int>(tgui::Event::KeyboardKey::T));
static_assert(static_cast<int>(sf::Keyboard::Key::U) == static_cast<int>(tgui::Event::KeyboardKey::U));
static_assert(static_cast<int>(sf::Keyboard::Key::V) == static_cast<int>(tgui::Event::KeyboardKey::V));
static_assert(static_cast<int>(sf::Keyboard::Key::W) == static_cast<int>(tgui::Event::KeyboardKey::W));
static_assert(static_cast<int>(sf::Keyboard::Key::X) == static_cast<int>(tgui::Event::KeyboardKey::X));
static_assert(static_cast<int>(sf::Keyboard::Key::Y) == static_cast<int>(tgui::Event::KeyboardKey::Y));
static_assert(static_cast<int>(sf::Keyboard::Key::Z) == static_cast<int>(tgui::Event::KeyboardKey::Z));
static_assert(static_cast<int>(sf::Keyboard::Key::Num0) == static_cast<int>(tgui::Event::KeyboardKey::Num0));
static_assert(static_cast<int>(sf::Keyboard::Key::Num1) == static_cast<int>(tgui::Event::KeyboardKey::Num1));
static_assert(static_cast<int>(sf::Keyboard::Key::Num2) == static_cast<int>(tgui::Event::KeyboardKey::Num2));
static_assert(static_cast<int>(sf::Keyboard::Key::Num3) == static_cast<int>(tgui::Event::KeyboardKey::Num3));
static_assert(static_cast<int>(sf::Keyboard::Key::Num4) == static_cast<int>(tgui::Event::KeyboardKey::Num4));
static_assert(static_cast<int>(sf::Keyboard::Key::Num5) == static_cast<int>(tgui::Event::KeyboardKey::Num5));
static_assert(static_cast<int>(sf::Keyboard::Key::Num6) == static_cast<int>(tgui::Event::KeyboardKey::Num6));
static_assert(static_cast<int>(sf::Keyboard::Key::Num7) == static_cast<int>(tgui::Event::KeyboardKey::Num7));
static_assert(static_cast<int>(sf::Keyboard::Key::Num8) == static_cast<int>(tgui::Event::KeyboardKey::Num8));
static_assert(static_cast<int>(sf::Keyboard::Key::Num9) == static_cast<int>(tgui::Event::KeyboardKey::Num9));
static_assert(static_cast<int>(sf::Keyboard::Key::Escape) == static_cast<int>(tgui::Event::KeyboardKey::Escape));
static_assert(static_cast<int>(sf::Keyboard::Key::LControl) == static_cast<int>(tgui::Event::KeyboardKey::LControl));
static_assert(static_cast<int>(sf::Keyboard::Key::LShift) == static_cast<int>(tgui::Event::KeyboardKey::LShift));
static_assert(static_cast<int>(sf::Keyboard::Key::LAlt) == static_cast<int>(tgui::Event::KeyboardKey::LAlt));
static_assert(static_cast<int>(sf::Keyboard::Key::LSystem) == static_cast<int>(tgui::Event::KeyboardKey::LSystem));
static_assert(static_cast<int>(sf::Keyboard::Key::RControl) == static_cast<int>(tgui::Event::KeyboardKey::RControl));
static_assert(static_cast<int>(sf::Keyboard::Key::RShift) == static_cast<int>(tgui::Event::KeyboardKey::RShift));
static_assert(static_cast<int>(sf::Keyboard::Key::RAlt) == static_cast<int>(tgui::Event::KeyboardKey::RAlt));
static_assert(static_cast<int>(sf::Keyboard::Key::RSystem) == static_cast<int>(tgui::Event::KeyboardKey::RSystem));
static_assert(static_cast<int>(sf::Keyboard::Key::Menu) == static_cast<int>(tgui::Event::KeyboardKey::Menu));
static_assert(static_cast<int>(sf::Keyboard::Key::LBracket) == static_cast<int>(tgui::Event::KeyboardKey::LBracket));
static_assert(static_cast<int>(sf::Keyboard::Key::RBracket) == static_cast<int>(tgui::Event::KeyboardKey::RBracket));
static_assert(static_cast<int>(sf::Keyboard::Key::Semicolon) == static_cast<int>(tgui::Event::KeyboardKey::Semicolon));
static_assert(static_cast<int>(sf::Keyboard::Key::Comma) == static_cast<int>(tgui::Event::KeyboardKey::Comma));
static_assert(static_cast<int>(sf::Keyboard::Key::Period) == static_cast<int>(tgui::Event::KeyboardKey::Period));
static_assert(static_cast<int>(sf::Keyboard::Key::Apostrophe) == static_cast<int>(tgui::Event::KeyboardKey::Quote));
static_assert(static_cast<int>(sf::Keyboard::Key::Slash) == static_cast<int>(tgui::Event::KeyboardKey::Slash));
static_assert(static_cast<int>(sf::Keyboard::Key::Backslash) == static_cast<int>(tgui::Event::KeyboardKey::Backslash));
static_assert(static_cast<int>(sf::Keyboard::Key::Grave) == static_cast<int>(tgui::Event::KeyboardKey::Tilde));
static_assert(static_cast<int>(sf::Keyboard::Key::Equal) == static_cast<int>(tgui::Event::KeyboardKey::Equal));
static_assert(static_cast<int>(sf::Keyboard::Key::Hyphen) == static_cast<int>(tgui::Event::KeyboardKey::Minus));
static_assert(static_cast<int>(sf::Keyboard::Key::Space) == static_cast<int>(tgui::Event::KeyboardKey::Space));
static_assert(static_cast<int>(sf::Keyboard::Key::Enter) == static_cast<int>(tgui::Event::KeyboardKey::Enter));
static_assert(static_cast<int>(sf::Keyboard::Key::Backspace) == static_cast<int>(tgui::Event::KeyboardKey::Backspace));
static_assert(static_cast<int>(sf::Keyboard::Key::Tab) == static_cast<int>(tgui::Event::KeyboardKey::Tab));
static_assert(static_cast<int>(sf::Keyboard::Key::PageUp) == static_cast<int>(tgui::Event::KeyboardKey::PageUp));
static_assert(static_cast<int>(sf::Keyboard::Key::PageDown) == static_cast<int>(tgui::Event::KeyboardKey::PageDown));
static_assert(static_cast<int>(sf::Keyboard::Key::End) == static_cast<int>(tgui::Event::KeyboardKey::End));
static_assert(static_cast<int>(sf::Keyboard::Key::Home) == static_cast<int>(tgui::Event::KeyboardKey::Home));
static_assert(static_cast<int>(sf::Keyboard::Key::Insert) == static_cast<int>(tgui::Event::KeyboardKey::Insert));
static_assert(static_cast<int>(sf::Keyboard::Key::Delete) == static_cast<int>(tgui::Event::KeyboardKey::Delete));
static_assert(static_cast<int>(sf::Keyboard::Key::Add) == static_cast<int>(tgui::Event::KeyboardKey::Add));
static_assert(static_cast<int>(sf::Keyboard::Key::Subtract) == static_cast<int>(tgui::Event::KeyboardKey::Subtract));
static_assert(static_cast<int>(sf::Keyboard::Key::Multiply) == static_cast<int>(tgui::Event::KeyboardKey::Multiply));
static_assert(static_cast<int>(sf::Keyboard::Key::Divide) == static_cast<int>(tgui::Event::KeyboardKey::Divide));
static_assert(static_cast<int>(sf::Keyboard::Key::Left) == static_cast<int>(tgui::Event::KeyboardKey::Left));
static_assert(static_cast<int>(sf::Keyboard::Key::Right) == static_cast<int>(tgui::Event::KeyboardKey::Right));
static_assert(static_cast<int>(sf::Keyboard::Key::Up) == static_cast<int>(tgui::Event::KeyboardKey::Up));
static_assert(static_cast<int>(sf::Keyboard::Key::Down) == static_cast<int>(tgui::Event::KeyboardKey::Down));
static_assert(static_cast<int>(sf::Keyboard::Key::Numpad0) == static_cast<int>(tgui::Event::KeyboardKey::Numpad0));
static_assert(static_cast<int>(sf::Keyboard::Key::Numpad1) == static_cast<int>(tgui::Event::KeyboardKey::Numpad1));
static_assert(static_cast<int>(sf::Keyboard::Key::Numpad2) == static_cast<int>(tgui::Event::KeyboardKey::Numpad2));
static_assert(static_cast<int>(sf::Keyboard::Key::Numpad3) == static_cast<int>(tgui::Event::KeyboardKey::Numpad3));
static_assert(static_cast<int>(sf::Keyboard::Key::Numpad4) == static_cast<int>(tgui::Event::KeyboardKey::Numpad4));
static_assert(static_cast<int>(sf::Keyboard::Key::Numpad5) == static_cast<int>(tgui::Event::KeyboardKey::Numpad5));
static_assert(static_cast<int>(sf::Keyboard::Key::Numpad6) == static_cast<int>(tgui::Event::KeyboardKey::Numpad6));
static_assert(static_cast<int>(sf::Keyboard::Key::Numpad7) == static_cast<int>(tgui::Event::KeyboardKey::Numpad7));
static_assert(static_cast<int>(sf::Keyboard::Key::Numpad8) == static_cast<int>(tgui::Event::KeyboardKey::Numpad8));
static_assert(static_cast<int>(sf::Keyboard::Key::Numpad9) == static_cast<int>(tgui::Event::KeyboardKey::Numpad9));
static_assert(static_cast<int>(sf::Keyboard::Key::F1) == static_cast<int>(tgui::Event::KeyboardKey::F1));
static_assert(static_cast<int>(sf::Keyboard::Key::F2) == static_cast<int>(tgui::Event::KeyboardKey::F2));
static_assert(static_cast<int>(sf::Keyboard::Key::F3) == static_cast<int>(tgui::Event::KeyboardKey::F3));
static_assert(static_cast<int>(sf::Keyboard::Key::F4) == static_cast<int>(tgui::Event::KeyboardKey::F4));
static_assert(static_cast<int>(sf::Keyboard::Key::F5) == static_cast<int>(tgui::Event::KeyboardKey::F5));
static_assert(static_cast<int>(sf::Keyboard::Key::F6) == static_cast<int>(tgui::Event::KeyboardKey::F6));
static_assert(static_cast<int>(sf::Keyboard::Key::F7) == static_cast<int>(tgui::Event::KeyboardKey::F7));
static_assert(static_cast<int>(sf::Keyboard::Key::F8) == static_cast<int>(tgui::Event::KeyboardKey::F8));
static_assert(static_cast<int>(sf::Keyboard::Key::F9) == static_cast<int>(tgui::Event::KeyboardKey::F9));
static_assert(static_cast<int>(sf::Keyboard::Key::F10) == static_cast<int>(tgui::Event::KeyboardKey::F10));
static_assert(static_cast<int>(sf::Keyboard::Key::F11) == static_cast<int>(tgui::Event::KeyboardKey::F11));
static_assert(static_cast<int>(sf::Keyboard::Key::F12) == static_cast<int>(tgui::Event::KeyboardKey::F12));
static_assert(static_cast<int>(sf::Keyboard::Key::F13) == static_cast<int>(tgui::Event::KeyboardKey::F13));
static_assert(static_cast<int>(sf::Keyboard::Key::F14) == static_cast<int>(tgui::Event::KeyboardKey::F14));
static_assert(static_cast<int>(sf::Keyboard::Key::F15) == static_cast<int>(tgui::Event::KeyboardKey::F15));
static_assert(static_cast<int>(sf::Keyboard::Key::Pause) == static_cast<int>(tgui::Event::KeyboardKey::Pause));

void SpiralOfFate::ShortcutsWindow::keyPressed(const tgui::Event::KeyEvent &event)
{
	if (this->_changingShortcut.second) {
		switch (this->_tmp.code) {
		case sf::Keyboard::Key::LControl:
		case sf::Keyboard::Key::RControl:
			this->_tmp.control = true;
			break;
		case sf::Keyboard::Key::LAlt:
		case sf::Keyboard::Key::RAlt:
			this->_tmp.alt = true;
			break;
		case sf::Keyboard::Key::LShift:
		case sf::Keyboard::Key::RShift:
			this->_tmp.shift = true;
			break;
		case sf::Keyboard::Key::LSystem:
		case sf::Keyboard::Key::RSystem:
			this->_tmp.meta = true;
			break;
		default:
			break;
		}

		this->_tmp.code = static_cast<sf::Keyboard::Key>(event.code);
		this->_changingShortcut.second->setText(this->_editor.shortcutToString(this->_tmp));

		switch (this->_tmp.code) {
		case sf::Keyboard::Key::LControl:
		case sf::Keyboard::Key::RControl:
		case sf::Keyboard::Key::LAlt:
		case sf::Keyboard::Key::RAlt:
		case sf::Keyboard::Key::LShift:
		case sf::Keyboard::Key::RShift:
		case sf::Keyboard::Key::LSystem:
		case sf::Keyboard::Key::RSystem:
			break;
		default:
			this->_shortcuts[this->_changingShortcut.first] = this->_tmp;
			Utils::setRenderer(this->_changingShortcut.second);
			this->_changingShortcut.second.reset();
			break;
		}
		return;
	}
	LocalizedContainer<ChildWindow>::keyPressed(event);
}

bool SpiralOfFate::ShortcutsWindow::canHandleKeyPress(const tgui::Event::KeyEvent &event)
{
	if (this->_changingShortcut.second)
		return true;
	return LocalizedContainer<ChildWindow>::canHandleKeyPress(event);
}

void SpiralOfFate::ShortcutsWindow::keyReleased(const sf::Event::KeyReleased &event)
{
	if (!this->_changingShortcut.second)
		return;
	if (event.code == this->_tmp.code) {
		this->_shortcuts[this->_changingShortcut.first] = this->_tmp;
		this->_changingShortcut.second->setText(this->_editor.shortcutToString(this->_tmp));
		Utils::setRenderer(this->_changingShortcut.second);
		this->_changingShortcut.second.reset();
	}
}

SpiralOfFate::ShortcutsWindow::Ptr SpiralOfFate::ShortcutsWindow::create(SpiralOfFate::FrameDataEditor &editor)
{
	return std::make_shared<ShortcutsWindow>(editor);
}

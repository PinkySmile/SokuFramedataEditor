//
// Created by Gegel85 on 07/04/2020.
//

#ifdef _WIN32
#include <windows.h>
#endif
#ifdef __GNUG__
#include <cxxabi.h>
#endif
#include <regex>
#include <codecvt>
#include <iostream>
#include <numeric>
#include <fstream>
#include <cmath>
#include <SFML/Graphics.hpp>
#include "Utils.hpp"

namespace Utils
{
	static tgui::Theme *theme = nullptr;
	static const std::map<std::string, std::string> _icons{
		{"folder", "assets/icons/folder.png"     },
		{".rar",   "assets/icons/archive.png"    },
		{".zip",   "assets/icons/archive.png"    },
		{".7z",    "assets/icons/archive.png"    },
		{".tgz",   "assets/icons/archive.png"    },
		{".gz",    "assets/icons/archive.png"    },
		{".xz",    "assets/icons/archive.png"    },
		{".htm",   "assets/icons/webFile.png"    },
		{".html",  "assets/icons/webFile.png"    },
		{".css",   "assets/icons/webFile.png"    },
		{".mimp",  "assets/icons/mimpFile.png"   },
		{".iso",   "assets/icons/discFile.png"   },
		{".txt",   "assets/icons/textFile.png"   },
		{".doc",   "assets/icons/textFile.png"   },
		{".mid",   "assets/icons/midiFile.png"   },
		{".midi",  "assets/icons/midiFile.png"   },
		{".class", "assets/icons/javaFile.png"   },
		{".jar",   "assets/icons/javaFile.png"   },
		{".ttf",   "assets/icons/fontsFile.png"  },
		{".otf",   "assets/icons/fontsFile.png"  },
		{".wav",   "assets/icons/musicFile.png"  },
		{".ogg",   "assets/icons/musicFile.png"  },
		{".mp3",   "assets/icons/musicFile.png"  },
		{".png",   "assets/icons/imageFile.png"  },
		{".jpg",   "assets/icons/imageFile.png"  },
		{".jpeg",  "assets/icons/imageFile.png"  },
		{".bmp",   "assets/icons/imageFile.png"  },
		{".gif",   "assets/icons/imageFile.png"  },
		{".exe",   "assets/icons/binaryFile.png" },
		{".o",     "assets/icons/binaryFile.png" },
		{".out",   "assets/icons/binaryFile.png" },
		{".a",     "assets/icons/binaryFile.png" },
		{".gch",   "assets/icons/binaryFile.png" },
		{".dll",   "assets/icons/binaryFile.png" },
		{".so",    "assets/icons/binaryFile.png" },
		{".xml",   "assets/icons/configFile.png" },
		{".ini",   "assets/icons/configFile.png" },
		{".json",  "assets/icons/configFile.png" },
		{".md",    "assets/icons/configFile.png" },
		{".c",     "assets/icons/sourceFile.png" },
		{".h",     "assets/icons/sourceFile.png" },
		{".hpp",   "assets/icons/sourceFile.png" },
		{".cpp",   "assets/icons/sourceFile.png" },
		{".lua",   "assets/icons/sourceFile.png" },
		{".java",  "assets/icons/sourceFile.png" },
		{".py",    "assets/icons/sourceFile.png" },
		{".asm",   "assets/icons/sourceFile.png" },
		{".php",   "assets/icons/sourceFile.png" },
		{".js",    "assets/icons/sourceFile.png" },
		{".sh",    "assets/icons/shellScript.png"},
		{".run",   "assets/icons/shellScript.png"},
		{".bat",   "assets/icons/shellScript.png"},
		{"",       "assets/icons/unknownFile.png"},
	};

	tgui::Theme &getTheme()
	{
		if (!theme)
			theme = new tgui::Theme("assets/gui/themes/Black.txt");
		return *theme;
	}

	void setRenderer(const tgui::Widget::Ptr &widget)
	{
		auto renderer = getTheme().getRendererNoThrow(widget->getWidgetType());

		if (renderer)
			widget->setRenderer(renderer);
	}

	std::wstring utf8ToWstring(const std::string& str)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
		return myconv.from_bytes(str);
	}

	std::string wstringToUtf8(const std::wstring& str)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
		return myconv.to_bytes(str);
	}

	std::string pathToString(const std::filesystem::path &path) {
		return path.string();
	}

	std::string getLastExceptionName()
	{
#ifdef __GNUG__
		int status;
		char *value;
		std::string name;

		auto val = abi::__cxa_current_exception_type();

		if (!val)
			return "No exception";

		value = abi::__cxa_demangle(val->name(), nullptr, nullptr, &status);
		name = value;
		free(value);
		return name;
#else
		return "Unknown exception";
#endif
	}

#ifndef __ANDROID__
	int	dispMsg(const std::string &title, const std::string &content, int variate, sf::RenderWindow *w)
	{
#ifdef _WIN32
		HWND hwnd = w ? w->getSystemHandle() : nullptr;

		return MessageBoxA(hwnd, content.c_str(), title.c_str(), variate);
#else
		auto button = tgui::Button::create("OK");
		auto text = tgui::TextBox::create();
		tgui::Gui gui;
		auto font = tgui::getGlobalFont();
		const auto startWidth = button->getSize().x + 102;
		unsigned width = startWidth;
		unsigned height = button->getSize().y + 60;
		float currentWidth = startWidth;
		auto size = text->getTextSize();

		setRenderer(button);
		setRenderer(text);
		for (char c: content) {
			currentWidth += font.getGlyph(c, size, false).advance;
			width = std::max(static_cast<unsigned>(currentWidth), width);
			if (c == '\n' || c == '\r')
				currentWidth = startWidth;
			if (c == '\n' || c == '\v')
				height += size;
			if (currentWidth >= 700) {
				currentWidth = startWidth;
				height += size;
			}
		}

		sf::RenderWindow win{{std::min(700U, width), std::min(220U, height)}, title, sf::Style::Titlebar | sf::Style::Close};
		tgui::Picture::Ptr pic;
		sf::Event event;
		try {
			if ((variate & 0xF0) == MB_ICONINFORMATION)
				pic = tgui::Picture::create("assets/icons/info.png");
			else if ((variate & 0xF0) == MB_ICONERROR)
				pic = tgui::Picture::create("assets/icons/error.png");
			else if ((variate & 0xF0) == MB_ICONWARNING)
				pic = tgui::Picture::create("assets/icons/warn.png");
		} catch (...) {}

		gui.setTarget(win);
		gui.add(button, "ok");
		gui.add(text);

		button->setPosition("&.w - w - 10", "&.h - h - 10");
		button->connect("Pressed", [&win] {
			win.close();
		});

		text->setText(content);
		text->setPosition(52, 10);
		text->setSize("ok.x - 62", "ok.y - 20");
		text->setReadOnly();
		text->getRenderer()->setBorderColor("transparent");
		text->getRenderer()->setBackgroundColor("transparent");

		if (pic) {
			pic->setPosition(10, 10);
			pic->setSize(32, 32);
			gui.add(pic);
		}

		while (win.isOpen()) {
			if (w)
				while (w->pollEvent(event))
					if (
						event.type != sf::Event::MouseEntered &&
						event.type != sf::Event::MouseLeft &&
						event.type != sf::Event::MouseMoved &&
						event.type == sf::Event::LostFocus &&
						event.type == sf::Event::GainedFocus
					)
						win.requestFocus();
			while (win.pollEvent(event)) {
				if (event.type == sf::Event::Closed)
					win.close();
				gui.handleEvent(event);
			}

			win.clear({80, 80, 80, 255});
			gui.draw();
			win.display();
		}

		return 0;
#endif
	}

	static void _makeFolders(
		std::filesystem::path &currentPath,
		const tgui::ScrollablePanel::Ptr &panel,
		const tgui::EditBox::Ptr &file,
		const std::function<void(bool)> &open,
		const std::regex &pattern = std::regex(".*", std::regex_constants::icase),
		bool onlyFolders = false
	) {
		auto pos = 10;
		std::vector<std::filesystem::directory_entry> paths = {
			std::filesystem::directory_entry("."),
			std::filesystem::directory_entry("..")
		};

		for (auto &entry : std::filesystem::directory_iterator(currentPath))
			paths.push_back(entry);

		std::sort(paths.begin(), paths.end(), [](std::filesystem::directory_entry &a, std::filesystem::directory_entry &b){
			if (a.is_directory() != b.is_directory())
				return a.is_directory();
			return a.path() < b.path();
		});
		panel->removeAllWidgets();
		for (auto &entry : paths) {
			std::string pic;
			const auto &filePath = entry.path();
			auto fileStr = pathToString(filePath.filename());

			if (entry.is_directory())
				pic = _icons.at("folder");
			else if (!std::regex_search(pathToString(filePath), pattern))
				continue;
			else
				try {
					pic = _icons.at(pathToString(filePath.extension()));
				} catch (std::out_of_range &) {
					pic = _icons.at("");
				}

			auto button = tgui::Button::create("");
			auto picture = tgui::Picture::create(pic);
			auto label = tgui::Label::create(fileStr);
			auto renderer = button->getRenderer();

			button->setPosition(10, pos);
			button->setSize({"&.w - 40", 20});
			button->connect("Clicked", [button, file, fileStr, &open]{
				if (file->getText() == fileStr)
					open(false);
				else
					file->setText(fileStr);
			});
			renderer->setBackgroundColor("transparent");
			renderer->setBackgroundColorFocused("#A0A0A0");
			renderer->setBackgroundColorHover("#808080");
			renderer->setBackgroundColorDown("#303030");
			renderer->setBorderColor("transparent");
			renderer->setBorderColorFocused("#B0B0B0");
			renderer->setBorderColorHover("#909090");
			label->setPosition(40, pos + 2);
			label->ignoreMouseEvents();
			setRenderer(label);
			picture->setPosition(12, pos + 2);
			picture->ignoreMouseEvents();
			panel->add(button);
			panel->add(label);
			panel->add(picture);
			pos += 30;
		}

		auto label = tgui::Label::create();

		label->setPosition(40, pos - 11);
		label->setSize(10, 9);
		panel->add(label);

		panel->setVerticalScrollAmount(30);
		panel->setVerticalScrollbarValue(0);
	}

	std::string cleanPath(const std::string &path)
	{
		std::vector<std::string> files = {};
		std::istringstream iss{path};
		std::string item;

		while (std::getline(iss, item, static_cast<char>(std::filesystem::path::preferred_separator)))
			if (item == "..") {
				if (files.size() > 1)
					files.pop_back();
			} else if (item != "." && !item.empty())
				files.push_back(item);

		return std::accumulate(
			files.begin() + 1,
			files.end(),
#ifdef _WIN32
			files[0],
#else
			static_cast<char>(std::filesystem::path::preferred_separator) + files[0],
#endif
			[](const std::string &a, const std::string &b){
				return a + static_cast<char>(std::filesystem::path::preferred_separator) + b;
			}
		);
	}

	std::string openFileDialog(const std::string &title, const std::string &basePath, const std::vector<std::pair<std::string, std::string>> &patterns, bool overWriteWarning, bool mustExist, bool allowFolder)
	{
#ifdef _WIN32
		if (!allowFolder) {
			std::filesystem::path cwd = std::filesystem::current_path();
			OPENFILENAME ofn;
			char fileName[MAX_PATH];
			char *pattern;
			std::string parent;
			size_t size = strlen("All Files*.*") + 3;
			std::vector<std::pair<std::string, std::string>> pats;
			std::regex reg{R"(\*\.\w+)"};

			for (auto &pat : patterns) {
				std::string result;

				result.reserve(pat.first.size());
				for (int i = 0; i < pat.first.size(); i++) {
					if (pat.first[i] == '*')
						continue;
					if (pat.first[i] == '\\') {
						result += pat.first[++i];
						continue;
					}
					if (pat.first[i] == '.')
						result += '*';
					else
						result += pat.first[i];
				}
				pats.emplace_back(result, pat.second);
			}
			pats.emplace_back("*.*", "All Files");
			for (auto &pat : pats)
				size += strlen(pat.first.c_str()) + strlen(pat.second.c_str()) + 2;
			pattern = new char[size + 1];
			size = 0;
			for (auto &pat : pats) {
				strcpy(pattern + size, pat.second.c_str());
				size += pat.second.size() + 1;
				strcpy(pattern + size, pat.first.c_str());
				size += pat.first.size() + 1;
			}
			pattern[size] = '\0';

			try {
				if (basePath != ".")
					strcpy(fileName, std::filesystem::path(basePath).filename().string().c_str());
				else
					*fileName = 0;
			} catch (...) {
				*fileName = 0;
			}
			ZeroMemory(&ofn, sizeof(OPENFILENAME));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.lpstrFilter = pattern;
			ofn.lpstrFile = fileName;
			for (auto &pat : pats) {
				if (std::regex_search(pat.first, reg)) {
					ofn.lpstrDefExt = pat.first.c_str() + 1;
					break;
				}
			}
			ofn.nMaxFile = MAX_PATH;
			ofn.lpstrTitle = title.c_str();
			try {
				parent = std::filesystem::path(basePath).parent_path().string();
				ofn.lpstrInitialDir = parent.c_str();
			} catch (...) {
				ofn.lpstrInitialDir = nullptr;
			}
			ofn.Flags = OFN_ENABLESIZING | OFN_EXPLORER;
			if (overWriteWarning)
				ofn.Flags |= OFN_OVERWRITEPROMPT;
			if (mustExist)
				ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

			bool r = (overWriteWarning ? GetSaveFileName : GetOpenFileName)(&ofn);

			std::filesystem::current_path(cwd);
			delete[] pattern;
			if (r) {
				std::string result = ofn.lpstrFile;

				if (ofn.nFilterIndex < pats.size() && result.find('.') == std::string::npos && overWriteWarning) {
					auto pat = pats[ofn.nFilterIndex - 1].first;

					result += pat.substr(pat.find_last_of('.'));
				}
				return result;
			}

			auto err = CommDlgExtendedError();

			if (!err)
				return "";
			MessageBoxA(nullptr, ("Error " + std::to_string(err) + " on GetOpenFileName. Please report this.").c_str(), "GetOpenFileName error", MB_ICONERROR);
		}
#endif
		sf::RenderWindow window{{500, 300}, title, sf::Style::Titlebar};
		tgui::Gui gui{window};
		std::string result;
		std::string startText;
		std::filesystem::path currentPath = basePath;
		sf::Event event;

		currentPath = std::filesystem::absolute(currentPath);
		while (!std::filesystem::is_directory(currentPath)) {
			if (!startText.empty())
				startText += std::filesystem::path::preferred_separator;
			startText += pathToString(currentPath.filename());
			currentPath = currentPath.parent_path();
		}

		gui.loadWidgetsFromFile("assets/gui/open_file_dialog.gui");

		auto path = gui.get<tgui::TextBox>("Path");
		auto file = gui.get<tgui::EditBox>("file");
		auto box = gui.get<tgui::ComboBox>("Patterns");
		auto panel = gui.get<tgui::ScrollablePanel>("Folders");
		std::function<void(bool)> open = [&gui, &result, &window, path, box, file, &currentPath, panel, &open, mustExist, overWriteWarning, allowFolder](bool fromButton){
			if (file->getText().isEmpty() && !allowFolder)
				return;

			std::string ext = box->getSelectedItemId();

			if (std::filesystem::path(file->getText()).is_relative())
				result = path->getText() + std::filesystem::path::preferred_separator + file->getText();
			else
				result = file->getText();

			if (std::filesystem::is_directory(result) && (!allowFolder || !fromButton)) {
				result = cleanPath(result);
				currentPath = result + static_cast<char>(std::filesystem::path::preferred_separator);
				path->setText(pathToString(currentPath));
				file->setText("");
				_makeFolders(currentPath, panel, file, open, std::regex(box->getSelectedItemId().toAnsiString(), std::regex_constants::icase), allowFolder);
				return;
			}

			if (mustExist && !std::filesystem::exists(result))
				return;

			if (overWriteWarning && std::filesystem::exists(result)) {
				auto pan = tgui::Panel::create({"100%", "100%"});
				pan->getRenderer()->setBackgroundColor({0, 0, 0, 175});
				gui.add(pan);

				auto win = tgui::ChildWindow::create();
				win->setPosition("(&.w - w) / 2", "(&.h - h) / 2");
				gui.add(win);
				setRenderer(win);

				win->setFocused(true);

				const bool tabUsageEnabled = gui.isTabKeyUsageEnabled();
				auto closeWindow = [&gui, win, pan, tabUsageEnabled]{
					gui.remove(win);
					gui.remove(pan);
					gui.setTabKeyUsageEnabled(tabUsageEnabled);
				};

				pan->connect("Clicked", closeWindow);
				win->connect({"Closed", "EscapeKeyPressed"}, closeWindow);
				win->loadWidgetsFromFile("assets/gui/overwrite_warning.gui");

				auto label = win->get<tgui::Label>("Label");

				label->setText(result + label->getText());
				win->setSize(label->getSize().x + 20, 100);
				win->get<tgui::Button>("Yes")->connect("Clicked", [&window]{
					window.close();
				});
				win->get<tgui::Button>("No")->connect("Clicked", [win]{
					win->close();
				});
				return;
			}

			window.close();
		};

		gui.get<tgui::Button>("Cancel")->connect("Clicked", [&result, &window]{
			result = "";
			window.close();
		});
		gui.get<tgui::Button>("Open")->connect("Clicked", open, true);

		if (overWriteWarning)
			gui.get<tgui::Button>("Open")->setText("Save");
		if (allowFolder) {
			auto o = gui.get<tgui::Button>("Open");
			auto c = gui.get<tgui::Button>("Cancel");

			o->setText(o->getText() + " Folder");
			o->setSize({o->getSize().x + 40, o->getSize().y});
			o->setPosition({o->getPosition().x - 40, o->getPosition().y});
			c->setPosition({c->getPosition().x - 40, c->getPosition().y});
		}

		for (auto &pair : patterns)
			box->addItem(pair.second, pair.first);
		box->addItem("All files", ".*");
		box->setSelectedItemByIndex(0);
		box->connect("ItemSelected", [&currentPath, &panel, &file, &box, &open, allowFolder]{
			_makeFolders(currentPath, panel, file, open, std::regex(box->getSelectedItemId().toAnsiString(), std::regex_constants::icase), allowFolder);
		});

		path->setText(pathToString(currentPath));
		file->setText(startText);
		_makeFolders(currentPath, panel, file, open, std::regex(box->getSelectedItemId().toAnsiString(), std::regex_constants::icase), allowFolder);

		while (window.isOpen()) {
			while (window.pollEvent(event)) {
				if (event.type == sf::Event::Closed) {
					result = "";
					window.close();
				} else if (event.type == sf::Event::Resized) {
					window.setView(sf::View{sf::FloatRect(0, 0, event.size.width, event.size.height)});
					gui.setView(sf::View{sf::FloatRect(0, 0, event.size.width, event.size.height)});
				}
				gui.handleEvent(event);
			}
			window.clear({80, 80, 80});
			gui.draw();
			window.display();
		}
		return result;
	}

	std::string saveFileDialog(const std::string &title, const std::string &basePath, const std::vector<std::pair<std::string, std::string>> &patterns)
	{
		return openFileDialog(title, basePath, patterns, true, false);
	}

	tgui::ChildWindow::Ptr openWindowWithFocus(tgui::Gui &gui, tgui::Layout width, tgui::Layout height)
	{
		auto panel = tgui::Panel::create({"100%", "100%"});

		panel->getRenderer()->setBackgroundColor({0, 0, 0, 175});
		gui.add(panel);

		auto window = tgui::ChildWindow::create();
		window->setSize(width, height);
		window->setPosition("(&.w - w) / 2", "(&.h - h) / 2");
		gui.add(window);

		window->setFocused(true);

		const bool tabUsageEnabled = gui.isTabKeyUsageEnabled();
		auto closeWindow = [&gui, window, panel, tabUsageEnabled]{
			gui.remove(window);
			gui.remove(panel);
			gui.setTabKeyUsageEnabled(tabUsageEnabled);
		};

		setRenderer(window);
		panel->connect("Clicked", closeWindow);
		window->connect({"Closed", "EscapeKeyPressed"}, closeWindow);
		return window;
	}
#else
	void __nothing() {};
	void *__nothing2() { return nullptr; };
	std::string __nothing3() { return "Not implemented"; }
#endif

	sf::Color HSLtoRGB(const HSLColor &color)
	{
		float h = color.h * 360 / 240.f;
		float s = color.s / 240.f;
		float l = color.l / 240.f;
		float c = (1.0f - std::abs(2.0f * l - 1.0f)) * s;
		float x = c * (1.0f - std::abs(std::fmod(h / 60.0f, 2.0f) - 1.0f));
		float m = l - c / 2.0f;
		float r = 0;
		float g = 0;
		float b = 0;

		if ((0.0f <= h && h < 60.0f) || h >= 360.0f) {
			r = c;    g = x;   b = 0.0f;
		} else if (60.0f <= h && h < 120.0f) {
			r = x;    g = c;   b = 0.0f;
		} else if (120.0f <= h && h < 180.0f) {
			r = 0.0f; g = c;   b = x;
		} else if (180.0f <= h && h < 240.0f) {
			r = 0.0f; g = x;   b = c;
		} else if (240.0f <= h && h < 300.0f) {
			r = x;   g = 0.0f; b = c;
		} else if (300.0f <= h && h < 360.0f) {
			r = c;   g = 0.0f; b = x;
		}
		return sf::Color(
			static_cast<sf::Uint8>(std::round((r + m) * 255.0f)),
			static_cast<sf::Uint8>(std::round((g + m) * 255.0f)),
			static_cast<sf::Uint8>(std::round((b + m) * 255.0f))
		);
	}

	HSLColor RGBtoHSL(const sf::Color &color)
	{
		float r = color.r / 255.f;
		float g = color.g / 255.f;
		float b = color.b / 255.f;
		float max = std::max(r, std::max(g, b));
		float min = std::min(r, std::min(g, b));
		float h = 0;
		float s = 0;
		float l = (max + min) / 2;

		if (max == min)
			h = s = 0;
		else {
			float d = max - min;
			s = l > 0.5 ? d / (2 - max - min) : d / (max + min);
			if (max == r)
				h = (g - b) / d + (g < b ? 6 : 0);
			else if (max == g)
				h = (b - r) / d + 2;
			else if (max == b)
				h = (r - g) / d + 4;
			h /= 6;
		}
		return {
			static_cast<unsigned char>(h * 240),
			static_cast<unsigned char>(s * 240),
			static_cast<unsigned char>(l * 240)
		};
	}

#ifndef __ANDROID__
	tgui::ChildWindow::Ptr makeColorPickWindow(tgui::Gui &gui, const std::function<void(sf::Color color)> &onFinish, sf::Color startColor)
	{
		char buffer[8];
		auto window = openWindowWithFocus(gui, 490, 370);

		window->loadWidgetsFromFile("assets/gui/color.gui");
		window->setTitle("Pick color");

		auto light = window->get<tgui::Slider>("Lightness");
		auto satHuePic = window->get<tgui::Picture>("SatHue");
		auto cross = window->get<tgui::Picture>("Cross");
		auto preview = window->get<tgui::TextBox>("Preview");
		auto hue = window->get<tgui::EditBox>("Hue");
		auto saturation = window->get<tgui::EditBox>("Sat");
		auto lightness = window->get<tgui::EditBox>("Light");
		auto edit = window->get<tgui::EditBox>("Edit");
		auto updateTexture = [light](HSLColor color) {
			sf::Image image;
			sf::Texture texture;

			image.create(light->getSize().x, light->getSize().y);

			auto size = image.getSize();

			for (unsigned y = 0; y < size.y; y++) {
				color.l = 240 - (240 * y / size.y);

				auto rgb = HSLtoRGB(color);
				sf::Color sf{rgb.r, rgb.g, rgb.b, 255};

				for (unsigned x = 0; x < size.x; x++)
					image.setPixel(x, y, sf);
			}
			texture.loadFromImage(image);
			light->getRenderer()->setTextureTrack({texture});
		};
		auto buttonCallBack = [edit, satHuePic, preview, light, hue, saturation, lightness, cross](tgui::Button::Ptr button){
			char buffer[8];
			auto c = button->getRenderer()->getBackgroundColor();
			sf::Color temp = {
				c.getRed(),
				c.getGreen(),
				c.getBlue()
			};
			HSLColor color = RGBtoHSL(temp);
			auto pos = satHuePic->getPosition();
			auto size = cross->getSize();

			sprintf(buffer, "#%02X%02X%02X", temp.r, temp.g, temp.b);
			light->setValue(color.l);
			hue->setText(std::to_string(color.h));
			saturation->setText(std::to_string(color.s));
			lightness->setText(std::to_string(color.l));
			cross->setPosition({
				(color.h * 200 / 240) + pos.x - size.x / 2,
				((240 - color.s) * 200 / 240) + pos.y - size.y / 2
			});
			edit->setText(buffer);
			preview->getRenderer()->setBackgroundColor(c);
		};
		auto sliderCallback = [cross, light, lightness, satHuePic, hue, saturation, preview, edit]{
			char buffer[8];
			unsigned char h = (cross->getSize().x / 2 + cross->getPosition().x - satHuePic->getPosition().x) * 240 / 200;
			unsigned char s = 240 - (cross->getSize().y / 2 + cross->getPosition().y - satHuePic->getPosition().y) * 240 / 200;
			HSLColor color = {
				h,
				s,
				static_cast<unsigned char>(light->getValue())
			};
			sf::Color bufferColor = HSLtoRGB(color);

			sprintf(buffer, "#%02X%02X%02X", bufferColor.r, bufferColor.g, bufferColor.b);
			preview->getRenderer()->setBackgroundColor({bufferColor.r, bufferColor.g, bufferColor.b, 255});
			edit->setText(buffer);
			light->setValue(color.l);
			lightness->setText(std::to_string(color.l));
		};

		for (auto &id : window->getWidgetNames())
			if (id.substring(0, strlen("Button")) == "Button") {
				auto button = window->get<tgui::Button>(id);

				button->connect("Clicked", buttonCallBack, button);
			}
		light->connect("ValueChanged", sliderCallback);
		edit->onReturnKeyPress.connect( [cross, hue, saturation, light, lightness, satHuePic, edit, updateTexture]{
			std::string text = edit->getText();

			if (text.size() > 7) {
				edit->setText(text.substr(0, 7));
				return;
			} else if (text.size() != 7)
				return;

			tgui::Color color{edit->getText()};
			sf::Color bufferColor{
				color.getRed(),
				color.getGreen(),
				color.getBlue()
			};
			auto tmp = RGBtoHSL(bufferColor);
			auto pos = satHuePic->getPosition();
			auto size = cross->getSize();

			light->setValue(tmp.l);
			hue->setText(std::to_string(tmp.h));
			saturation->setText(std::to_string(tmp.s));
			lightness->setText(std::to_string(tmp.l));
			cross->setPosition({
				(tmp.h * 200 / 240.f) + pos.x - size.x / 2,
				((240 - tmp.s) * 200 / 240.f) + pos.y - size.y / 2
			});
			updateTexture(tmp);
		});
		hue->onReturnKeyPress.connect([cross, satHuePic, light, hue, saturation, lightness, edit, preview, updateTexture]{
			unsigned char h;

			try {
				auto nbr = std::stoul(hue->getText().toAnsiString());

				if (nbr > 240) {
					hue->setText("0");
					return;
				}
				h = nbr;
			} catch (...) { hue->setText("0"); return; }

			char buffer[8];
			unsigned char s = 240 - (cross->getSize().y / 2 + cross->getPosition().y - satHuePic->getPosition().y) * 240 / 200;
			HSLColor color = {
				h,
				s,
				static_cast<unsigned char>(light->getValue())
			};
			sf::Color temp = HSLtoRGB(color);
			auto pos = satHuePic->getPosition();
			auto size = cross->getSize();

			sprintf(buffer, "#%02X%02X%02X", temp.r, temp.g, temp.b);
			cross->setPosition({
				(h * 200 / 240) + pos.x - size.x / 2,
				cross->getPosition().y
			});
			edit->setText(buffer);
			preview->getRenderer()->setBackgroundColor({buffer});
			updateTexture(color);
		});
		saturation->onReturnKeyPress.connect([cross, satHuePic, light, hue, saturation, lightness, edit, preview, updateTexture]{
			unsigned char s;

			try {
				auto nbr = std::stoul(saturation->getText().toAnsiString());

					if (nbr > 240) {
						saturation->setText("0");
						return;
					}
				s = nbr;
			} catch (...) { saturation->setText("0"); return; }

			char buffer[8];
			unsigned char h = (cross->getSize().x / 2 + cross->getPosition().x - satHuePic->getPosition().x) * 240 / 200;
			HSLColor color = {
				h,
				s,
				static_cast<unsigned char>(light->getValue())
			};
			sf::Color temp = HSLtoRGB(color);
			auto pos = satHuePic->getPosition();
			auto size = cross->getSize();

			sprintf(buffer, "#%02X%02X%02X", temp.r, temp.g, temp.b);
			cross->setPosition({
				(color.h * 200 / 240) + pos.x - size.x / 2,
				((240 - color.s) * 200 / 240) + pos.y - size.y / 2
			});
			edit->setText(buffer);
			preview->getRenderer()->setBackgroundColor({buffer});
			updateTexture(color);
		});
		lightness->onReturnKeyPress.connect([cross, satHuePic, light, hue, saturation, lightness, edit, preview]{
			unsigned char l;

			try {
				auto nbr = std::stoul(lightness->getText().toAnsiString());

				if (nbr > 240) {
					lightness->setText("0");
					return;
				}
				l = nbr;
			} catch (...) { lightness->setText("0"); return; }

			char buffer[8];
			unsigned char h = (cross->getSize().x / 2 + cross->getPosition().x - satHuePic->getPosition().x) * 240 / 200;
			unsigned char s = 240 - (cross->getSize().y / 2 + cross->getPosition().y - satHuePic->getPosition().y) * 240 / 200;
			HSLColor color = {
				h,
				s,
				l
			};
			sf::Color temp = HSLtoRGB(color);

			sprintf(buffer, "#%02X%02X%02X", temp.r, temp.g, temp.b);
			light->setValue(l);
			edit->setText(buffer);
			preview->getRenderer()->setBackgroundColor({buffer});
		});

		HSLColor color = RGBtoHSL(startColor);
		auto pos = satHuePic->getPosition();
		auto size = cross->getSize();

		sprintf(buffer, "#%02X%02X%02X", startColor.r, startColor.g, startColor.b);
		light->setValue(color.l);
		hue->setText(std::to_string(color.h));
		saturation->setText(std::to_string(color.s));
		lightness->setText(std::to_string(color.l));
		cross->setPosition({
			(color.h * 200 / 240) + pos.x - size.x / 2,
			((240 - color.s) * 200 / 240) + pos.y - size.y / 2
		});
		edit->setText(buffer);
		preview->getRenderer()->setBackgroundColor({buffer});
		updateTexture(color);

		cross->ignoreMouseEvents();

		auto colorPickHandler = [cross, satHuePic, light, hue, saturation, edit, preview, updateTexture](tgui::Vector2f pos){
			char buffer[8];
			unsigned char h = pos.x * 240 / 200;
			unsigned char s = 240 - pos.y * 240 / 200;
			HSLColor color = {
				h,
				s,
				static_cast<unsigned char>(light->getValue())
			};
			sf::Color temp = HSLtoRGB(color);
			auto size = cross->getSize();

			sprintf(buffer, "#%02X%02X%02X", temp.r, temp.g, temp.b);
			hue->setText(std::to_string(color.h));
			saturation->setText(std::to_string(color.s));
			cross->setPosition({
				(color.h * 200 / 240) + satHuePic->getPosition().x - size.x / 2,
				((240 - color.s) * 200 / 240) + satHuePic->getPosition().y - size.y / 2
			});
			edit->setText(buffer);
			preview->getRenderer()->setBackgroundColor({buffer});
			updateTexture(color);
		};

		satHuePic->onMousePress.connect(colorPickHandler);
		satHuePic->onMouseRelease.connect(colorPickHandler);

		window->get<tgui::Button>("Cancel")->connect("Clicked", [window]{
			window->close();
		});
		window->get<tgui::Button>("OK")->connect("Clicked", [onFinish, edit, window]{
			tgui::Color color{edit->getText()};
			sf::Color bufferColor{
				color.getRed(),
				color.getGreen(),
				color.getBlue(),
				255
			};

			if (onFinish)
				onFinish(bufferColor);
			window->close();
		});
		return window;
	}

	tgui::ChildWindow::Ptr makeSliderWindow(tgui::Gui &gui, const std::function<void(float value)> &onFinish, float defaultValue, float min, float max, float step)
	{
		auto window = openWindowWithFocus(gui, 260, 80);
		window->loadWidgetsFromFile("assets/gui/slider.gui");

		auto slider = window->get<tgui::Slider>("Slider");
		auto txt = window->get<tgui::Label>("Nb");
		auto sliderCallback = [slider, txt]{
			std::stringstream s;

			s << slider->getValue();
			txt->setText(s.str());
		};
		std::stringstream s;

		slider->setMaximum(min);
		slider->setMaximum(max);
		slider->setStep(step);
		slider->setValue(defaultValue);
		slider->connect("ValueChanged", sliderCallback);

		s << slider->getValue();
		txt->setText(s.str());

		window->get<tgui::Button>("Cancel")->connect("Clicked", [window]{
			window->close();
		});
		window->get<tgui::Button>("Ok")->connect("Clicked", [onFinish, slider, window]{
			if (onFinish)
				onFinish(slider->getValue());
			window->close();
		});
		return window;
	}
#endif
}

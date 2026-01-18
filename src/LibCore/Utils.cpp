//
// Created by PinkySmile on 07/04/2020.
//

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/ptrace.h>
#endif
#ifdef __GNUG__
#include <cxxabi.h>
#endif
#include <regex>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <numeric>
#include <fstream>
#include <cmath>
#include "Resources/Game.hpp"
#include "Utils.hpp"

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

namespace SpiralOfFate::Utils
{
	static const std::unordered_map<std::string, std::string> _icons{
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

#ifdef USE_TGUI
	tgui::FileDialog::Ptr openFileDialog(tgui::Gui &gui, const std::string &title, const std::filesystem::path &basePath, bool overWriteWarning, bool mustExist)
	{
		auto dialog = tgui::FileDialog::create(title, overWriteWarning ? "Save" : "Open", overWriteWarning);

		dialog->setFileMustExist(mustExist);
		dialog->setPath(tgui::Filesystem::Path(absolute(basePath)));
		openWindowWithFocus(gui, 0, 0, dialog);
		return dialog;
	}

	tgui::FileDialog::Ptr saveFileDialog(tgui::Gui &gui, const std::string &title, const std::filesystem::path &basePath)
	{
		return openFileDialog(gui, title, basePath, true, false);
	}
#else
#ifdef USE_SDL
	int dispMsg(const std::string &title, const std::string &content, int variate, Screen *win)
	{
		SDL_ShowSimpleMessageBox(variate, title.c_str(), content.c_str(), win ? win->getSDLWindow() : nullptr);
		return 0;
	}
#endif
	void __nothing() {};
	void *__nothing2() { return nullptr; };
	std::filesystem::path __nothing3() { return "Not implemented"; }
#endif

#ifdef USE_TGUI
	void setRenderer(tgui::Container *widget)
	{
		setRenderer(*widget);
	}

	void setRenderer(tgui::Container &widget)
	{
		auto renderer = tgui::Theme::getDefault()->getRendererNoThrow(widget.getWidgetType());

		if (renderer)
			widget.setRenderer(renderer);
		for (auto &w : widget.getWidgets()) {
			try {
				if (!w->getUserData<bool>())
					continue;
			} catch (std::bad_any_cast &) {}
			if (auto c = w->cast<tgui::Container>())
				Utils::setRenderer(c);
			else
				Utils::setRenderer(w);
		}
	}

	void setRenderer(const tgui::Gui &widget)
	{
		for (auto &w : widget.getWidgets()) {
			try {
				if (!w->getUserData<bool>())
					continue;
			} catch (std::bad_any_cast &) {}
			if (auto c = w->cast<tgui::Container>())
				Utils::setRenderer(c);
			else
				Utils::setRenderer(w);
		}
	}

	template<>
	void setRenderer(const tgui::Container::Ptr &widget)
	{
		Utils::setRenderer(*widget);
	}

	std::string getLocale()
	{
#ifdef _WIN32
		char locale[LOCALE_NAME_MAX_LENGTH] = {0};
		wchar_t localw[LOCALE_NAME_MAX_LENGTH] = {0};

		GetUserDefaultLocaleName(localw, sizeof(localw));
		for (int i = 0; localw[i]; i++)
			locale[i] = localw[i];

		auto end = strchr(locale, '-');

		if (end)
			*end = 0;
		return locale;
#else
		char locale[1024] = {0};
		char *lang = getenv("LANG");

		if (!lang)
			strcpy(locale, "en_EN");
		else
			strncpy(locale, lang, sizeof(locale) - 1);

		auto end = strchr(locale, '_');

		if (end)
			*end = 0;
		return locale;
#endif
	}
#endif

#ifndef _WIN32
	// https://forum.juce.com/t/detecting-if-a-process-is-being-run-under-a-debugger/2098
	bool isBeingDebugged()
	{
		static bool isCheckedAlready = false;
		static bool underDebugger = false;

		if (!isCheckedAlready) {
			if (ptrace(PTRACE_TRACEME, 0, 1, 0) < 0)
				underDebugger = true;
			else
				ptrace(PTRACE_DETACH, 0, 1, 0);
			isCheckedAlready = true;
		}
		return underDebugger;
	}
#else
	bool isBeingDebugged()
	{
	     return IsDebuggerPresent() == TRUE;
	}
#endif

	unsigned char *allocateManually(size_t size)
	{
		return new unsigned char[size];
	}
	void deallocateManually(unsigned char *buffer)
	{
		delete[] buffer;
	}

	std::string toHex(const unsigned char *buffer, size_t size, bool endian)
	{
		char digits[] = "0123456789ABCDEF";
		std::string result;

		result.resize(size * 2);
		for (size_t i = 0; i < size; i++) {
			size_t index = endian == TOHEX_ENDIAN_BIG ? i : size - i - 1;
			unsigned char c = buffer[index];

			result[i * 2 + 0] = digits[c >> 4];
			result[i * 2 + 1] = digits[c & 0xF];
		}
		return result;
	}
}

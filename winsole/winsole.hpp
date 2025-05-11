#pragma once

#include <windows.h>
#include <cstdio>
#include <cwchar>
#include <string>
#include "winapi_tools.hpp"

enum Color {
    BLACK, BLUE, GREEN, AQUA, RED, PURPLE, YELLOW, WHITE,
    GRAY, GREY = GRAY,
    LIGHT_BLUE, LIGHT_GREEN, LIGHT_AQUA, LIGHT_RED,
    LIGHT_PURPLE, LIGHT_YELLOW, LIGHT_WHITE,
    AUTO
};

inline const char* Color_cstr(Color color) {
    switch(color) {
        case BLACK: return "BLACK"; case BLUE: return "BLUE";
        case GREEN: return "GREEN"; case AQUA: return "AQUA";
        case RED: return "RED"; case PURPLE: return "PURPLE";
        case YELLOW: return "YELLOW"; case WHITE: return "WHITE";
        case GRAY: case GREY: return "GRAY";
        case LIGHT_BLUE: return "LIGHT_BLUE"; case LIGHT_GREEN: return "LIGHT_GREEN";
        case LIGHT_AQUA: return "LIGHT_AQUA"; case LIGHT_RED: return "LIGHT_RED";
        case LIGHT_PURPLE: return "LIGHT_PURPLE"; case LIGHT_YELLOW: return "LIGHT_YELLOW";
        case LIGHT_WHITE: return "LIGHT_WHITE"; case AUTO: return "AUTO";
        default: return "UNKNOWN";
    }
}

struct COLORS {
    Color fore = AUTO;
    Color back = AUTO;
};

using cwstr = const wchar_t*;

inline std::wstring wide(const char* cstr) {
    int len = MultiByteToWideChar(CP_UTF8, 0, cstr, -1, nullptr, 0);
    std::wstring wstr(len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, cstr, -1, &wstr[0], len);
    wstr.pop_back(); // eliminar terminador nulo
    return wstr;
}

class Font {
public:
    Font() = default;
    Font(const Font& copy) = default;
    Font(Font&& move) noexcept = default;

    bool init(HANDLE handle, bool max_window = false);
    COORD get_size() const;
    cwstr get_font() const;
    int get_font_family() const;

    void set_size(const COORD& size);
    void set_font(cwstr font_name, UINT weight = FW_NORMAL);
    void set_font(const char* font_name, UINT weight = FW_NORMAL);
    bool update();

private:
    HANDLE handle = nullptr;
    bool max_window = false;
    CONSOLE_FONT_INFOEX info = { sizeof(CONSOLE_FONT_INFOEX) };
};

inline bool Font::init(HANDLE handle, bool max_window) {
    this->handle = handle;
    this->max_window = max_window;
    info.cbSize = sizeof(info);
    return GetCurrentConsoleFontEx(handle, max_window, &info);
}

inline COORD Font::get_size() const {
    return GetConsoleFontSize(handle, info.nFont);
}

inline cwstr Font::get_font() const {
    return info.FaceName;
}

inline int Font::get_font_family() const {
    return info.FontFamily;
}

inline void Font::set_size(const COORD& size) {
    info.dwFontSize = size;
}

inline void Font::set_font(cwstr font_name, UINT weight) {
    info.FontWeight = weight;
    wcsncpy_s(info.FaceName, font_name, LF_FACESIZE);
}

inline void Font::set_font(const char* font_name, UINT weight) {
    std::wstring wname = wide(font_name);
    set_font(wname.c_str(), weight);
}

inline bool Font::update() {
    return SetCurrentConsoleFontEx(handle, max_window, &info);
}

class Winsole {
public:
    Winsole() = default;
    Winsole(const Winsole&) = default;
    Winsole(Winsole&&) noexcept = default;
    ~Winsole();

    bool init();
    COORD get_max_raw_size() const;
    const SMALL_RECT& get_raw_size() const;
    const COORD& get_size() const;

    Color get_foreground() const;
    Color get_background() const;
    COLORS get_colors() const;

    void set_size(const COORD& size);
    void set_raw_size(const SMALL_RECT& size);
    bool set_colors(const COLORS& colors);

    void put(char c, const COLORS& colors);
    void print(const char* cstr, const COLORS& colors);
    bool clear();
    bool update();

    HANDLE get_handle() const;

private:
    HANDLE handle = nullptr;
    CONSOLE_SCREEN_BUFFER_INFOEX info = { sizeof(CONSOLE_SCREEN_BUFFER_INFOEX) };
};

inline Winsole::~Winsole() {
    if (handle && handle != INVALID_HANDLE_VALUE) {
        FreeConsole();
    }
}

inline bool Winsole::init() {
    handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (handle == INVALID_HANDLE_VALUE)
        return false;

    info.cbSize = sizeof(info);
    if (!GetConsoleScreenBufferInfoEx(handle, &info))
        return false;

    info.srWindow.Bottom += 1;
    info.srWindow.Right += 1;
    info.dwSize.Y = info.srWindow.Bottom;
    info.dwSize.X = info.srWindow.Right;
    return true;
}

inline HANDLE Winsole::get_handle() const {
    return handle;
}

inline COORD Winsole::get_max_raw_size() const {
    return GetLargestConsoleWindowSize(handle);
}

inline const SMALL_RECT& Winsole::get_raw_size() const {
    return info.srWindow;
}

inline const COORD& Winsole::get_size() const {
    return info.dwMaximumWindowSize;
}

inline Color Winsole::get_foreground() const {
    return static_cast<Color>(info.wAttributes & 0x0F);
}

inline Color Winsole::get_background() const {
    return static_cast<Color>((info.wAttributes & 0xF0) >> 4);
}

inline COLORS Winsole::get_colors() const {
    return { get_foreground(), get_background() };
}

inline void Winsole::set_size(const COORD& size) {
    info.dwMaximumWindowSize = size;
}

inline void Winsole::set_raw_size(const SMALL_RECT& size) {
    info.srWindow = size;
    info.dwSize.X = size.Right;
    info.dwSize.Y = size.Bottom;
}

inline bool Winsole::set_colors(const COLORS& colors) {
    WORD attrs = info.wAttributes;
    if (colors.fore != AUTO) attrs = (attrs & 0xF0) | colors.fore;
    if (colors.back != AUTO) attrs = (attrs & 0x0F) | (colors.back << 4);
    return SetConsoleTextAttribute(handle, attrs);
}

inline void Winsole::put(char c, const COLORS& colors) {
    COLORS last = get_colors();
    if(set_colors(colors)) putc(c, stdout);
    set_colors(last);
}

inline void Winsole::print(const char* cstr, const COLORS& colors) {
    COLORS last = get_colors();
    set_colors(colors);
    DWORD written;
    WriteConsoleA(handle, cstr, static_cast<DWORD>(strlen(cstr)), &written, nullptr);
    set_colors(last);
}

inline bool Winsole::clear() {
    COORD topLeft = {0, 0};
    DWORD written;
    DWORD size = info.dwSize.X * info.dwSize.Y;

    if (!FillConsoleOutputCharacter(handle, L' ', size, topLeft, &written)) return false;
    if (!FillConsoleOutputAttribute(handle, info.wAttributes, size, topLeft, &written)) return false;
    return SetConsoleCursorPosition(handle, topLeft);
}

inline bool Winsole::update() {
    return SetConsoleScreenBufferInfoEx(handle, &info);
}

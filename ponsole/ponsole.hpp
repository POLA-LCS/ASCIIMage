#pragma once
#define PONSOLE_HPP 1

#include <windows.h>
#include <cstdio>
#include <cwchar>
#include "winapi_tools.hpp"

/* ### Console color index references
*  If colormap changes:
   e.g ```BLACK``` may be ```any color``` or
   e.g ```LIGHT_AQUA``` references to ```LIGHT_BLUE``` color
*/
enum Color {
    BLACK,
    BLUE,
    GREEN,
    AQUA,
    RED,
    PURPLE,
    YELLOW,
    WHITE,
    GRAY,
    GREY = GRAY,
    LIGHT_BLUE,
    LIGHT_GREEN,
    LIGHT_AQUA,
    LIGHT_RED,
    LIGHT_PURPLE,
    LIGHT_YELLOW,
    LIGHT_WHITE,
    AUTO
};

const char* Color_cstr(Color color) {
    switch(color) {
        case BLACK  :       return "BLACK";
        case BLUE   :       return "BLUE";
        case GREEN  :       return "GREEN";
        case AQUA   :       return "AQUA";
        case RED    :       return "RED";
        case PURPLE :       return "PURPLE";
        case YELLOW :       return "YELLOW";
        case WHITE  :       return "WHITE";
        case GRAY   :       return "GRAY";
        case LIGHT_BLUE   : return "LIGHT_BLUE";
        case LIGHT_GREEN  : return "LIGHT_GREEN";
        case LIGHT_AQUA   : return "LIGHT_AQUA";
        case LIGHT_RED    : return "LIGHT_RED";
        case LIGHT_PURPLE : return "LIGHT_PURPLE";
        case LIGHT_YELLOW : return "LIGHT_YELLOW";
        case LIGHT_WHITE  : return "LIGHT_WHITE";
        case AUTO   :       return "AUTO";
        default     :       return "UNKNOWN";
    }
    return "UNKNOWN";
}

/* ### Console cell colors
```fore(ground)``` and ```back(ground)```
*/
struct COLORS {
    Color fore = AUTO;
    Color back = AUTO;
};

// ```Const Wchar_t STRing```
typedef const wchar_t* cwstr;

// ```cstr``` -> ```cwstr```
cwstr wide(const char* cstr) {
    int len = MultiByteToWideChar(CP_UTF8, 0, cstr, -1, nullptr, 0);
    wchar_t* wide_string = new wchar_t[len];
    MultiByteToWideChar(CP_UTF8, 0, cstr, -1, wide_string, len);
    return wide_string;
}

/* ### Console's font handler class
*  Can be modified 
*/
class Font {
public:
    Font() = default;
    Font(const Font& copy);
    Font(Font&& move) noexcept;
    
    bool init(HANDLE handle, bool max_window = false);
    COORD get_size() const;
    const wchar_t* get_font() const;
    int get_font_family() const;

    void set_size(const COORD& size);
    void set_font(cwstr font_name, UINT weight = FW_NORMAL);
    void set_font(const char* font_name, UINT weight = FW_NORMAL);
    bool update();

private:
    HANDLE handle;
    bool max_window;
    CONSOLE_FONT_INFOEX info;
};

Font::Font(const Font& copy) : handle(copy.handle), info(copy.info) {}

Font::Font(Font&& move) noexcept : handle(move.handle), info(move.info) {
    move.handle = nullptr;
}

bool Font::init(HANDLE handle, bool max_window) {
    this->handle = handle;
    if(this->handle == nullptr) {
        fprintf(stderr, "Font handle is null.\n");
        return false;
    }

    this->max_window = max_window;
    info = {sizeof(info)};
    if(!GetCurrentConsoleFontEx(handle, max_window, &info)) {
        fprintf(stderr, "Cannot get the console font.\n");
        return false;
    }

    return true;
}

// Font getters
COORD Font::get_size() const {
    return GetConsoleFontSize(handle, info.nFont);
}

const wchar_t* Font::get_font() const {
    return info.FaceName;
}

int Font::get_font_family() const {
    return info.FontFamily;
}

// Font setters
void Font::set_size(const COORD& size) {
    info.dwFontSize = size;
}

void Font::set_font(cwstr font_name, UINT weight) {
    info.FontWeight = weight;
    wcscpy(info.FaceName, font_name);
}

void Font::set_font(const char* font_name, UINT weight) {
    set_font(wide(font_name), weight);
}

bool Font::update() {
    return SetCurrentConsoleFontEx(handle, max_window, &info);
}

// Console class to handle the console, needs to be Console::init
class Console {
public:
    Console() = default;
    Console(const Console& copy);
    Console(Console&& move) noexcept;
    ~Console();
    
    bool init();
    COORD get_max_raw_size() const;
    const SMALL_RECT& get_raw_size() const;
    const COORD& get_size() const;

    Color get_foreground() const;
    Color get_background() const;
    const COLORS get_colors() const;

    void set_size(const COORD& size);
    void set_raw_size(const SMALL_RECT& size);
    bool set_colors(const COLORS& colors);

    void put(char c, const COLORS& colors);
    void print(const char* cstr, const COLORS& colors);
    bool clear();
    bool update();

    HANDLE get_handle() const;
private:
    HANDLE handle;
    CONSOLE_SCREEN_BUFFER_INFOEX info;
};

bool Console::init() {
    handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if(handle == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Cannot get the console handle.\n");
        return false;
    }

    info = {sizeof(info)};
    if(!GetConsoleScreenBufferInfoEx(handle, &info)) {
        fprintf(stderr, "Cannot get the console buffer info.\n");
        return false;
    }

    info.srWindow.Bottom += 1;
    info.srWindow.Right += 1;
    info.dwSize.Y = info.srWindow.Bottom;
    info.dwSize.X = info.srWindow.Right;
    return true;
}

Console::~Console() {
    if(handle != INVALID_HANDLE_VALUE) {
        FreeConsole();
    }
}

Console::Console(const Console& copy) : handle(copy.handle), info(copy.info) {}

Console::Console(Console&& move) noexcept : handle(move.handle), info(move.info) {
    move.handle = nullptr;
}

// Console getters
HANDLE Console::get_handle() const {
    return handle;
}

COORD Console::get_max_raw_size() const {
    return GetLargestConsoleWindowSize(handle);
}

const SMALL_RECT& Console::get_raw_size() const {
    return info.srWindow;
}

const COORD& Console::get_size() const {
    return info.dwMaximumWindowSize;
}

Color Console::get_foreground() const {
    return (Color)(info.wAttributes & 0x0F);
}

Color Console::get_background() const {
    return (Color)(info.wAttributes & 0xF0);
}

const COLORS Console::get_colors() const {
    return { get_foreground(), get_background() };
}

// Console setters

void Console::set_size(const COORD& size) {
    info.dwMaximumWindowSize = size;
}

void Console::set_raw_size(const SMALL_RECT& size) {
    info.srWindow = size;
    info.dwSize.X = info.srWindow.Right;
    info.dwSize.Y = info.srWindow.Bottom;
}

bool Console::set_colors(const COLORS& colors) {
    WORD new_attributes = (colors.fore != AUTO) ? colors.fore : (info.wAttributes & 0x0F);
    new_attributes |= (colors.back != AUTO) ? (colors.back << 4) : (info.wAttributes & 0xF0);

    return SetConsoleTextAttribute(handle, new_attributes);
}

// Console methods
void Console::put(const char c, const COLORS& colors) {
    COLORS last = get_colors();
    set_colors(colors);
    putc(c, stdout);
    set_colors(last);
}

void Console::print(const char* cstr, const COLORS& colors) {
    COLORS last = get_colors();
    set_colors(colors);
    WriteConsole(handle, cstr, strlen(cstr), nullptr, nullptr);
    set_colors(last);
}

bool Console::clear() {
    COORD topLeft = {0, 0};
    DWORD charsWritten;
    DWORD consoleSize = info.dwSize.X * info.dwSize.Y;

    if(!FillConsoleOutputCharacter(handle, ' ', consoleSize, topLeft, &charsWritten)) {
        fprintf(stderr, "Error: FillConsoleOutputCharacter failed.\n");
        return false;
    }

    if(!FillConsoleOutputAttribute(handle, info.wAttributes, consoleSize, topLeft, &charsWritten)) {
        fprintf(stderr, "Error: FillConsoleOutputAttribute failed.\n");
        return false;
    }

    if(!SetConsoleCursorPosition(handle, topLeft)) {
        fprintf(stderr, "Error: SetConsoleCursorPosition failed.\n");
        return false;
    }

    return true;
}

bool Console::update() {
    return SetConsoleScreenBufferInfoEx(handle, &info);
}
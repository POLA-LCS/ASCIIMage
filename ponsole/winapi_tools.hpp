#ifndef WINAPI_HPP
#define WINAPI_HPP 1
#include <windows.h>
#include <stdio.h>

typedef struct _CONSOLE_SCREEN_BUFFER_INFOEX {
    ULONG cbSize;
    COORD dwSize;
    COORD dwCursorPosition;
    WORD wAttributes;
    SMALL_RECT srWindow;
    COORD dwMaximumWindowSize;
    WORD wPopupAttributes;
    BOOL bFullscreenSupported;
    COLORREF ColorTable[16];
} CONSOLE_SCREEN_BUFFER_INFOEX, *PCONSOLE_SCREEN_BUFFER_INFOEX;

const char* rect_cstr(const SMALL_RECT& rect) {
    char* buffer = new char[41];
    sprintf(buffer, "(%hi, %hi, %hi, %hi)", rect.Left, rect.Top, rect.Right, rect.Bottom);
    return buffer;
}

const char* coord_cstr(const COORD& coord) {
    char* buffer = new char[21];
    sprintf(buffer, "(%hi, %hi)", coord.X, coord.Y);
    return buffer;
}

#ifdef __cplusplus
extern "C" {
#endif

BOOL WINAPI GetConsoleScreenBufferInfoEx(
    HANDLE hConsoleOutput,
    PCONSOLE_SCREEN_BUFFER_INFOEX lpConsoleScreenBufferInfoEx
);

BOOL WINAPI SetConsoleScreenBufferInfoEx(
    HANDLE hConsoleOutput,
    PCONSOLE_SCREEN_BUFFER_INFOEX lpConsoleScreenBufferInfoEx
);

// BOOL WINAPI SetConsoleWindowInfo(
//         HANDLE     hConsoleOutput,
//         BOOL       bAbsolute,
//   const SMALL_RECT *lpConsoleWindow
// );

COORD WINAPI GetLargestConsoleWindowSize(
    HANDLE hConsoleOutput
);

// --- FONT ---

typedef struct _CONSOLE_FONT_INFOEX {
    ULONG cbSize;
    DWORD nFont;
    COORD dwFontSize;
    UINT FontFamily;
    UINT FontWeight;
    WCHAR FaceName[LF_FACESIZE];
} CONSOLE_FONT_INFOEX, *PCONSOLE_FONT_INFOEX;

BOOL WINAPI GetCurrentConsoleFontEx(
    HANDLE               hConsoleOutput,
    BOOL                 bMaximumWindow,
    PCONSOLE_FONT_INFOEX lpConsoleCurrentFontEx
);

BOOL WINAPI SetCurrentConsoleFontEx(
    HANDLE               hConsoleOutput,
    BOOL                 bMaximumWindow,
    PCONSOLE_FONT_INFOEX lpConsoleCurrentFontEx
);

COORD WINAPI GetConsoleFontSize(
    HANDLE hConsoleOutput,
    DWORD nFont
);

#ifdef __cplusplus
}
#endif

#endif//WINAPI_HPP
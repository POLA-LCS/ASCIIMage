# Ponsole v1.2

`Ponsole` is a C++ header that focus on a simple console management with WinAPI.

## Dependencies
- Windows OS.

## How to install
1. Copy the file `ponsole.hpp` in your project.
2. Make sure to include the file `winapi_tools.hpp` in the same directory.

TIP: You can do:  
```
Ponsole/  
| ponsole.hpp  
| winapi_tools.hpp  
```
and then `#include "Ponsole/ponsole.hpp"` 

## Characteristics

### Color references
`Color` is an enum which each color is an index of the actual console color palette.  
This means that for example `Color::BLACK = 0` it's just the first color, not the `BLACK` color.  
  
- **Default palette:** for the time being there's no way to modify the color palette.

### Font
**Acts as a console font handler**.  
You need to provide the memory of a `HANDLE` which can be the same as a `Console` instance.  
**You can change:**
- Width (if supported)
- Height
- Face name
- Weight

### Console
Offers methods to:
- Configure and console window properties.
- Modify text and background color.
- Print.
- Clear.
- Update (`Console::update()` in case of buffer manipulation).

## Usage example

```cpp
#include "ponsole.hpp"

int main() {
    Console console;
    if(!console.init()) {
        return 1;
    }

    Font font;
    // Uses the same handle of the console
    if(!font.init(console.get_handle())) {
        return 2;
    }

    // Save last font size
    COORD last_font_size = font.get_size();
    
    // If supported font would be 24x24. If not ??x24
    font.set_size({24, 24});
    font.update();

    // Prints "Green text!" in LIGHT_GREEN text and BLACK background
    console.print("Green text!", {Color::LIGHT_GREEN, Color::BLACK});
    
    getch();

    // Returns the font to the normality and clears the console
    font.set_size(last_font_size);
    font.update();
    console.clear();

    return 0;
}
```
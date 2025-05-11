#include <stdio.h>
#include <stdint.h>
#include <string>
#include <vector>
#include "winsole/winsole.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

typedef unsigned char byte;

struct RGBA {
    byte r, g, b, a = 255;

    byte max_value() const { return (r > g ? (r > b ? r : b) : (g > b ? g : b)); }
    byte min_value() const { return (r < g ? (r < b ? r : b) : (g < b ? g : b)); }
    float average() const { return (r + g + b) / 3.0f; }
};

struct Image {
    std::string path;
    int width, height, bpp, channels;
    byte* data = nullptr;

    Image() = default;

    Image(std::string path, int channels = 3) : path(path), channels(channels) {}

    size_t size() const { return width * height * channels; }
    size_t image_size() const { return width * height; }

    bool read(std::string rpath = "", int rchannels = 0) {
        if(rchannels < 3 || rchannels > 4) rchannels = channels;
        if(rpath.empty()) rpath = path;
        if(data) stbi_image_free(data);

        data = stbi_load(rpath.c_str(), &width, &height, &bpp, rchannels);
        if (!data) return false;

        channels = rchannels;
        return true;
    }

    void get_color_array(RGBA*& color_array) const {
        if (color_array) delete[] color_array;
        color_array = new RGBA[image_size()];
        for (size_t b = 0, color = 0; b < size(); b += channels)
            color_array[color++] = {data[b], data[b + 1], data[b + 2], (channels == 4) ? data[b + 3] : 255};
    }

    ~Image() {
        if (data) stbi_image_free(data);
    }
};

float map(float input, float x1, float x2, float y1, float y2) {
    return y1 + (input - x1) * (y2 - y1) / (x2 - x1);
}

std::string ascii_image(const Image& image, RGBA*& colors, const std::string& ascii_map) {
    std::string ascii_output;
    ascii_output.reserve(image.image_size() + image.height); // '\n's
    for(size_t ci = 0, row = 1; ci < image.image_size(); ci++) {
        byte grey = (colors[ci].max_value() + colors[ci].min_value()) / 2;
        byte index = map(grey, 0, 255, 0, ascii_map.length() - 1);
        if(ci >= (row * image.width)) {
            ascii_output += '\n';
            row++;
        }
        ascii_output += ascii_map[index];
    }
    return ascii_output;
}

void print_color_image_fast(Winsole& winsole, const Image& image, RGBA*& colors, const std::vector<Color>& colormap) {
    size_t w = image.width, h = image.height;
    std::vector<WinPixel> pixels;
    pixels.reserve(w * h);

    size_t map_size = colormap.size();

    for(size_t i = 0; i < image.image_size(); ++i) {
        byte grey = (colors[i].max_value() + colors[i].min_value()) / 2;
        Color color = colormap[static_cast<size_t>(map(grey, 0, 255, 0, map_size - 1))];
        pixels.push_back({' ', {AUTO, color}});
    }

    for(size_t y = 0; y < h; ++y)
        winsole.put_line(0, y, &pixels[y * w], w);
}

void print_color_ascii_fast(Winsole& winsole, const std::string& ascii_image, RGBA*& colors, const Image& image, const std::vector<Color>& colormap) {
    size_t w = image.width, h = image.height;
    std::vector<WinPixel> pixels;
    pixels.reserve(w * h);

    size_t ci = 0;
    size_t map_size = colormap.size();

    for(char ch : ascii_image) {
        if(ch == '\n') continue;
        byte grey = (colors[ci].max_value() + colors[ci].min_value()) / 2;
        Color color = colormap[static_cast<size_t>(map(grey, 0, 255, 0, map_size - 1))];
        pixels.push_back({ch, {color, AUTO}});
        ci++;
    }

    for(size_t y = 0; y < h; ++y)
        winsole.put_line(0, y, &pixels[y * w], w);
}

void fast_print(const Winsole& console, const std::string& buffer) {
    WriteConsole(console.get_handle(), buffer.c_str(), buffer.length(), nullptr, nullptr);
}

#define version_message "AsciiMage v1.1 (May 2025)\n\n"
#define DEFAULT_ASCII " ._-3#@"
#define DEFAULT_COLOR_MAP {BLACK, BLACK, GREY, GREY, BLUE, LIGHT_BLUE, AQUA, LIGHT_AQUA, WHITE, WHITE}

void print_help() {
    printf(version_message);
    printf("[USAGE]\n");
    printf("    asciimage [--help]                Display this message.\n");
    printf("    asciimage <input> <mode> [map]    Prints an image in the selected mode.\n");
    printf("\n[MODES]\n");
    printf("    ASCII    Prints ASCII version fast.\n");
    printf("    COLOR    Colored image (optimized).\n");
    printf("    ASCOL    ASCII+color (optimized).\n");
    printf("\n[MAPS]\n");
    printf("    ASCII mode: single string map.\n");
    printf("    COLOR/ASCOL: ASCII map + color palette string (e.g., \"0193BF\").\n");
}

int main(int argc, char* argv[]) {
    argc -= 1;
    if(!argc) { print_help(); return 0; }

    std::string str_args[argc];
    for(size_t i = 0; i < argc; i++) {
        str_args[i] = argv[i + 1];
        if(str_args[i] == "-h" || str_args[i] == "--help") { print_help(); return 0; }
    }

    Winsole console;
    if(!console.init()) {
        fprintf(stderr, "[!] Failed to init console.\n");
        return 1;
    }

    std::string input_path = str_args[0];
    std::string ascii_map, color_map;

    Image input_image(input_path.c_str());
    if(!input_image.read()) {
        fprintf(stderr, "[!] Failed to read image.\n");
        return 1;
    }

    RGBA* colors = nullptr;
    input_image.get_color_array(colors);
    if(!colors) {
        fprintf(stderr, "[!] Failed to get color array.\n");
        return 1;
    }

    if(argc == 1) str_args[1] = "ASCII";

    if(str_args[1] == "ASCII") {
        ascii_map = (argc == 2) ? DEFAULT_ASCII : str_args[2];
        std::string ascii_output = ascii_image(input_image, colors, ascii_map);
        fast_print(console, ascii_output);
        delete[] colors;
        return 0;
    }

    std::vector<Color> colormap;
    if(argc == 2) {
        colormap = DEFAULT_COLOR_MAP;
    } else {
        for(char ch : str_args[2]) {
            Color color = (ch >= 'A' && ch <= 'F') ? static_cast<Color>((ch - 'A') + 10) : static_cast<Color>(ch - '0');
            colormap.push_back(color);
        }
    }

    if(str_args[1] == "COLOR") {
        print_color_image_fast(console, input_image, colors, colormap);
    } else if(str_args[1] == "ASCOL") {
        std::string ascii_map = (argc == 4) ? str_args[3] : DEFAULT_ASCII;
        std::string ascii_output = ascii_image(input_image, colors, ascii_map);
        print_color_ascii_fast(console, ascii_output, colors, input_image, colormap);
    }

    delete[] colors;
    return 0;
}

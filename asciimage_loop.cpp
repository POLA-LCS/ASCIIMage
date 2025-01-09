#include <stdio.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <conio.h>
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

    Image(std::string path, int channels = 3) : path(path), channels(channels) {
        if (channels < 3 || channels > 4) {
            fprintf(stderr, "[!] Invalid number of channels for Image.\n");
            exit(1);
        }
    }

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

    void write(std::string rpath = nullptr, int rchannels = 0) const {
        if(rchannels < 3 || rchannels > 4) rchannels = channels;
        if(rpath.empty()) rpath = path;

        if(rchannels == 3) {
            stbi_write_jpg(rpath.c_str(), width, height, channels, data, 100);
        } else {
            stbi_write_png(rpath.c_str(), width, height, channels, data, 0);
        }
    }

    void get_color_array(RGBA*& color_array) const {
        if (color_array) delete[] color_array;
        color_array = new RGBA[image_size()];
        for (size_t b = 0, color = 0; b < size(); b += channels) {
            color_array[color++] = {data[b], data[b + 1], data[b + 2], (channels == 4) ? data[b + 3] : static_cast<byte>(255)};
        }
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

void print_color_image(Winsole& winsole, const Image& image, RGBA*& colors, const std::vector<Color>& colormap) {
    size_t map_size = colormap.size();
    for(size_t ci = 0, row = 1; ci < image.image_size(); ci++) {
        byte grey = (colors[ci].max_value() + colors[ci].min_value()) / 2;
        size_t index = static_cast<size_t>(map(grey, 0, 255, 0, map_size - 1));
        Color color = colormap[index];

        if(ci >= (row * image.width)) {
            putchar('\n');
            row++;
        }

        winsole.put(' ', {AUTO, color});
    }
}

void print_color_ascii(Winsole& winsole, const std::string& ascii_image, RGBA*& colors, const Image& image, const std::vector<Color>& colormap) {
    size_t map_size = colormap.size();
    size_t ascii_index = 0;

    for(size_t ci = 0; ci < image.image_size(); ci++) {
        if(ascii_image[ascii_index] == '\n') {
            putchar('\n');
            ascii_index += 2;
            continue;
        }

        byte grey = (colors[ci].max_value() + colors[ci].min_value()) / 2;
        size_t index = static_cast<size_t>(map(grey, 0, 255, 0, map_size - 1));
        Color color = colormap[index];

        winsole.put(ascii_image[ascii_index], {color, AUTO});
        ascii_index++;
    }
}

void fast_print(const Winsole& console, const std::string& buffer) {
    WriteConsole(console.get_handle(), buffer.c_str(), buffer.length(), nullptr, nullptr);
}

#define version_message "AsciiMage v1.0 (Dec 9 2024)\n\n"

void print_help() {
    printf(version_message);
    printf("[USAGE]\n");
    printf("    asciimage [--help]                Display this message.\n");
    printf("    asciimage <input> <mode> [map]    Prints an image in the selected mode.\n");
    printf("\n[MODES]\n");
    printf("    ASCII    Maps each pixel of the image with an ascii character  (BLINK FAST).\n");
    printf("    COLOR    Uses winsole to print the colored image               (SLOW).\n");
    printf("    ASCOL    Uses winsole to tint the ASCII result with a colormap (SLOW as COLOR).\n");
    printf("\n[MAPS]\n");
    printf("    A map is a string used by the program to generate a custom output.\n");
    printf("    If space character (32) is wanted to be in the ascii map double quotes are needed.\n");
    printf("    This behaviour helps the program to detect spaces easier.\n");
    printf("    ASCOL needs two maps (first is the ASCII map, second is the COLOR map).\n");
    printf("\ne.g:\n");
    printf("    asciimage image.jpg ASCII ( ._-oa3O@)\n");
    printf("    asciimage image.jpg COLOR 0193BF\n");
    printf("    asciimage image.jpg ASCOL ( ._-oa3O@) 0193BF\n");
    printf("\nNOTE:\n");
    printf("    The color indexes are based on your console palette.\n");
    printf("    ASCOL maps length can be different.\n");
}

#define DEFAULT_ASCII " ._-3#@"
#define DEFAULT_COLOR_MAP {BLACK, BLACK, BLACK, GREY, GREY, BLUE, LIGHT_BLUE, AQUA, LIGHT_AQUA, WHITE, WHITE, WHITE}

#define LOOP while(true) if(kbhit()) break

int main(int argc, char* argv[]) {
    argc -= 1;

    // NO ARGS WHERE PROVIDED
    if(!argc) {
        print_help();
        return 0;
    }

    // Stringify argv to compare easily
    std::string str_args[argc];
    for(size_t i = 0; i < argc; i++) {
        str_args[i] = argv[i + 1];

        // HELP
        if(str_args[i] == "-h" || str_args[i] == "--help") {
            print_help();
            LOOP;
            return 0;
        }
    }

    // BASIC CONSOLE SETUP
    Winsole console;
    if(!console.init()) {
        fprintf(stderr, "[!] Failed to init the console.\n");
        LOOP;
        return 1;
    }

    // console.set_raw_size({300, 9000});
    // if(console.update()) {
    //     fprintf(stderr, "[!] Failed to resize the console buffer.\n");
    //     return 1;
    // }

    std::string input_path = str_args[0];
    std::string ascii_map, color_map;

    Image input_image(input_path.c_str());
    if(!input_image.read()) {
        fprintf(stderr, "[!] Failed to read image.\n");
        LOOP;
        return 1;
    }

    RGBA* colors = nullptr;
    input_image.get_color_array(colors);
    if(!colors) {
        fprintf(stderr, "[!] Failed to get color array.\n");
        LOOP;
        return 1;
    }

    if(argc == 1) str_args[1] = "ASCII";

    if(str_args[1] == "ASCII") {
        ascii_map = (argc == 2) ? DEFAULT_ASCII : str_args[2];
        std::string ascii_output = ascii_image(input_image, colors, ascii_map);
        fast_print(console, ascii_output);
        LOOP;
        return 0;
    }
    
    if(str_args[1] == "COLOR" || str_args[1] == "ASCOL") {
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
            print_color_image(console, input_image, colors, colormap);
        } else if (str_args[1] == "ASCOL") {
            std::string ascii_map = (argc == 4) ? str_args[3] : DEFAULT_ASCII;
            std::string ascii_output = ascii_image(input_image, colors, ascii_map);
            print_color_ascii(console, ascii_output, colors, input_image, colormap);
        }

        LOOP;
        return 0;
    }

    delete[] colors;
    LOOP;
    return 0;
}

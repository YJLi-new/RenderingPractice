#ifndef RTW_STB_IMAGE_H
#define RTW_STB_IMAGE_H

#ifdef _MSC_VER
#pragma once
#endif

#ifdef RTW_STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif
#define STBI_FAILURE_USERMSG
#include "stb-master/stb_image.h"

#include <cstdlib>
#include <iostream>
#include <string>

class rtw_image {
public:
    rtw_image() {}
    explicit rtw_image(const char* image_filename) {
        auto filename = std::string(image_filename);

        #if defined(_MSC_VER)
            char* envval = nullptr;
        size_t envlen = 0;
        if (_dupenv_s(&envval, &envlen, "RTW_IMAGES") == 0 && envval) {
            std::string imagedir(envval);
            free(envval);
            if (load(imagedir + "/" + image_filename)) return;
        }

        #else
        auto imagedir = getenv("RTW_IMAGES");
        if (imagedir && load(std::string(imagedir) + "/" + image_filename)) return;
        #endif

        if (load(filename)) return;
        if (load("images/" + filename)) return;
        if (load("../images/" + filename)) return;
        if (load("../../images/" + filename)) return;
        if (load("../../../images/" + filename)) return;
        if (load("../../../../images/" + filename)) return;
        if (load("../../../../../images/" + filename)) return;
        if (load("../../../../../../images/" + filename)) return;
        std::cerr << "ERROR: Could not load image file '" << image_filename << "'.\n";
    }

    ~rtw_image() {
        delete[] bdata;
        bdata = nullptr;
        if (fdata) {
            stbi_image_free(fdata);
            fdata = nullptr;
        }
    }

    bool load(const std::string& filename) {
        int n = bytes_per_pixel;
        fdata = stbi_loadf(filename.c_str(), &image_width, &image_height, &n, bytes_per_pixel);
        if (!fdata) return false;
        bytes_per_scanline = image_width * bytes_per_pixel;
        convert_to_bytes();
        return true;
    }

    int width()  const { return (fdata == nullptr) ? 0 : image_width; }
    int height() const { return (fdata == nullptr) ? 0 : image_height; }

    const unsigned char* pixel_data(int x, int y) const {
        static unsigned char magenta[] = { 255, 0, 255 };
        if (!bdata) return magenta;
        x = clamp(x, 0, image_width);
        y = clamp(y, 0, image_height);
        return bdata + y * bytes_per_scanline + x * bytes_per_pixel;
    }

private:
    const int bytes_per_pixel = 3;
    float* fdata = nullptr;
    unsigned char* bdata = nullptr;
    int            image_width = 0;
    int            image_height = 0;
    int            bytes_per_scanline = 0;

    static int clamp(int x, int low, int high) {
        if (x < low) return low;
        if (x < high) return x;
        return high - 1;
    }

    static unsigned char float_to_byte(float v) {
        if (v <= 0.0) return 0;
        if (1.0 <= v) return 255;
        return static_cast<unsigned char>(256.0 * v);
    }

    void convert_to_bytes() {
        int total_bytes = image_width * image_height * bytes_per_pixel;
        bdata = new unsigned char[total_bytes];
        auto* bptr = bdata;
        auto* fptr = fdata;
        for (int i = 0; i < total_bytes; ++i, ++fptr, ++bptr) *bptr = float_to_byte(*fptr);
    }
};

#ifdef _MSC_VER
#endif
#endif // RTW_STB_IMAGE_H

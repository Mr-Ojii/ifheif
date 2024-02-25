#ifndef _HEIF_SPI_HPP_
#define _HEIF_SPI_HPP_
#include <cstdint>
#include <windows.h>

static const char* plugin_info[] = {
    "00IN",
    "HEIF AVIF Reader by Mr-Ojii",
    "*.heif;*.heic;*.avif",
    "HEIF File(*.heif,*.heic);AVIF File(*.avif)"
};

typedef struct {
    long left, top;
    long width;
    long height;
    WORD x_density;
    WORD y_density;
    short colorDepth;
    HLOCAL hInfo;
} PictureInfo;

typedef struct {
    uint8_t b, g, r, a;
} Pixel_BGRA;

typedef struct {
    uint8_t r, g, b, a;
} Pixel_RGBA;


// https://nokiatech.github.io/heif/technical.html
// https://aomediacodec.github.io/av1-avif/
static const char* support_brand[] = {
    // "mif1",
    // "mis1",
    "heic",
    "heix",
    "hevc",
    "hevx",
    "avif",
    "avis",
};

int load_heif(void* buf, long len, PictureInfo* info, HLOCAL* data, BOOL decode_image);

#endif

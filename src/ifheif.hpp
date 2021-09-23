#ifndef _HEIF_SPI_HPP_
#define _HEIF_SPI_HPP_
#include <windows.h>

static const char* plugin_info[] = {
	"00IN",
	"HEIF AVIF Reader by Mr-Ojii",
    "*.heif;*.avif",
	"HEIF File(*.heif);AVIF File(*.avif)"
};

struct PictureInfo {
    long left, top;
    long width;
    long height;
    WORD x_density;
    WORD y_density;
    short colorDepth;
    HLOCAL hInfo;
};

int load_heif(void* buf, long len, PictureInfo* info, HLOCAL* data, BOOL decode_image);

#endif

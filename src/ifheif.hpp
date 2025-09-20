#ifndef _HEIF_SPI_HPP_
#define _HEIF_SPI_HPP_
#include <cstdint>
#include <windows.h>

// https://www2f.biglobe.ne.jp/~kana/spi_api/spi_getplugininfo.html
// 規格の2n+2, 2n+3について、AviUtlの拡張編集に読み込ませるために複数ファイル形式を1つにまとめた書き方をする。
// Susie-Pluginの2n+2に対しては問題が発生しない実装にしたため、規格に準拠しているホストであれば、正常に読み込みが可能であると思われる。
// https://scrapbox.io/Mr-Ojii/AviUtl%E3%81%AE%E6%8B%A1%E5%BC%B5%E7%B7%A8%E9%9B%86%E3%81%ABSusie%E3%83%97%E3%83%A9%E3%82%B0%E3%82%A4%E3%83%B3%E3%82%92%E5%B0%8E%E5%85%A5%E3%81%97%E3%81%9F%E3%81%8C%E3%80%81%E7%94%BB%E5%83%8F%E3%83%95%E3%82%A1%E3%82%A4%E3%83%AB%E3%82%92%E8%AA%AD%E3%81%BF%E8%BE%BC%E3%81%BE%E3%81%AA%E3%81%84
static const char* plugin_info[] = {
    "00IN",
    "HEIF AVIF Reader by Mr-Ojii",
    "*.heif;*.heic;*.avif",
    "HEIF File(*.heif,*.heic);AVIF File(*.avif)"
};

#pragma pack(push, 1)
typedef struct {
    long left, top;
    long width;
    long height;
    WORD x_density;
    WORD y_density;
    short colorDepth;
#ifdef _WIN64
    char dummy[2]; // アラインメント合わせ用(64bit版)
#endif
    HLOCAL hInfo;
} PictureInfo;
#pragma pack(pop)

enum SUSIE_ERROR {
    SUSIE_SUCCESS = 0,
    SUSIE_NOT_IMPLEMENTED = -1,
    SUSIE_ABORT = 1,
    SUSIE_UNKNOWN_FORMAT = 2,
    SUSIE_BROKEN_DATA = 3,
    SUSIE_FAILED_ALLOC_MEMORY = 4,
    SUSIE_GENERIC_MEMORY_ERROR = 5,
    SUSIE_FILE_READ_ERROR = 6,
    SUSIE_RESERVED = 7,
    SUSIE_INTERNAL_ERROR = 8,
    SUSIE_FILE_WRITE_ERROR = 9,
};

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

void copy_pinfo_to_bminfo(const PictureInfo* pinfo, BITMAPINFO* bminfo);
int read_file(HANDLE handle, void** buf, long* len);
int read_file_a(LPCSTR filename, void** buf, long* len);
int read_file_w(LPCWSTR filename, void** buf, long* len);
int load_heif(const void* buf, long len, PictureInfo* info, HLOCAL* data);

#endif

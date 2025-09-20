#include <cstdio>
#include <algorithm>
#include <libheif/heif.h>
#include <windows.h>
#include "ifheif.hpp"

EXTERN_C int __declspec(dllexport) __stdcall GetPluginInfo(int infono, LPSTR buf, int buflen) {
    if (infono < 0 || infono >= sizeof(plugin_info) / sizeof(plugin_info[0]))
        return 0;

    const char* info = plugin_info[infono];
    memset(buf, 0, buflen);
    int cpbytes = std::min((int)strlen(info), buflen);
    memcpy(buf, info, cpbytes);

    return cpbytes;
}

EXTERN_C int __declspec(dllexport) __stdcall IsSupported(LPSTR filename, const void* dw) {
    const BYTE* data;

    // ファイルハンドル指定は使用禁止であるため削除
    data = reinterpret_cast<const BYTE*>(dw);

    uint32_t byte_pos = 0;
    for(uint32_t byte_pos = 0; byte_pos < 2048;) {
        uint32_t size = 0;
        for(int i = 0; i < 4; i++) {
            size = size << 8;
            size += data[byte_pos];
            byte_pos++;
        }
        if(size <= 8) {
            break;
        }

        char box_type[5] = { '\0' };
        for(int i = 0; i < 4; i++) {
            box_type[i] = data[byte_pos];
            byte_pos++;
        }
        if(strcmp("ftyp", box_type) == 0) {
            char major_brand[5] = { '\0' };
            for(int i = 0; i < 4; i++) {
                major_brand[i] = data[byte_pos];
                byte_pos++;
            }

            int support_brand_length = sizeof(support_brand) / sizeof(support_brand[0]);
            int ret = 0;
            for(int i = 0; i < support_brand_length; i++)
            {
                if(strcmp(support_brand[i], major_brand) == 0)
                    ret = 1;
            }
            return ret;
        }

        byte_pos += (size - 8);
    }

    return 0;
}

EXTERN_C int __declspec(dllexport) __stdcall GetPictureInfo(LPCSTR buf, LONG_PTR len, unsigned int flag, PictureInfo *lpInfo) {
    const void* data;
    long length;
    void* bufpoint = NULL;
    int ret = SUSIE_INTERNAL_ERROR;

    if((flag & 0b111) == 0) {
        long readsize;
        ret = read_file_a(buf, &bufpoint, &readsize);
        if (ret != SUSIE_SUCCESS)
            return ret;
        data = bufpoint;
        length = readsize;
    } else {
        data = buf;
        length = len;
    }

    if(load_heif(data, length, lpInfo, NULL)) {
        ret = SUSIE_SUCCESS;
    } else {
        ret = SUSIE_UNKNOWN_FORMAT;
    }

    free(bufpoint);

    return ret;
}

EXTERN_C int __declspec(dllexport) __stdcall GetPicture(LPCSTR buf, LONG_PTR len, unsigned int flag, HLOCAL *pHBInfo, HLOCAL *pHBm, FARPROC lpPrgressCallback, LONG_PTR lData) {
    lpPrgressCallback = NULL;
    const void* data;
    long length;
    void* bufpoint = NULL;
    int ret = SUSIE_INTERNAL_ERROR;

    if((flag & 0b111) == 0) {
        long readsize;
        ret = read_file_a(buf, &bufpoint, &readsize);
        if (ret != SUSIE_SUCCESS)
            return ret;
        data = bufpoint;
        length = readsize;
    } else {
        data = buf;
        length = len;
    }

    PictureInfo info;
    HLOCAL pic_data;
    if(load_heif(data, length, &info, &pic_data)) {
        *pHBInfo = LocalAlloc(LMEM_MOVEABLE, sizeof(BITMAPINFO));
        BITMAPINFO* pinfo = reinterpret_cast<BITMAPINFO*>(LocalLock(*pHBInfo));

        pinfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        pinfo->bmiHeader.biWidth = info.width;
        pinfo->bmiHeader.biHeight = info.height;
        pinfo->bmiHeader.biPlanes = 1;
        pinfo->bmiHeader.biBitCount = info.colorDepth;
        pinfo->bmiHeader.biCompression = BI_RGB;
        pinfo->bmiHeader.biSizeImage = 0;
        pinfo->bmiHeader.biXPelsPerMeter = 0;
        pinfo->bmiHeader.biYPelsPerMeter = 0;
        pinfo->bmiHeader.biClrUsed = 0;
        pinfo->bmiHeader.biClrImportant = 0;
        
        LocalUnlock(*pHBInfo);

        *pHBm = pic_data;

        ret = SUSIE_SUCCESS;
    } else {
        ret = SUSIE_UNKNOWN_FORMAT;
    }
    free(bufpoint);
    return ret;
}

EXTERN_C int __declspec(dllexport) __stdcall GetPreview(LPSTR buf, long len, unsigned int flag, HANDLE *pHBInfo, HANDLE *pHBm, FARPROC loPrgressCallback, long lData) {
    return -1;
}

int read_file(HANDLE handle, void** buf, long* len) {
    if (handle == INVALID_HANDLE_VALUE)
        return SUSIE_FILE_READ_ERROR;

    DWORD filesize = GetFileSize(handle, NULL);
    if (filesize == 0)
        return SUSIE_FILE_READ_ERROR;

    void* buf_ptr = malloc(filesize);
    if (!buf_ptr)
        return SUSIE_FAILED_ALLOC_MEMORY;

    DWORD readsize;
    if (!ReadFile(handle, buf_ptr, filesize, &readsize, NULL)) {
        free(buf_ptr);
        return SUSIE_FILE_READ_ERROR;
    }

    *buf = buf_ptr;
    *len = readsize;
    return SUSIE_SUCCESS;
}

int read_file_a(LPCSTR filename, void** buf, long* len) {
    HANDLE handle = CreateFileA(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    int ret = read_file(handle, buf, len);
    CloseHandle(handle);
    return ret;
}

int read_file_w(LPCWSTR filename, void **buf, long *len) {
    HANDLE handle = CreateFileW(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    int ret = read_file(handle, buf, len);
    CloseHandle(handle);
    return ret;
}

int load_heif(const void* buf, long len, PictureInfo* info, HLOCAL* data) {
    int ret = 0;
    if (buf != NULL && len != 0) {
        struct heif_context* ctx = heif_context_alloc();
        if(!heif_context_read_from_memory(ctx, buf, len, NULL).code) {
            struct heif_image_handle* handle;
            if(!heif_context_get_primary_image_handle(ctx, &handle).code){
                //pic_info
                info->left       = 0;
                info->top        = 0;
                info->width      = heif_image_handle_get_width(handle);
                info->height     = heif_image_handle_get_height(handle);
                info->x_density  = 0;
                info->y_density  = 0;
                info->colorDepth = 32;
                info->hInfo      = NULL;

                if(data) {
                    struct heif_image* img;
                    if(!heif_decode_image(handle, &img, heif_colorspace_RGB, heif_chroma_interleaved_RGBA, NULL).code) {
                        int stride;
                        const uint8_t* b_dat = reinterpret_cast<const uint8_t*>(heif_image_get_plane_readonly(img, heif_channel_interleaved, &stride));
                        *data = LocalAlloc(LMEM_MOVEABLE, info->width * info->height * (info->colorDepth / 8));
                        if(*data) {
                            Pixel_BGRA* dat = reinterpret_cast<Pixel_BGRA*>(LocalLock(*data));
                            for(int i = 0; i < info->height; i++) {
                                const Pixel_RGBA* img_dat = reinterpret_cast<const Pixel_RGBA*>(&b_dat[(stride * (info->height - i - 1))]);
                                for(int j = 0; j < info->width; j++) {
                                    dat[info->width * i + j].b = img_dat[j].b;
                                    dat[info->width * i + j].g = img_dat[j].g;
                                    dat[info->width * i + j].r = img_dat[j].r;
                                    dat[info->width * i + j].a = img_dat[j].a;
                                }
                            }
                            LocalUnlock(*data);
                            ret = 1;
                        }
                        heif_image_release(img);
                    }
                }
                heif_image_handle_release(handle);
            }
        }
        heif_context_free(ctx);
    }
    return ret;
}

#include <stdio.h>
#include <libheif/heif.h>
#include <windows.h>
#include "ifheif.hpp"

extern "C" int __declspec(dllexport) __stdcall GetPluginInfo(int infono, LPSTR buf, int buflen) {
    if (infono < 0 || infono >= sizeof(plugin_info) / sizeof(plugin_info[0]))
        return 0;

    strcpy(buf, plugin_info[infono]);

    return strlen(buf); 
}

extern "C" int __declspec(dllexport) __stdcall IsSupported(LPSTR filename, DWORD dw) {
    BYTE buf[2048] = {0}, *data;
    
    if( dw & ~0xffff ) {
        data = (BYTE *)dw;
    } else {
        DWORD bytes;
    	if(!ReadFile((HANDLE)dw, buf, sizeof(buf), &bytes, nullptr)) {
            return 0;
	    }
        SetFilePointer((HANDLE)dw, 0, NULL, FILE_BEGIN);
        data = buf;
    }

    BOOL ret = FALSE;
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
            if(strcmp("heic", major_brand) == 0 || strcmp("avif", major_brand) == 0) {
                return 1;
            } else {
                return 0;
            }
        }

        byte_pos += (size - 8);
    }

    return 0;
}

extern "C" int __declspec(dllexport) __stdcall GetPictureInfo(LPSTR buf, long len, unsigned int flag, PictureInfo *lpInfo) {
    void* data;
    long length;
    void* bufpoint = nullptr;
    int buf_success = FALSE;
    int ret = 1;

    if((flag & 0b111) == 0) {
        HANDLE handle = CreateFile(buf, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (handle != INVALID_HANDLE_VALUE) {
            DWORD filesize = GetFileSize(handle, NULL);
            DWORD readsize;
            if (filesize != 0) {
                bufpoint = malloc(filesize);
                if(bufpoint){
                    if (ReadFile(handle, bufpoint, filesize, &readsize, NULL)) {
                        data = bufpoint;
                        length = readsize;
                        buf_success = true;
                    } else {
                        ret = 1;
                    }
                } else {
                    ret = 4;
                }
            } else {
                ret = 6;
            }

            CloseHandle(handle);
        } else {
            ret = 6;
        }
    } else {
        data = buf;
        length = len;
        buf_success = TRUE;
    }

    if(buf_success) {
        HLOCAL hl;
        if(load_heif(data, length, lpInfo, &hl, FALSE)) {
            ret = 0;
        } else {
            ret = 2;
        }
    } else {
        ret = 4;
    }

    free(bufpoint);

    return ret;
}

extern "C" int __declspec(dllexport) __stdcall GetPicture(LPSTR buf, long len, unsigned int flag, HANDLE *pHBInfo, HANDLE *pHBm, FARPROC lpPrgressCallback, long lData) {
    lpPrgressCallback = NULL;
    void* data;
    long length;
    void* bufpoint = nullptr;
    int buf_success = FALSE;
    int ret = 1;

    if((flag & 0b111) == 0) {
        HANDLE handle = CreateFile(buf, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (handle != INVALID_HANDLE_VALUE) {
            DWORD filesize = GetFileSize(handle, NULL);
            DWORD readsize;
            if (filesize != 0) {
                bufpoint = malloc(filesize);
                if(bufpoint){
                    if (ReadFile(handle, bufpoint, filesize, &readsize, NULL)) {
                        data = bufpoint;
                        length = readsize;
                        buf_success = true;
                    } else {
                        ret = 1;
                    }
                } else {
                    ret = 4;
                }
            } else {
                ret = 6;
            }

            CloseHandle(handle);
        } else {
            ret = 6;
        }
    } else {
        data = buf;
        length = len;
        buf_success = TRUE;
    }

    if(buf_success) {
        PictureInfo info;
        HLOCAL pic_data;
        if(load_heif(data, length, &info, &pic_data, TRUE)) {
            *pHBInfo = LocalAlloc(LMEM_MOVEABLE, sizeof(BITMAPINFO));
            BITMAPINFO* pinfo = (BITMAPINFO*)LocalLock(*pHBInfo);

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

            ret = 0;
        }
    }
    free(bufpoint);
    return ret;
}

extern "C" int __declspec(dllexport) __stdcall GetPreview(LPSTR buf, long len, unsigned int flag, HANDLE *pHBInfo, HANDLE *pHBm, FARPROC loPrgressCallback, long lData) {
    return -1;
}

int load_heif(void* buf, long len, PictureInfo* info, HLOCAL* data, BOOL decode_image) {
    int ret = 0;
    const uint8_t* img_dat = nullptr;
    if (buf != nullptr && len != 0) {
        heif_context* ctx = heif_context_alloc();
        if(!heif_context_read_from_memory(ctx, buf, len, nullptr).code) {
            heif_image_handle* handle;
            if(!heif_context_get_primary_image_handle(ctx, &handle).code){
                //pic_info
                info->left       = 0;
                info->top        = 0;
                info->width      = heif_image_handle_get_width(handle);
                info->height     = heif_image_handle_get_height(handle);
                info->x_density  = 0;
                info->y_density  = 0;
                info->colorDepth = 32;
                info->hInfo      = nullptr;

                if(decode_image) {
                    heif_image* img;
                    if(!heif_decode_image(handle, &img, heif_colorspace_RGB, heif_chroma_interleaved_RGBA, nullptr).code) {
                        int stride;
                        img_dat = heif_image_get_plane_readonly(img, heif_channel_interleaved, &stride);
                        *data = LocalAlloc(LMEM_MOVEABLE, info->width * info->height * (info->colorDepth / 8));
                        if(*data) {
                            uint8_t* dat = (uint8_t*)LocalLock(*data);
                            for(int i = 0; i < info->height; i++) {
                                for(int j = 0; j < info->width; j++) {
                                    dat[(info->width * i) * 4 + j * 4 + 0] = img_dat[(info->width * (info->height - i - 1)) * 4 + j * 4 + 2];
                                    dat[(info->width * i) * 4 + j * 4 + 1] = img_dat[(info->width * (info->height - i - 1)) * 4 + j * 4 + 1];
                                    dat[(info->width * i) * 4 + j * 4 + 2] = img_dat[(info->width * (info->height - i - 1)) * 4 + j * 4 + 0];
                                    dat[(info->width * i) * 4 + j * 4 + 3] = img_dat[(info->width * (info->height - i - 1)) * 4 + j * 4 + 3];
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

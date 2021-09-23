# ifheif
libheifを使用し、HEIF/AVIFを読み込むためのSusie Plug-inです。(AviUtlの拡張編集でも使えます)  
プルリク大歓迎です！

## ビルド方法
+ [libde265](https://github.com/strukturag/libde265)
+ [dav1d](https://code.videolan.org/videolan/dav1d)
+ [libheif](https://github.com/strukturag/libheif)

のインストールを先にして、

```sh
i686-w64-mingw32-g++ ifheif.cpp ifheif.def "-Wl,--dll,--enable-stdcall-fixup" -o ifheif.spi `PKG_CONFIG_PATH=任意のフォルダ i686-w64-mingw32-pkg-config --libs --cflags libheif libde265 dav1d` -static -shared
```

などでビルドしてください。

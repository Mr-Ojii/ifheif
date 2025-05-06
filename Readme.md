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

MSYS2を用いてのコンパイル手順は[build.yml](https://github.com/Mr-Ojii/ifheif/blob/master/.github/workflows/build.yml)をご覧ください。

## 注意事項
+ もともとの画像が何bit depthであろうが、8bit depthに変換します。(多分)
+ 複数画像が入っていたとしても、単一画像として扱います(多分)
+ デコードに多少時間がかかります
+ 必要になったので、突貫で書いたものです。バグが含まれている可能性があります。発見した場合、IssueやPull requestをお願いします

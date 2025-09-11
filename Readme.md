# ifheif
libheifを使用し、HEIF/AVIFを読み込むためのSusie Plug-inです。(AviUtlの拡張編集でも使えます)  
プルリク大歓迎です！

## 対応プラグイン形式
以下のいずれも[TORO氏の Susie 32bit / 64bit Plug-in の仕様(2025-8-10版)](http://toro.d.dooo.jp/dlsphapi.html)に準拠するよう、作成しています。
- .spi   Susie 32bit Plug-in
- .sph   Susie 64bit(x64) Plug-in

## ビルド方法
MSYS2を用いてのビルド手順は[build.yml](https://github.com/Mr-Ojii/ifheif/blob/master/.github/workflows/build.yml)をご覧ください。

## 注意事項
+ もともとの画像が何bit depthであろうが、8bit depthに変換します。(多分)
+ 複数画像が入っていたとしても、単一画像として扱います(多分)
+ デコードに多少時間がかかります
+ 必要になったので、突貫で書いたものです。バグが含まれている可能性があります。発見した場合、IssueやPull requestをお願いします

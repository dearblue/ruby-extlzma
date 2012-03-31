# encoding:utf-8

= liblzma for Ruby

- AUTHOR : dearblue
- MAIL : dearblue@users.sourceforge.jp
- LICENSE : 2-clause BSD License (二条項BSDライセンス)
- PROJECTPAGE : http://sourceforge.jp/projects/rutsubo/

This document is written in Japanese.


== はじめに

xzユーティリティに含まれるliblzmaのRubyバインディングです。

gemパッケージになっており、『gem install liblzma-0.2.gem』でビルド、インストールが完了します。

MinGW32およびFreeBSD 9.0R i386で動作確認を行っています。

MinGW32向けにはバイナリパッケージも用意してあります (もしかしたらruby-mswin32でも動作するかもしれません)。

FreeBSD 9.0R i386 上では標準コンパイラであるGCCのほかにllvm clangでもビルド可能なことを確認してあります。


== 利用できる主な機能

※カッコ内は、本来のliblzmaが提供する関数を示す

* LZMA::Stream::Encoder/Decoder (lzma_stream_encoder / lzma_stream_decoder)
* LZMA::Stream::RawEncoder/RawDecoder (lzma_raw_encoder / lzma_raw_decoder)
* LZMA::Utils.crc32/crc64 (lzma_crc32 / lzma_crc64)
* LZMA::Filter::LZMA1/LZMA2/Delta


== 実際に利用するには

まともな文書化が出来ていないため、gemパッケージ内の『samples』ディレクトリに含まれる各サンプルを頼りにして下さい。


== ライセンスについて

liblzma for Ruby は、二条項BSDライセンスの下で利用できます。


# extlzma - lzma for Ruby

  * **package name**: extlzma
  * **author**: dearblue (mailto:dearblue@users.osdn.me)
  * **report issue to**: <https://osdn.jp/projects/rutsubo/ticket/>
  * **how to install**: ``gem install extlzma``
  * **version**: 0.4
  * **release quality**: thechnical preview
  * **license**: BSD-2-Clause (二条項BSDライセンス)
  * **dependency gems**: none
  * **dependency external c libraries**: liblzma <http://tukaani.org/xz/>
  * **bundled external c libraries**: none

This document is written in Japanese.


## はじめに

xz ユーティリティに含まれる liblzma の ruby バインディングです。

MinGW32 および FreeBSD 10.3R amd64 で動作確認を行っています。

MinGW32 向けにはバイナリパッケージも用意してあります (もしかしたら ruby-mswin32 でも動作するかもしれません)。


## 利用できる主な機能

※カッコ内は、本来の liblzma が提供する関数を示す

  * LZMA::Stream::Encoder / LZMA::Stream::Decoder (lzma\_stream\_encoder / lzma\_stream\_decoder)
  * LZMA::Stream::RawEncoder / LZMA::Stream::RawDecoder (lzma\_raw\_encoder / lzma\_raw\_decoder)
  * LZMA::Filter::LZMA1 / LZMA::Filter::LZMA2 / LZMA::Filter::Delta
  * LZMA.crc32 / LZMA.crc64 (lzma\_crc32 / lzma\_crc64)


## 実際に利用するには

まともな文書化が出来ていないため、gem パッケージ内の『examples』ディレクトリに含まれる各サンプルを頼りにして下さい。


## ライセンスについて

extlzma は、二条項 BSD ライセンスの下で利用できます。

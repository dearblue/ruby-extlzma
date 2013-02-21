
= liblzma for Ruby

- AUTHOR : dearblue
- MAIL : dearblue@users.sourceforge.jp
- LICENSE : 2-clause BSD License (二条項BSDライセンス)
- PROJECT PAGE : http://sourceforge.jp/projects/rutsubo/

This document is written in Japanese.


== はじめに

xz ユーティリティに含まれる liblzma の Ruby バインディングです。

gemパッケージになっており、『gem install liblzma-0.3.gem』でビルド、インストールが完了します。

MinGW32 および FreeBSD 9.0R i386 で動作確認を行っています。

MinGW32 向けにはバイナリパッケージも用意してあります (もしかしたら ruby-mswin32 でも動作するかもしれません)。

FreeBSD 9.0R i386 上では標準コンパイラである GCC のほかに llvm clang でもビルド可能なことを確認してあります。


== 利用できる主な機能

※カッコ内は、本来の liblzma が提供する関数を示す

- LZMA::Stream::Encoder / Decoder (lzma_stream_encoder / lzma_stream_decoder)
- LZMA::Stream::RawEncoder / RawDecoder (lzma_raw_encoder / lzma_raw_decoder)
- LZMA::Filter::LZMA1 / LZMA2 / Delta
- LZMA::Utils.crc32 / crc64 (lzma_crc32 / lzma_crc64)


== 実際に利用するには

まともな文書化が出来ていないため、gem パッケージ内の『samples』ディレクトリに含まれる各サンプルを頼りにして下さい。


== ライセンスについて

liblzma for Ruby は、二条項 BSD ライセンスの下で利用できます。

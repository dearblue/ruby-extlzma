#!/usr/bin/env ruby
#vim: set fileencoding:utf-8

# このrubyスクリプトはruby-lzmaの利用手引きを兼ねたサンプルスクリプトです。


# 01-* ではデータの全てを用意してから処理を行う方法を提示しますた。
# このサンプルではオンメモリでは難しい大きなデータを処理する方法を提示します。


# 始めはお決まりで
require "liblzma"

# 今回のサンプルでファイルI/Oを再現するためにStringIOを利用します
require "stringio"

# 圧縮元となるデータを用意します
original_data = "abcdefghijklmnopqrstuvwxyz" * 50


# まずは圧縮してみましょう
source = StringIO.new(original_data)
dest = ""
encoder = LZMA::Stream.encoder(LZMA::Filter.lzma2)  # (1)
while fragment = source.read(16)                    # (2)
    dest << encoder.update(fragment)                #  -
end                                                 #  -
dest << encoder.finish                              # (3)

# (1) はじめに圧縮器を構築します
#     LZMA::Stream.encoderの引数は、1つ以上をとることが出来ます (上限はliblzma由来となる4つです)
#     LZMA::Filter.lzma2の引数は、LZMA.encodeの二つ目以降の引数と同じです
#     (LZMA::Filter.lzma2(1) とか LZMA::Filter.lzma2(mode: LZMA::MODE_FAST) とか)
# (2) 取り込んだ部分データを処理します
# (3) データの終わりを通知し、圧縮器内に残ったデータを取り出します
#     これを忘れるとxzデータストリームとして間違ったものが生成されます



encoded_data = dest


# 伸張も圧縮とほとんど同じです
source = StringIO.new(encoded_data)
dest = ""
decoder = LZMA::Stream.decoder                      # (1)
while fragment = source.read(16)                    # (2)
    dest << decoder.update(fragment)                #  -
end                                                 #  -
dest << decoder.finish                              # (3)

puts(dest == original_data ? "data ok" : "bad data")

# 圧縮時と伸張時の違いがわかりますか?
# はじめ(1)の伸張器の構築部分 (encoderかdecoderか) を変更するだけで、後は同じような文で処理を行えます


# 上の例ではxzデータストリームのみの対応です
# lzmaデータストリームは対象となりません
# xzもlzmaも区別なく伸張したい場合は、decoder ではなく auto_decoder を使います
source = StringIO.new(encoded_data)
dest = ""
decoder = LZMA::Stream.auto_decoder                 # (1)
while fragment = source.read(16)                    # (2)
    dest << decoder.update(fragment)                #  -
end                                                 #  -
dest << decoder.finish                              # (3)

puts(dest == original_data ? "data ok" : "bad data")



# 今回利用したクラス/モジュール/メソッド
#   LZMA::Stream.encoder        LZMA::Stream::Encoder.new と等価
#   LZMA::Stream.decoder        LZMA::Stream::Decoder.new と等価
#   LZMA::Stream.auto_decoder   LZMA::Stream::AutoDecoder.new と等価
#   LZMA::Filter.lzma2          LZMA::Filter::LZMA2.new と等価
#   LZMA::Stream#update
#   LZMA::Stream#finish


# [EOF]

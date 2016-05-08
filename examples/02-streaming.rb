#!/usr/bin/env ruby
#vim: set fileencoding:utf-8

# この ruby スクリプトは extlzma の利用手引きを兼ねたサンプルスクリプトです。


# このファイルではオンメモリでは難しい大きなデータを処理する方法を提示します。
#
# 今回利用するクラス/モジュール/メソッド
# * LZMA.encode
# * LZMA.decode
# * LZMA::Encoder#<< (LZMA::Encoder#write)
# * LZMA::Encoder#finish
# * LZMA::Decoder#read


# 始めはお決まりで
require "extlzma"

# 今回のサンプルでファイルI/Oを再現するためにStringIOを利用します
require "stringio"

# 圧縮元となるデータを用意します
original_data = "abcdefghijklmnopqrstuvwxyz" * 50


# まずは圧縮してみましょう
src = StringIO.new(original_data)
dest = StringIO.new("")
encoder = LZMA.encode(dest)   ## 圧縮器の生成
while buf = src.read(50)
  encoder << buf              ## データの圧縮
end
encoder.finish                ## 最終処理を忘れずに実行して下さい
p dest.string # => 圧縮された xz データ


# 上の処理はブロックを用いて次のように置き換えることが出来ます
src = StringIO.new(original_data)
dest = StringIO.new("")
LZMA.encode(dest) do |encoder|  ## 圧縮器の生成
  while buf = src.read(50)
    encoder << buf              ## データの圧縮
  end
end                             ## finish は不要
p dest.string # => 圧縮された xz データ


# 次は伸長処理です
encoded_data = dest.string

src = StringIO.new(encoded_data)
dest = StringIO.new("")
decoder = LZMA.decode(src)
while buf = dest.read(50)
  dest << buf
end
puts(dest.string == original_data ? "data ok" : "bad data")

# これもブロックを用いて置き換えることが出来ます
src = StringIO.new(encoded_data)
dest = StringIO.new("")
LZMA.decode(src) do |decoder|
  while buf = dest.read(50)
    dest << buf
  end
end
puts(dest.string == original_data ? "data ok" : "bad data")


# [EOF]

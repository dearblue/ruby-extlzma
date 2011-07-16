#!/usr/local/bin/ruby
#vim: set fileencoding:utf-8

# このrubyスクリプトはruby-lzmaの利用手引きを兼ねたサンプルスクリプトです。


# rubyでliblzmaの機能を利用するのに必要なのは、次の一文のみです
require "lzma"


# 圧縮するデータを用意します
original_data = "0123456789abcdefghijklmnopqrstuvwxyz\n" * 10


# liblzmaを使って、データを圧縮します (xzデータストリームそのものが出力されます)
encoded_data = LZMA.encode(original_data)


# 圧縮レベルを変える場合は、第二引数に0から9の整数値を与えます
# この例は xz -1 と等価です
# (『9』を与える場合、作業メモリ空間が700MBほど必要になります)
encoded_data = LZMA.encode(original_data, 1)


# さらに細かい調整は、圧縮レベル(プリセット値)の後にハッシュ値を指定します
# この例ではプリセット値を省略しています
# (プリセット値の規定値はliblzma由来であり、『6』です)
encoded_data = LZMA.encode(original_data, mode: LZMA::MODE_FAST)

# この他にどんな値が与えられるかは、LZMA::Filter::LZMA2.newを参考にしてください


# 必要であればファイルに書き出しましょう
# xzデータストリームそのものです
File.open("sample.txt.xz", "wb") { |f| f << encoded_data }

# 出力されたファイルがxzユーティリティ(もしくは7-Zip)で開けることを確認してください



# 圧縮されたデータを伸張してみましょう
# たったこれだけです
puts LZMA.decode(encoded_data)



# 今回は処理したいデータをあらかじめ用意してから、一気に処理を行う方法を提示しました
# 現実には少しずつデータを処理していく必要がある事例も少なくありません
# (巨大な――数ギガバイトにも及ぶ――ファイルなどはオンメモリでは現実的に処理できませんね)
# データを少しずつ圧縮/伸張していく、ストリーム志向での処理は 02-* にて提示します


# 今回利用したクラス/モジュール/メソッド
#   LZMA.encode
#   LZMA.decode


# [EOF]

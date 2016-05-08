#!/usr/bin/env ruby
#vim: set fileencoding:utf-8

# この ruby スクリプトは extlzma の利用手引きを兼ねたサンプルスクリプトです。


# このファイルにはメソッドを 1 回呼ぶだけで、圧縮・伸長を行う方法を記述しています。
#
# 今回利用するクラス/モジュール/メソッド
# * LZMA.encode
# * LZMA.decode


# ruby で extlzma の機能を利用するのに必要なのは、次の一文のみです
require "extlzma"


# 圧縮するデータを用意します
original_data = "0123456789abcdefghijklmnopqrstuvwxyz\n" * 10


# extlzma を使って、データを圧縮します (xz データストリームそのものが出力されます)
encoded_data = LZMA.encode(original_data)


# 圧縮レベルを変える場合は、第二引数に 0 から 9 の整数値を与えます
# この例は xz -1 と等価です
# (『9』を与える場合、作業メモリ空間が 700 MB ほど必要になります)
encoded_data = LZMA.encode(original_data, 1)


# さらに細かい調整は、圧縮レベル (プリセット値) の後にハッシュ値を指定します
# この例ではプリセット値を省略しています
# (プリセット値の規定値は liblzma 由来であり、『6』です)
encoded_data = LZMA.encode(original_data, mode: LZMA::MODE_FAST)

# この他にどんな値が与えられるかは、LZMA::Filter::LZMA2.new を参考にしてください


# 必要であればファイルに書き出しましょう
# xz データストリームそのものです
File.write("sample.txt.xz", encoded_data, mode: "wb")

# 出力されたファイルが xz ユーティリティ (もしくは 7-Zip) で開けることを確認してください



# 圧縮されたデータを伸張してみましょう
# たったこれだけです
puts LZMA.decode(encoded_data)


# 今回は処理したいデータをあらかじめ用意してから、一気に処理を行う方法を提示しました。
# しかしながら現実には、少しずつデータを処理していく必要のある事例も少なくありません。
# データを少しずつ圧縮・伸張していく、ストリーム志向での処理は 02-* にて提示します。


# [EOF]

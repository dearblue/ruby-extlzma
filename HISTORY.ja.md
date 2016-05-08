# extlzma 更新履歴

## extlzma-0.4 (2016-5-8)

  * 名称を liblzma から extlzma に変更
  * ext/liblzma.c を機能ごとに分割
  * LZMA::Stream が提供する機能を最低限のみとするように変更
      * ``liblzma:lzma_code()`` を実行するのに最低限必要となる処理・機能のみの提供となるように変更しました。
      * バッファリングなどの高級機能については、新設された LZMA::Encoder / LZMA::Decoder が提供します。
      * LZMA::Stream は互換性を失っています。以前の stream.update、
        stream.flush、stream.finish メソッドは元々 ``liblzma:lzma_code()`` を用いていたため、
        stream.code に統合された形となっています。
  * LZMA::Utils.crc32 および LZMA::Utils.crc64 で与える文字列オブジェクトのロックを排除
      * これまで ``ruby:rb_str_locktmp()`` と ``ruby:rb_str_unlocktmp()``
        を用いて文字列オブジェクトにロックをかけてきましたが、これを廃止しました。
      * LZMA::Utils.crc32 / LZMA::Utils.crc64 を実行中に変更する可能性がある場合は、
        利用者側で排他的ロックを行うか、str.dup によってオブジェクトを複製する必要があります。
  * LZMA::Utils.crcXX に関する追加機能
      * LZMA::Utils.crc32\_digest / LZMA::Utils.crc32\_hexdigest
      * LZMA::Utils.crc64\_digest / LZMA::Utils.crc64\_hexdigest
    これらは LZMA::Utils::CRCXX.digest や LZMA::Utils::CRCXX.hexdigest を呼び出します。

## liblzma-0.3 (2013-2-21)

(NO INFORMATION)

## liblzma-0.2 (2012-3-31)

(NO INFORMATION)

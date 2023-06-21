#!ruby

=begin
# 必要と思われる試験項目

* LZMA.encode
  * LZMA.decode で伸長できるか
  * xz-cli で伸長できるか
  * 汚染状態の伝搬
  * security level
* LZMA.encode_file
  * LZMA.decode_file で伸長できるか
  * xz-cli で伸長できるか
* LZMA.decode
* LZMA.decode_file
* LZMA.test_file

* 試験で用いる試料
  * /usr/ports/INDEX-10
  * /boot/kernel/kernel
  * 長さ 0 の空データ
  * 0 で埋められた小さなデータ
  * 0 で埋められたでかいデータ
  * 0xaa で埋められた小さなデータ
  * 0xaa で埋められたでかいデータ
  * /dev/random (4000 bytes)
  * /dev/random (12000000 bytes)
  * 可能であれば数十 GB レベルのファイル
=end

require "test-unit"
require "openssl" # for OpenSSL::Random.random_bytes
require "extlzma"

require_relative "sampledata"

class TestStreamAPI < Test::Unit::TestCase
  SAMPLES.each_pair do |name, data|
    define_method("test_encode_decode_sample:#{name}", -> {
      assert(data, LZMA.decode(LZMA.encode(data)))
    }) if data
  end

  def test_encode_args
    assert_kind_of(LZMA::Encoder, LZMA.encode)
    assert_kind_of(LZMA::Encoder, LZMA.encode(StringIO.new("")))
    assert_kind_of(String, LZMA.encode { |e| e.outport })
    io = StringIO.new("")
    assert_same(io, LZMA.encode(io) { |e| io })
    assert_kind_of(LZMA::Encoder, LZMA.encode(io, 9))
  end

  def test_decode_args
    assert_raise(ArgumentError) { LZMA.decode }
    assert_raise(NoMethodError) { LZMA.decode(nil).read } # undefined method `read' for nil:NilClass
    assert_raise(LZMA::BufError) { LZMA.decode("") } # read error (or already EOF)
  end
end

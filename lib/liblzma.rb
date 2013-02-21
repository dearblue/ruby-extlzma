#vim: set fileencoding:utf-8

ver = RbConfig::CONFIG["ruby_version"]
lib = File.join(File.dirname(__FILE__), ver, "liblzma.so")
if File.file?(lib)
  require_relative File.join(ver, "liblzma.so")
else
  require_relative "liblzma.so"
end


#--
# This comment is avoid the problem that is recognized binary by rdoc
#                                                                       #
#                                                                       #
#                                                                       #
#                                                                       #
#                                                                       #
#                                                                       #
#                                                                       #
#                                                                       #
#                                                                       #
#                                                                       #
#                                                                       #
#                                                                       #
#++

module LibLZMA
  LZMA = self

  # call-seq:
  # LibLZMA.encode(data, ...) -> encoded_xz_data
  #
  # データを圧縮します。圧縮されたデータ列は、xz ファイルフォーマットとなるため、コマンドラインの xz などで伸張させることが可能です。
  #
  # [data]        圧縮元となるデータを String インスタンスで渡します。liblzma はエンコーディング情報を無視し、そのままのバイト列をバイナリデータと見立て処理を行います。
  #
  # [options]     LZMA::Filter::LZMA2.new に渡される可変引数です。詳細は LZMA::Filter::LZMA2.new を見てください。
  #
  # [RETURN]      xz データストリームとしての String インスタンスです。
  #
  # [EXCEPTIONS]  (NO DOCUMENT)
  module_function
  def encode(data, *args)
    s = Stream.encoder(Filter.lzma2(*args))
    s.update(data, LZMA::FINISH)
  ensure
    s.finish rescue nil if s
  end

  # call-seq:
  # LZMA.decode(encoded_data, ...) -> string
  #
  # 圧縮されたデータを伸張します。
  #
  # [encoded_data] 圧縮されたデータを与えます。圧縮されたデータの形式は、xz と lzma です。これらはあらかじめ区別する必要なく与えることが出来ます。
  #
  # [options]     LZMA::Filter::LZMA2.new に渡される可変引数です。詳細は LZMA::Filter::LZMA2.new を見てください。
  #
  # [RETURN]      xz データストリームとしての String インスタンスです。
  #
  # [EXCEPTIONS]  (NO DOCUMENT)
  module_function
  def decode(data, *args)
    s = Stream.auto_decoder(*args)
    s.update(data, LZMA::FINISH)
  ensure
    s.finish rescue nil if s
  end

  # call-seq:
  # LZMA.raw_encode(data, ...) -> encoded_data
  #
  # データを圧縮します。圧縮されたデータ列は生の lzma1/lzma2 のデータ列であるため、伸張する際に適切なフィルタを与える必要があります。
  #
  # xz ファイルフォーマットヘッダや整合値を持たないため、これらが不要な場合は有用かもしれません。
  #
  # [data]        圧縮元となるデータを String インスタンスで渡します。liblzma はエンコーディング情報を無視し、そのままのバイト列をバイナリデータと見立てて処理を行います。
  #
  # [options]     LZMA::Filter のインスタンスを与えます。
  #
  # [RETURN]      生の lzma1/lzma2 のデータ列となる String インスタンスです。
  #
  # [EXCEPTIONS]  (NO DOCUMENT)
  module_function
  def raw_encode(data, *args)
    s = Stream.raw_encoder(*args)
    s.update(data, LZMA::FINISH)
  ensure
    s.finish rescue nil if s
  end

  # call-seq:
  # LZMA.raw_decode(encoded_data, ...) -> decoded_data
  #
  # 圧縮されたデータを伸張します。圧縮した際に用いたフィルタをそのままの順番・数で与える必要があります。
  #
  # [data]        圧縮された生の lzma1/lzma2 の String インスタンスを渡します。liblzma はエンコーディング情報を無視し、そのままのバイト列をバイナリデータと見立てて処理を行います。
  #
  # [options]     LZMA::Filter のインスタンスを与えます。
  #
  # [RETURN]      伸張されたデータ列となる String インスタンスです。
  #
  # [EXCEPTIONS]  (NO DOCUMENT)
  module_function
  def raw_decode(data, *args)
    s = Stream.raw_decoder(*args)
    s.update(data, LZMA::FINISH)
  ensure
    s.finish rescue nil if s
  end

  class Stream
    def self.encoder(*args)
      Encoder.new(*args)
    end

    def self.decoder(*args)
      Decoder.new(*args)
    end

    def self.auto_decoder(*args)
      AutoDecoder.new(*args)
    end

    def self.raw_encoder(*args)
      RawEncoder.new(*args)
    end

    def self.raw_decoder(*args)
      RawDecoder.new(*args)
    end
  end

  module Utils
    # CRC32 を Digest のように生成できるようになります。
    class CRC32 < Struct.new(:state, :init)
      def initialize(state = 0)
        state = state.to_i
        super(state, state)
      end

      # call-seq:
      # LZMA::Utils::CRC32#update(data) -> self
      def update(data)
        self.state = Utils.crc32(data, state)
        self
      end

      alias << update

      def finish
        self
      end

      def reset
        self.state = init
        self
      end

      def digest
        [state].pack("N")
      end

      def hexdigest
        "%08x" % state
      end

      alias to_s hexdigest

      def to_str
        "CRC32 <#{hexdigest}>"
      end

      alias inspect to_str
    end

    # CRC64 を Digest のように生成できるようになります。
    class CRC64 < Struct.new(:state, :init)
      def initialize(state = 0)
        state = state.to_i
        super(state, state)
      end

      # call-seq:
      # LZMA::Utils::CRC64#update(data) -> self
      def update(data)
        self.state = Utils.crc64(data, state)
        self
      end

      alias << update

      def finish
        self
      end

      def reset
        self.state = init
        self
      end

      def digest
        [state].pack("Q>")
      end

      def hexdigest
        "%016x" % state
      end

      alias to_s hexdigest

      def to_str
        "CRC64 <#{hexdigest}>"
      end

      alias inspect to_str
    end
  end
end

LZMA = LibLZMA unless Object.const_defined?(:LZMA)

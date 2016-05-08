#vim: set fileencoding:utf-8

begin
  libname = File.basename(__FILE__, ".rb") << ".so"
  require_relative libname
rescue LoadError
  ver = RUBY_VERSION.slice(/\d+\.\d+/)
  require_relative "#{ver}/#{libname}"
end

require_relative "extlzma/version"
require "stringio"

module LZMA
  #
  # call-seq:
  #   encode(string_data, preset = LZMA::PRESET_DEFAULT, opts = {}) -> encoded_xz_data
  #   encode(string_data, filter...) -> encoded_xz_data
  #   encode(output_stream = nil, preset = LZMA::PRESET_DEFAULT, opts = {}) -> stream_encoder
  #   encode(output_stream, filter...) -> stream_encoder
  #   encode(output_stream = nil, preset = LZMA::PRESET_DEFAULT, opts = {}) { |encoder| ... } -> yield return value
  #   encode(output_stream, filter...) { |encoder| ... } -> yield return value
  #
  # データを圧縮、または圧縮器を生成します。
  #
  # 圧縮されたデータ列は xz ファイルフォーマットとなるため、コマンドラインの xz などで伸張させることが可能です。
  #
  # [RETURN encoded_xz_data]
  #   xz データストリームとしての String インスタンスです。
  # [RETURN stream_encoder]
  #   xz データストリームを生成する圧縮器を返します。
  # [RETURN output_stream]
  #   引数として渡した <tt>output_stream</tt> そのものを返します。
  # [string_data]
  #   圧縮元となるデータを String インスタンスで渡します。
  #   liblzma はエンコーディング情報を無視し、そのままのバイト列をバイナリデータと見立て処理を行います。
  # [preset = LZMA::PRESET_DEFAULT]
  #   圧縮プリセット値を指定します (圧縮レベルのようなものです)。詳細は LZMA::Filter::LZMA2.new を見てください。
  # [opts]
  #   LZMA::Filter::LZMA2.new に渡される可変引数です。詳細は LZMA::Filter::LZMA2.new を見てください。
  # [filter]
  #   LZMA::Filter で定義されているクラスのインスタンスを指定します。最大4つまで指定することが出来ます。
  # [output_stream]
  #   圧縮データの受け皿となるオブジェクトを指定します。
  #
  #   <tt>.<<</tt> メソッドが呼ばれます。
  # [YIELD RETURN]
  #   無視されます。
  # [YIELD encoder]
  #   圧縮器が渡されます。
  #
  # [EXCEPTIONS]
  #   (NO DOCUMENT)
  #
  def self.encode(src = nil, *args, &block)
    Aux.encode(src, Stream.encoder(*args), &block)
  end

  #
  # call-seq:
  #   decode(string_data) -> decoded data
  #   decode(string_data, filter...) -> decoded data
  #   decode(input_stream) -> decoder
  #   decode(input_stream, filter...) -> decoder
  #   decode(input_stream) { |decoder| ... }-> yield return value
  #   decode(input_stream, filter...) { |decoder| ... }-> yield return value
  #
  # 圧縮されたデータを伸張します。
  #
  # [RETURN decoded data]
  #   xz データストリームとしての String インスタンスです。
  # [RETURN decoder]
  # [RETURN yield return value]
  # [string_data]
  #   圧縮されたデータを与えます。圧縮されたデータの形式は xz と lzma です。これらはあらかじめ区別する必要なく与えることが出来ます。
  # [options]
  #   LZMA::Filter::LZMA2.new に渡される可変引数です。詳細は LZMA::Filter::LZMA2.new を見てください。
  # [EXCEPTIONS]
  #   (NO DOCUMENT)
  #
  def self.decode(src, *args, &block)
    Aux.decode(src, Stream.auto_decoder(*args), &block)
  end

  #
  # call-seq:
  #   LZMA.raw_encode(src) -> encoded data
  #   LZMA.raw_encode(src, filter...) -> encoded data
  #   LZMA.raw_encode(outport = nil) -> encoder
  #   LZMA.raw_encode(outport, filter...) -> encoder
  #   LZMA.raw_encode(outport = nil) { |encoder| ... } -> yield return value
  #   LZMA.raw_encode(outport, filter...)  { |encoder| ... } -> yield return value
  #
  # データを圧縮します。圧縮されたデータ列は生の lzma1/lzma2 のデータ列であるため、伸張する際に適切なフィルタを与える必要があります。
  #
  # xz ファイルフォーマットヘッダや整合値を持たないため、これらが不要な場合は有用かもしれません。
  #
  # [RETURN encoded data]
  #   生の lzma1/lzma2 のデータ列となる String インスタンスです。
  # [src]
  #   圧縮元となるデータを String インスタンスで渡します。
  # [filter]
  #   LZMA::Filter のインスタンスを与えます。最大4つまで指定可能です。
  #
  #   省略時は lzma2 フィルタが指定されたとみなします。
  # [EXCEPTIONS]
  #   (NO DOCUMENT)
  #
  def self.raw_encode(src, *args, &block)
    Aux.encode(src, Stream.raw_encoder(*args), &block)
  end

  # 
  # call-seq:
  #   LZMA.raw_decode(encoded_data) -> decoded data
  #   LZMA.raw_decode(encoded_data, filter...) -> decoded data
  #   LZMA.raw_decode(inport) -> raw decoder
  #   LZMA.raw_decode(inport, filter...) -> raw decoder
  #   LZMA.raw_decode(inport) { |decoder| ... } -> yield return value
  #   LZMA.raw_decode(inport, filter...) { |decoder| ... } -> yield return value
  #
  # 圧縮されたデータを伸張します。圧縮した際に用いたフィルタをそのままの順番・数で与える必要があります。
  #
  # [RETURN decoded data]
  #   伸張されたデータ列となる String インスタンスです。
  # [src]
  #   圧縮された生の lzma1/lzma2 の String インスタンスを渡します。
  # [options]
  #   LZMA::Filter のインスタンスを与えます。最大4つまで指定可能です。
  #
  #   省略時は lzma2 フィルタが指定されたとみなします。
  # [EXCEPTIONS]
  #   (NO DOCUMENT)
  #
  def self.raw_decode(src, *args, &block)
    Aux.decode(src, Stream.raw_decoder(*args), &block)
  end

  def self.lzma1(*args)
    LZMA::Filter::LZMA1.new(*args)
  end

  def self.lzma2(*args)
    LZMA::Filter::LZMA2.new(*args)
  end

  def self.delta(*args)
    LZMA::Filter::Delta.new(*args)
  end

  class Encoder < Struct.new(:context, :outport, :writebuf, :workbuf, :status)
    BLOCKSIZE = 256 * 1024 # 256 KiB

    def initialize(context, outport)
      super(context, outport,
            StringIO.new("".force_encoding(Encoding::BINARY)),
            "".force_encoding(Encoding::BINARY), [1])
      self.class.method(:finalizer_regist).(self, context, outport, writebuf, workbuf, status)
    end

    def write(buf)
      writebuf.rewind
      writebuf.string.clear
      writebuf << buf
      until writebuf.string.empty?
        s = context.code(writebuf.string, workbuf, BLOCKSIZE, 0)
        unless s == 0
          Utils.raise_err s
        end
        outport << workbuf
        workbuf.clear
      end

      self
    end

    alias << write

    def close
      if eof?
        raise "already closed stream - #{inspect}"
      end

      self.class.method(:finalizer_close).(context, outport, workbuf)

      status[0] = nil

      nil
    end

    def eof
      !status[0]
    end

    alias eof? eof

    class << self
      private
      def finalizer_regist(obj, context, outport, writebuf, workbuf, status)
        ObjectSpace.define_finalizer(obj, finalizer_make(context, outport, writebuf, workbuf, status))
      end

      private
      def finalizer_make(context, outport, writebuf, workbuf, status)
        proc do
          if status[0]
            until writebuf.string.empty?
              s = context.code(writebuf.string, workbuf, BLOCKSIZE, 0)
              Utils.raise_err s unless s == LZMA::OK
              outport << workbuf
              workbuf.clear
            end

            finalizer_close(context, outport, workbuf)

            status[0] = nil
          end
        end
      end

      private
      def finalizer_close(context, outport, workbuf)
        while true
          workbuf.clear
          s = context.code(nil, workbuf, BLOCKSIZE, LZMA::FINISH)
          outport << workbuf
          break if s == LZMA::STREAM_END
          Utils.raise_err s unless s == LZMA::OK
        end
      end
    end
  end

  class Decoder < Struct.new(:context, :inport, :readbuf, :workbuf, :status)
    BLOCKSIZE = 256 * 1024 # 256 KiB

    def initialize(context, inport)
      super context, inport,
            "".force_encoding(Encoding::BINARY),
            StringIO.new("".force_encoding(Encoding::BINARY)),
            :ready
    end

    def read(size = nil, buf = "".force_encoding(Encoding::BINARY))
      buf.clear
      size = size.to_i if size
      return buf if size == 0

      tmp = "".force_encoding(Encoding::BINARY)
      while !eof && (size.nil? || size > 0)
        if workbuf.eof?
          fetch or break
        end

        workbuf.read(size, tmp) or break
        buf << tmp
        size -= tmp.bytesize if size
      end

      (buf.empty? ? nil : buf)
    end

    def eof
      !status && workbuf.eof?
    end

    alias eof? eof

    def close
      self.status = nil
      workbuf.rewind
      workbuf.string.clear
      nil
    end

    private
    def fetch
      return nil unless status == :ready

      while workbuf.eof
        if readbuf.empty?
          inport.read(BLOCKSIZE, readbuf)
        end

        workbuf.string.clear
        workbuf.rewind
        if readbuf.empty?
          s = context.code(nil, workbuf.string, BLOCKSIZE, LZMA::FINISH)
        else
          s = context.code(readbuf, workbuf.string, BLOCKSIZE, 0)
        end

        case s
        when LZMA::OK
        when LZMA::STREAM_END
          self.status = :finished
          break
        else
          Utils.raise_err s
        end
      end

      self
    end
  end

  class Stream
    def self.encoder(*args, **opts)
      case
      when args.empty?
        Encoder.new(Filter::LZMA2.new(LZMA::PRESET_DEFAULT), **opts)
      when args.size == 1 && args[0].kind_of?(Numeric)
        Encoder.new(Filter::LZMA2.new(args[0]), **opts)
      else
        Encoder.new(*args, **opts)
      end
    end

    def self.decoder(*args)
      case
      when args.empty?
        Decoder.new(Filter::LZMA2.new(LZMA::PRESET_DEFAULT))
      when args.size == 1 && args[0].kind_of?(Numeric)
        Decoder.new(Filter::LZMA2.new(args[0]))
      else
        Decoder.new(*args)
      end
    end

    def self.auto_decoder(*args)
      AutoDecoder.new(*args)
    end

    def self.raw_encoder(*args)
      case
      when args.size == 0
        RawEncoder.new(Filter::LZMA2.new(LZMA::PRESET_DEFAULT))
      when args.size == 1 && args[0].kind_of?(Numeric)
        RawEncoder.new(Filter::LZMA2.new(args[0]))
      else
        RawEncoder.new(*args)
      end
    end

    def self.raw_decoder(*args)
      case
      when args.size == 0
        RawDecoder.new(Filter::LZMA2.new(LZMA::PRESET_DEFAULT))
      when args.size == 1 && args[0].kind_of?(Numeric)
        RawDecoder.new(Filter::LZMA2.new(args[0]))
      else
        RawDecoder.new(*args)
      end
    end
  end

  class Filter
    def self.lzma1(*args)
      LZMA1.new(*args)
    end

    def self.lzma2(*args)
      LZMA2.new(*args)
    end

    def self.delta(*args)
      Delta.new(*args)
    end
  end

  module Utils
    extend self

    def crc32_digest(seq, init = 0)
      [Utils.crc32(seq, init)].pack("N")
    end

    def crc32_hexdigest(seq, init = 0)
      "%08X" % Utils.crc32(seq, init)
    end

    def crc64_digest(seq, init = 0)
      [Utils.crc64(seq, init)].pack("Q>")
    end

    def crc64_hexdigest(seq, init = 0)
      "%016X" % Utils.crc64(seq, init)
    end

    # 
    # CRC32 を Digest のように生成できるようになります。
    #
    class CRC32 < Struct.new(:state, :init)
      def initialize(state = 0)
        state = state.to_i
        super(state, state)
      end

      # 
      # call-seq:
      #   LZMA::Utils::CRC32#update(data) -> self
      #
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

    # 
    # CRC64 を Digest のように生成できるようになります。
    #
    class CRC64 < Struct.new(:state, :init)
      def initialize(state = 0)
        state = state.to_i
        super(state, state)
      end

      # 
      # call-seq:
      #   LZMA::Utils::CRC64#update(data) -> self
      #
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

    def raise_err(lzma_ret, mesg = nil)
      et = Utils.lookup_error(lzma_ret)
      raise et, mesg, caller
    end
  end

  extend Utils

  #
  # extlzma の内部で利用される補助モジュールです。
  #
  # extlzma の利用者が直接利用することは想定していません。
  #
  module Aux
    def self.encode(src, encoder)
      if src.kind_of?(String)
        s = Encoder.new(encoder, "".force_encoding(Encoding::BINARY))
        s << src
        s.close
        return s.outport
      end

      s = Encoder.new(encoder, (src || "".force_encoding(Encoding::BINARY)))
      return s unless block_given?

      begin
        yield(s)
      ensure
        s.close
      end
    end

    def self.decode(src, decoder)
      if src.kind_of?(String)
        return decode(StringIO.new(src), decoder) { |s| s.read }
      end

      s = Decoder.new(decoder, src)
      return s unless block_given?

      begin
        yield(s)
      ensure
        s.close rescue nil
      end
    end
  end
end

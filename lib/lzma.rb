#vim: set fileencoding:utf-8

require "lzma.so"

module LZMA
    module_function
    def encode(data, *args)
        s = Stream.encoder(Filter.lzma2(*args))
        return s.update(data, LZMA::FINISH)
    rescue
        $!.set_backtrace caller
        raise
    end

    module_function
    def decode(data, *args)
        s = Stream.auto_decoder(*args)
        return s.update(data, LZMA::FINISH)
    rescue
        $!.set_backtrace caller
        raise
    end

    module_function
    def raw_encode(data, *args)
        s = Stream.raw_encoder(*args)
        return s.update(data, LZMA::FINISH)
    rescue
        $!.set_backtrace caller
        raise
    end

    module_function
    def raw_decode(data, *args)
        s = Stream.raw_decoder(*args)
        return s.update(data, LZMA::FINISH)
    rescue
        $!.set_backtrace caller
        raise
    end

    class Stream
        def self.encoder(*args)
            return Encoder.new(*args)
        end

        def self.decoder(*args)
            return Decoder.new(*args)
        end

        def self.auto_decoder(*args)
            return AutoDecoder.new(*args)
        end

        def self.raw_encoder(*args)
            return RawEncoder.new(*args)
        end

        def self.raw_decoder(*args)
            return RawDecoder.new(*args)
        end
    end
end

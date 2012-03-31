system "cd ext && ( if not exist Makefile ( ruby extconf.rb ) ) && make" or exit $?.exitstatus
system "copy /y ext\\liblzma.so lib\\" or exit $?.exitstatus

Gem::Specification.new do |spec|
    spec.name = "liblzma"
    spec.version = "0.2"
    spec.platform = "mingw32"
    spec.summary = "ruby bindings for liblzma that is included in the xz utilities"
    spec.license = "2-clause BSD License"
    spec.author = "dearblue"
    spec.email = "dearblue@users.sourceforge.jp"
    spec.files = %w(
        README.txt
        lib/liblzma.rb
        lib/liblzma.so
        samples/01-easy_using.rb
        samples/02-streaming.rb
    )
    spec.has_rdoc = false
    spec.required_ruby_version = ">= 1.9.3"
end

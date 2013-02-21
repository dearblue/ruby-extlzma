#!ruby
#vim: set fileencoding:utf-8

ruby19 = ENV["RUBY19"] or raise "please set env RUBY19"
ruby20 = ENV["RUBY20"] or raise "please set env RUBY20"

require "fileutils"

FileUtils.mkpath "lib/1.9.1"
FileUtils.mkpath "lib/2.0.0"

system "cd lib\\1.9.1 && ( if not exist Makefile ( #{ruby19} ../../ext/extconf.rb -s ) ) && make" or exit $?.exitstatus
system "cd lib\\2.0.0 && ( if not exist Makefile ( #{ruby20} ../../ext/extconf.rb -s ) ) && make && strip -s *.so" or exit $?.exitstatus

Gem::Specification.new do |spec|
  spec.name = "liblzma"
  spec.version = "0.3"
  spec.platform = "x86-mingw32"
  spec.summary = "ruby bindings for liblzma that is included in the xz utilities"
  spec.license = "2-clause BSD License"
  spec.author = "dearblue"
  spec.email = "dearblue@users.sourceforge.jp"
  spec.files = %w(
    README.txt
    ext/extconf.rb
    ext/liblzma.c
    lib/liblzma.rb
    lib/1.9.1/liblzma.so
    lib/2.0.0/liblzma.so
    samples/01-easy_using.rb
    samples/02-streaming.rb
  )
  spec.has_rdoc = false
  spec.rdoc_options = %w(-eUTF-8 -mREADME.txt README.txt ext/liblzma.c lib/liblzma.rb)
  # spec.extra_rdoc_files = 
  spec.required_ruby_version = ">= 1.9.3"
end

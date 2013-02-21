Gem::Specification.new do |spec|
  spec.name = "liblzma"
  spec.version = "0.3"
  spec.summary = "ruby bindings for liblzma that is included in the xz utilities"
  spec.license = "2-clause BSD License"
  spec.author = "dearblue"
  spec.email = "dearblue@users.sourceforge.jp"
  spec.files = %w(
    README.txt
    ext/extconf.rb
    ext/liblzma.c
    lib/liblzma.rb
    samples/01-easy_using.rb
    samples/02-streaming.rb
  )
  spec.has_rdoc = false
  spec.rdoc_options = %w(-eUTF-8 -mREADME.txt README.txt ext/liblzma.c lib/liblzma.rb)
  spec.required_ruby_version = ">= 1.9.3"
  spec.extensions << "ext/extconf.rb"
end

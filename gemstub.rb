require_relative "lib/extlzma/version"

GEMSTUB = Gem::Specification.new do |s|
  s.name = "extlzma"
  s.version = LZMA::VERSION
  s.summary = "ruby bindings for liblzma in the xz utilities"
  s.description = <<EOS
ruby bindings for liblzma in the xz utilities <http://tukaani.org/xz/>
EOS
  s.license = "BSD-2-Clause"
  s.author = "dearblue"
  s.email = "dearblue@users.osdn.me"
  s.homepage = "https://osdn.jp/projects/rutsubo/"

  s.required_ruby_version = ">= 2.0"
  s.add_development_dependency "rake", "~> 11.0"
end

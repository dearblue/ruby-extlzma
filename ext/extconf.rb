#!ruby

require "optparse"

opt = OptionParser.new

staticlink = nil
opt.on("-s", "link liblzma.a as static library") { staticlink = true }

opt.parse!


require "mkmf"

have_header "lzma.h" or raise "need lzma.h"
have_library "lzma" or raise "need liblzma.a"
if staticlink
  # 静的ライブラリリンクを優先する
  $libs = ["-Wl,-dn,-llzma", "-Wl,-llzma", $libs].join(" ")
end
create_makefile "liblzma"

# Makefile から、『-L.』を取り除く
# (本来のliblzma.aではなく、これから作成するliblzma.soへの結合を抑止するため)
mf = File.read("Makefile", mode: "r:binary")
mf.gsub!(/(?<=\s)-L\.(?=\s|$)/, " ")
File.write("Makefile", mf, mode: "wb")

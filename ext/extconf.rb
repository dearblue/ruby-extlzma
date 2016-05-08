#!ruby

require "mkmf"

needlist = []

have_header "lzma.h" or needlist << "lzma.h"
have_library "lzma" or needlist << "liblzma.a"

unless needlist.empty?
  abort "#$0: dependency files are not found (#{needlist.join(" ")})."
end

staticlink = arg_config("--liblzma-static-link", false)

if staticlink
  # 静的ライブラリリンクを優先する
  $libs = ["-Wl,-dn,-llzma", "-Wl,-llzma", $libs].join(" ")
end

if RbConfig::CONFIG["arch"] =~ /mingw/
  $LDFLAGS << " -static-libgcc " if try_link "void main(void){}", " -static-libgcc "
end

$LDFLAGS << " -Wl,-Bsymbolic " if try_link "void main(void){}", " -Wl,-Bsymbolic "

create_makefile "extlzma"

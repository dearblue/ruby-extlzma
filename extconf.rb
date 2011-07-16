require "mkmf"

$LDFLAGS << " -llzma "
create_makefile("lzma")

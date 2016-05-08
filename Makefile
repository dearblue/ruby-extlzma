all:	native-package mingw32-package

clean:
	-@rake clean RUBYSET=$(rubyset)

native-package:
	rake clean all RUBYSET=$(rubyset)

mingw32-package:
	rake clean all RUBYSET=$(rubyset_mingw32) "EXTCONF=--with-opt-dir=$(HOME)/mingw32/xz-5.0.7-mingw32 --liblzma-static-link"

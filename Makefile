all:
	gem build liblzma-mingw32.gemspec
	gem build liblzma.gemspec

clean:
	@- cd lib/1.9.1 && make clean
	@- cd lib/2.0.0 && make clean

rdoc:
	rdoc -veUTF-8 -mREADME.txt README.txt ext/liblzma.c lib/liblzma.rb

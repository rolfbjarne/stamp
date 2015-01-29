stamp: stamp.c Makefile
	clang stamp.c -g -o$@
	cp $@ ~/bin
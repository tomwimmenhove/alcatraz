all:
	make -C src
	make -C example

clean:
	make -C src clean
	make -C example clean

example: all
	./example/example


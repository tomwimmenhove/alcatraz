all:
	make -C kvmpp/src
	make -C src
	make -C example

clean:
	make -C kvmpp/src clean
	make -C src clean
	make -C example clean

example: all
	./example/example


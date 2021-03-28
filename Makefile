all: newlib/build/libc.a
	make -C kvmpp/src
	make -C src
	make -C example

newlib/build/libc.a:
	mkdir -p newlib/build/
	cd newlib/build/ ; ../newlib/configure --disable-multilib --target=x86_64-pc-none   --build x86_64-elf
	make -C newlib/build/

clean:
	make -C kvmpp/src clean
	make -C src clean
	make -C example clean

example: all
	./example/example


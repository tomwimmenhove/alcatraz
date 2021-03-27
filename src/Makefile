all: host/libhost.a guest/libguest.a

host/libhost.a:
	make -C host

guest/libguest.a:
	make -C guest

clean:
	make -C host clean
	make -C guest clean

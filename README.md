# Alcatraz

Alcatraz is framework to run parts of a C++ application inside a virtual machine sandbox on x64 processors. The sandboxed code can 'call out' to the host using a set of pre-defined functions.

The framework uses [kvmpp](https://github.com/tomwimmenhove/kvmpp/) to create and run KVM instances on Linux and [rpcbuf](https://github.com/tomwimmenhove/rpcbuf/) for calling out from the guest to the host. For libc, [newlib](https://sourceware.org/newlib/) is used.

## Building and running the example
```
git clone --recursive https://github.com/tomwimmenhove/alcatraz.git
cd alcatraz
make example
```

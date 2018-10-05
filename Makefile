PREFIX = /usr/local
dbg = -g
CFLAGS = -O3 $(dbg)


all: libzysys-toolkit.so

zysys-toolkit.o: zysys-toolkit.c zysys-toolkit.h
	gcc -fPIC -pthread -o zysys-toolkit.o -c zysys-toolkit.c $(CFLAGS)

libzysys-toolkit.so: zysys-toolkit.o #let's link library files into a static library
	gcc -shared -o libzysys-toolkit.so zysys-toolkit.o -lm

libs: libzysys-toolkit.so

install: libzysys-toolkit.so
	install -d $(PREFIX)/lib/
	install -m 644 libzysys-toolkit.so $(PREFIX)/lib/
	install -d $(PREFIX)/include/
	install -m 644 zysys-toolkit.h $(PREFIX)/include/

clean:
	rm -f zysys-toolkit *.o *.a *.gch #This way is cleaner than your clean

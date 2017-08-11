all: libzysys-toolkit.so

zysys-toolkit.o: zysys-toolkit.c zysys-toolkit.h
	gcc -o zysys-toolkit.o -c zysys-toolkit.c -O3

libzysys-toolkit.so: zysys-toolkit.o #let's link library files into a static library
	gcc -shared -o libzysys-toolkit.so zysys-toolkit.o -lm

libs: libzysys-toolkit.so

ifeq ($(PREFIX),)
	PREFIX := /usr/local
endif

install: libzysys-toolkit.so
	install -d $(DESTDIR)$(PREFIX)/lib/
	install -m 644 libzysys-toolkit.so  $(DESTDIR)$(PREFIX)/lib/
	install -d $(DESTDIR)$(PREFIX)/include/
	install -m 644 zysys-toolkit.h $(DESTDIR)$(PREFIX)/include/

clean:
	rm -f zysys-toolkit *.o *.a *.gch #This way is cleaner than your clean

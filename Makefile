ALL : fasttext

BGFLAGS := -I/usr/include/freetype2 -I/usr/include/cairo -lrt

OBJECTS := /usr/lib/libfreetype.so /usr/lib/libcairo.so /usr/local/lib/libfcgi.so hash.o qs.o

fasttext: fasttext.h fasttext.c $(OBJECTS)
	$(CC) $(BGFLAGS) $(OBJECTS) fasttext.c -o $@

hash.o: simplehash/hash.h simplehash/hash.c
	$(CC) -c simplehash/hash.c -o $@

qs.o: qs/qs.h qs/qs.c
	$(CC) -c qs/qs.c -o $@

clean:
	rm -rf hash.o qs.o fasttext


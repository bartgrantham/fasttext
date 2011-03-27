ALL : fasttext

BGFLAGS := -I/usr/include/freetype2 -I/usr/include/cairo -lrt

OBJECTS := /usr/lib/libfreetype.so /usr/lib/libcairo.so /usr/lib/libfcgi.so hash.o qs_parse.o

fasttext: fasttext.h fasttext.c $(OBJECTS)
	$(CC) $(BGFLAGS) $(OBJECTS) fasttext.c -o $@

hash.o: simplehash/hash.h simplehash/hash.c
	$(CC) -c simplehash/hash.c -o $@

qs_parse.o: qs_parse/qs_parse.h qs_parse/qs_parse.c
	$(CC) -c qs_parse/qs_parse.c -o $@

clean:
	rm -rf hash.o qs_parse.o fasttext


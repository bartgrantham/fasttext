ALL : fasttext

BGFLAGS := -I/usr/include/freetype2 -I/usr/include/cairo

OBJECTS := /usr/lib/libfreetype.so /usr/lib/libcairo.so /usr/local/lib/libfcgi.so hash.o qs.o

# gcc -I/usr/include/freetype2 -I/usr/include/cairo textrender.c /usr/lib/libfreetype.so /usr/lib/libcairo.so /usr/local/lib/libfcgi.so hash.o -o textrender

fasttext: fasttext.c hash.o qs.o
	$(CC) $(BGFLAGS) $(OBJECTS) fasttext.c -o $@

hash.o: hash/hash.h hash/hash.c
	$(CC) -c hash/hash.c -o $@

qs.o: qs/qs.h qs/qs.c
	$(CC) -c qs/qs.c -o $@

clean:
	rm -rf hash.o qs.o textrender


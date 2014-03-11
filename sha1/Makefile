SOLIB=libsha1.so
STATICLIB=libsha1.a
CFLAGS=-fpic
all:$(SOLIB) $(STATICLIB)

libsha1.so:sha1.o
	gcc -shared -o $@ $^

libsha1.a:sha1.o
	ar -r $@ $^

clean:
	rm -rf *.o $(SOLIB) $(STATICLIB)

.PHONY: all clean

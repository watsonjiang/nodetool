LDFLAGS=-lpthread \
        -L leveldb -lleveldb \
        -L sha1 -lsha1 

CXXFLAGS=-I leveldb/include \
         -I sha1 \
         -I /home/watson/myshard/trunk/hadb_publisher \
         -fpermissive \
         -D_DEBUG_ \
         -fPIC \
         -g

OBJS=hashtree.o inputmsg.o filtermsg.o

BINS=nodemon

LIBS=pyaae.so

TESTBINS=hashtree_test filtermsg_test inputmsg_test

#all:$(BINS) $(LIBS)
all:$(LIBS)

test:$(TESTBINS)

pyaae.so: pywrapper.o $(OBJS) libmsgclient.a libsox.a libconfig.a
	g++ --shared $(LDFLAGS) -o $@ $^ 

nodemon:nodemon.o $(OBJS) libmsgclient.a libsox.a libconfig.a
	g++ $(LDFLAGS) -o $@ $^

hashtree_test:hashtree_test.o $(OBJS) libmsgclient.a libsox.a libconfig.a 
	g++ $(LDFLAGS) -o $@ $^

filtermsg_test:filtermsg_test.o $(OBJS) libmsgclient.a libsox.a libconfig.a 
	g++ $(LDFLAGS) -o $@ $^

inputmsg_test:inputmsg_test.o $(OBJS) libmsgclient.a libsox.a libconfig.a 
	g++ $(LDFLAGS) -o $@ $^

pywrapper.o:pywrapper.cpp
	g++ -I/usr/include/python2.7 $(CXXFLAGS) -c -o $@ $^

clean:
	rm -rf *.o $(TESTBINS) $(LIBS)

.PHONY:all clean test

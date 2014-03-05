LDFLAGS=-lpthread \
        -L ht -lht \
        -L leveldb -lleveldb \
        -L sha1 -lsha1 \
        -L mysql -lmysqlclient

CXXFLAGS=-I leveldb/include \
         -I sha1 \
         -I ht \
         -I mysql/mysql \
         -I /home/watson/myshard/trunk/hadb_publisher \
         -fpermissive \
         -D_DEBUG_ \
         -g

OBJS=hashtree.o inputmsg.o console.o readline.o filtermsg.o

BINS=nodetool

TESTBINS=console_test hashtree_test filtermsg_test inputmsg_test

all:$(BINS)

test:$(TESTBINS)

nodetool:nodetool.o $(OBJS) libmsgclient.a libsox.a libconfig.a
	g++ $(LDFLAGS) -o $@ $^

console_test:console_test.o $(OBJS) libmsgclient.a libsox.a libconfig.a 
	g++ $(LDFLAGS) -o $@ $^

hashtree_test:hashtree_test.o $(OBJS) libmsgclient.a libsox.a libconfig.a 
	g++ $(LDFLAGS) -o $@ $^

filtermsg_test:filtermsg_test.o $(OBJS) libmsgclient.a libsox.a libconfig.a 
	g++ $(LDFLAGS) -o $@ $^

inputmsg_test:inputmsg_test.o $(OBJS) libmsgclient.a libsox.a libconfig.a 
	g++ $(LDFLAGS) -o $@ $^

clean:
	rm -rf *.o $(BINS) $(TESTBINS)

.PHONY:all clean test

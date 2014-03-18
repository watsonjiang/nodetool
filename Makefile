LDFLAGS=-lpthread \
        -L leveldb -lleveldb \
        -L sha1 -lsha1 \
        -L mysql -lmysqlclient -lmysqlpp \
        -L crypto -lcrypto
        

CXXFLAGS=-I leveldb/include \
         -I sha1 \
         -I /home/watson/myshard/trunk/hadb_publisher \
         -fpermissive \
         -DYYDEBUG=1 \
         -fPIC \
         -g

YACC=bison

LEX=flex

OBJS=hashtree.o filtermsg.o dmpfileparser.lex.o dmpfileparser.yacc.o inputmsg.o debug.o

BINS=nodemon

LIBS=pyaae.so

#all:$(BINS) $(LIBS)
all:$(LIBS)

pyaae.so: pywrapper.o $(OBJS) libshardpub.a libsox.a libconfig.a 
	g++ --shared $(LDFLAGS) -o $@ $^ 

pywrapper.o:pywrapper.cpp dmpfileparser.yacc.h
	g++ -I/usr/include/python2.7 $(CXXFLAGS) -c -o $@ pywrapper.cpp

clean:
	rm -rf *.o $(TESTBINS) $(LIBS) *.lex.cpp *.yacc.h *.yacc.cpp

.cpp.o:
	$(CXX) -I/usr/include/python2.7 -c ${CXXFLAGS} -o $@ $<
   
dmpfileparser.yacc.cpp dmpfileparser.yacc.h: dmpfileparser.y
	$(YACC) -d $<
	mv dmpfileparser.tab.c dmpfileparser.yacc.cpp
	mv dmpfileparser.tab.h dmpfileparser.yacc.h

dmpfileparser.lex.cpp: dmpfileparser.l dmpfileparser.yacc.h
	$(LEX) $<
	mv lex.yy.c dmpfileparser.lex.cpp


.PHONY:all clean test

.SUFFIXES:
.SUFFIXES: .cpp .o .y .lex


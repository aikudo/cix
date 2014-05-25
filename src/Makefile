# $Id: Makefile,v 1.1 2014-05-25 12:44:05-07 - - $

GPP        = g++ -g -O0 -Wall -Wextra -std=gnu++11

DEPFILE    = Makefile.dep
HEADERS    = sockets.h cixlib.h logstream.h
CPPSRCS    = sockets.cpp cixlib.cpp \
             cixdaemon.cpp cixclient.cpp cixserver.cpp
CLIENTOBJS = cixclient.o sockets.o cixlib.o
SERVEROBJS = cixserver.o sockets.o cixlib.o
DAEMONOBJS = cixdaemon.o sockets.o cixlib.o
OBJECTS    = ${CLIENTOBJS} ${SERVEROBJS} ${DAEMONOBJS}
EXECBINS   = cixclient cixserver cixdaemon
LISTING    = Listing.ps
SOURCES    = ${HEADERS} ${CPPSRCS} Makefile

all: ${DEPFILE} ${EXECBINS}

cixclient: ${CLIENTOBJS}
	${GPP} -o $@ ${CLIENTOBJS}

cixserver: ${SERVEROBJS}
	${GPP} -o $@ ${SERVEROBJS}

cixdaemon: ${DAEMONOBJS}
	${GPP} -o $@ ${DAEMONOBJS}

%.o: %.cpp
	${GPP} -c $<

ci:
	- checksource ${SOURCES}
	- cid + ${SOURCES}
	@ git add ${SOURCES}
	@ git commit -m "autocommited on `date`"

lis: all ${SOURCES} ${DEPFILE}
	mkpspdf ${LISTING} ${SOURCES} ${DEPFILE}

clean:
	- rm ${LISTING} ${LISTING:.ps=.pdf} ${OBJECTS}

spotless: clean
	- rm ${EXECBINS}

dep:
	- rm ${DEPFILE}
	make --no-print-directory ${DEPFILE}

${DEPFILE}:
	${GPP} -MM ${CPPSRCS} >${DEPFILE}

again: ${SOURCES}
	make --no-print-directory spotless ci all lis

include ${DEPFILE}


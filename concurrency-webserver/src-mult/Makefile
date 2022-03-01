# An admittedly primitive Makefile
# To compile, type "make" or make "all"
# To remove files, type "make clean"

CC = gcc
CFLAGS = -Wall -g -pthread
OBJS = wserver.o wclient.o request.o io_helper.o 
TEST_FILES_DIR = ../test_files

.SUFFIXES: .c .o 

all: wserver wclient spin.cgi adder.cgi

wserver: wserver.o request.o io_helper.o
	$(CC) $(CFLAGS) -o wserver wserver.o request.o io_helper.o 

wclient: wclient.o io_helper.o
	$(CC) $(CFLAGS) -o wclient wclient.o io_helper.o

%.cgi: %.c
	$(CC) $(CFLAGS) -o $@ $< ;\
	cp $@ ${TEST_FILES_DIR};

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	-rm -f $(OBJS) wserver wclient spin.cgi all.cgi

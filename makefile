DEPS= globals.h user.h list.h sockets.h comm.h udpComm.h commands.h 
OBJ=  user.o list.o sockets.o  comm.o udpComm.o commands.o main.o
CFLAGS= -g -Wall -pedantic 
CC= gcc
 
all: dd

%.o: %.c $(DEPS)
	$(CC) -c -g -o $@ $< $(CFLAGS)
 
dd: $(OBJ)
	$(CC) -g -o $@ $^ $(CFLAGS)
	rm -f $^

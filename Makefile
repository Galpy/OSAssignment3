#IDIR = ../include	 if include files are in a dir
CC=g++
CFLAGS=-I.
MAKE = make
RM = rm
DEPS = 		#if you have .h files that .o files are dependent on
OBJ = client.o toLowerCase.o server.o	#.o files included here

# ODIR = obj 	this is .o file dir
# LDIR = ../lib 	this is local lib dir

LIBS = -lm

# _DEPS = hellomake.h
# DEPS = $(patsubst %,$(IDIR)/%,%(_DEPS)) 

# _OBJ = hellomake.o hellofunc.o
# OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

# 	$(ODIR)/%.o: %.c $(DEPS)
#	$(CC) -c -o $@ $< $(CFLAGS)

# 	hellomake: $(OBJ)
#	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

all: p1.cpp p2.cpp
	$(CC) -o p1 p1.cpp -lrt -pthread
	$(CC) -o p2 p2.cpp -lrt -pthread
	gnome-terminal -t p1_target --working-directory=/home/galpy/CMSC4153/Assignment3
	gnome-terminal -t p2_target --working-directory=/home/galpy/CMSC4153/Assignment3
	sleep 4s

server_target:
	./p1 test2.txt

client_target: 
	./p2 &

clean:
	rm server 
	rm client


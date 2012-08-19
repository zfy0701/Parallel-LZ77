VERSION = 1.0
SHELL = /bin/sh

CC = g++
#CFLAGS = -g -pg
CFLAGS = -O3 -DNDEBUG
LIBS = -lm
OBJS = utils.o RMQ_succinct.o mmsearch.o main.o 

EXEC = lz

.cpp.o:
	$(CC) $(CFLAGS) -DSA_K=4 -I/home/gog/sdsl2/build/include/ -c $<

all: $(EXEC) lz_new lz_new_bwd lz_new_bwd1 lz_new_bwd2 lz_new_bwd4 lz_new_bwd8 lz_new_bwd16 lz_new_bwd32 main_new_bwd_debug

lz: $(OBJS)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJS) $(LIBS)

lz_new: main_new.o utils.o
	$(CC) $(CFLAGS) -o lz_new main_new.o utils.o 

#lz_new_bwd: 
#	$(CC) $(CFLAGS) -DSA_K=4 -I/home/gog/sdsl2/build/include/ -c main_new_bwd.cpp -o main_new_bwd4.o
#	$(CC) $(CFLAGS) -o lz_new_bwd main_new_bwd4.o utils.o /home/gog/sdsl2/build/lib/libsdsl.a 
#
#	
#lz_new_bwd_debug: 
#	$(CC) $(CFLAGS) -DSA_K=1 -I/home/gog/sdsl2/build/include/ -c main_new_bwd_debug.cpp -o main_new_bwd_debug.o
#	$(CC) $(CFLAGS) -o lz_new_bwd_debug main_new_bwd_debug.o utils.o /home/gog/sdsl2/build/lib/libsdsl.a 
#
#
#lz_new_bwd1: main_new_bwd.o utils.o
#	$(CC) $(CFLAGS) -DSA_K=1 -I/home/gog/sdsl2/build/include/ -c main_new_bwd.cpp -o main_new_bwd1.o
#	$(CC) $(CFLAGS) -o lz_new_bwd1 main_new_bwd1.o utils.o /home/gog/sdsl2/build/lib/libsdsl.a 
#
#
#lz_new_bwd2: main_new_bwd.o utils.o
#	$(CC) $(CFLAGS) -DSA_K=2 -I/home/gog/sdsl2/build/include/ -c main_new_bwd.cpp -o main_new_bwd2.o
#	$(CC) $(CFLAGS) -o lz_new_bwd2 main_new_bwd2.o utils.o /home/gog/sdsl2/build/lib/libsdsl.a 
#
#
#lz_new_bwd4: main_new_bwd.o utils.o
#	$(CC) $(CFLAGS) -DSA_K=4 -I/home/gog/sdsl2/build/include/ -c main_new_bwd.cpp -o main_new_bwd4.o
#	$(CC) $(CFLAGS) -o lz_new_bwd4 main_new_bwd4.o utils.o /home/gog/sdsl2/build/lib/libsdsl.a 
#
#
#lz_new_bwd8: main_new_bwd.o utils.o
#	$(CC) $(CFLAGS) -DSA_K=8 -I/home/gog/sdsl2/build/include/ -c main_new_bwd.cpp -o main_new_bwd8.o
#	$(CC) $(CFLAGS) -o lz_new_bwd8 main_new_bwd8.o utils.o /home/gog/sdsl2/build/lib/libsdsl.a 
#
#lz_new_bwd16: main_new_bwd.o utils.o
#	$(CC) $(CFLAGS) -DSA_K=16 -I/home/gog/sdsl2/build/include/ -c main_new_bwd.cpp -o main_new_bwd16.o
#	$(CC) $(CFLAGS) -o lz_new_bwd16 main_new_bwd16.o utils.o /home/gog/sdsl2/build/lib/libsdsl.a 
#
#lz_new_bwd32: main_new_bwd.o utils.o
#	$(CC) $(CFLAGS) -DSA_K=32 -I/home/gog/sdsl2/build/include/ -c main_new_bwd.cpp -o main_new_bwd32.o
#	$(CC) $(CFLAGS) -o lz_new_bwd32 main_new_bwd32.o utils.o /home/gog/sdsl2/build/lib/libsdsl.a 
#
clean:
	/bin/rm -f $(EXEC) core *.o 

package:
	tar cv Makefile README *.h *.cpp | gzip > lz.tar.gz

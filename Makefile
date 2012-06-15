
CC=g++
CFLAGS=-g -fopenmp -DOPENMP

all: plz77 
#ansv2 sa

.PHONY: clean

clean: 
	rm -rf plz77 ansv2 sa *~ *.o

ANSV.o: ANSV.cpp ANSV.h
	$(CC) $(CFLAGS) -c $<
	
#RMQ.o: RMQ.cpp RMQ.h
#	$(CC) $(CFLAGS) -c $<
	
#PrefixSum.o: PrefixSum.cpp PrefixSum.h
#	$(CC) $(CFLAGS) -c $<

rangeMin.o: rangeMin.cpp rangeMin.h
	$(CC) $(CFLAGS) -c $<

suffixArray.o: suffixArray.cpp merge.h mysort.h
	$(CC) $(CFLAGS) -c $<

#suffixArrayTest.o: suffixArrayTest.cpp
#	$(CC) $(CFLAGS) -c suffixArrayTest.cpp

#sa: suffixArrayTest.o suffixArray.o rangeMin.o
#	$(CC) $(CFLAGS) -o $@ suffixArrayTest.o suffixArray.o rangeMin.o

#ansv2.o: ansv2.cpp ansv2.h
#	$(CC) $(CFLAGS) -c $<

#ansvTest.o: ansvTest.cpp 
#	$(CC) $(CFLAGS) -c ansvTest.cpp
	
#ansv2: ansvTest.o ansv2.o
#	$(CC) $(CFLAGS) -o $@ ansvTest.o ansv2.o

PLZ77.o: PLZ77.cpp mysort.h merge.h
	$(CC) $(CFLAGS) -c $<

LZ77.o: LZ77.cpp
	$(CC) $(CFLAGS) -c $<

main.o: main.cpp
	$(CC) $(CFLAGS) -c $<

plz77: main.o PLZ77.o LZ77.o ANSV.o rangeMin.o suffixArray.o
	$(CC) $(CFLAGS) -o $@ $^

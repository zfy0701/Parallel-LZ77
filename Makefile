CC = g++
CFLAGS = -O2

ifdef OPENMP
PCC = $(CC)
CFLAGS = -fopenmp -O2 -DOPENMP

else ifdef CILK
PCC = $(CC)
CFLAGS = -O2 -lcilkrts -DCILK

else ifdef IPPROOT
PCC = icpc
CFLAGS = -O2 -DCILKP

else 
PCC = $(CC)
CFLAGS = -g
endif

all: plz77 lz77_1 lz77_2 lz77_3

.PHONY: clean

clean: 
	rm -rf plz77 lz77_1 lz77_2 *~ *.o

ANSV.o: ANSV.cpp ANSV.h
	$(PCC) $(CFLAGS) -c $<

rangeMin.o: rangeMin.cpp rangeMin.h
	$(PCC) $(CFLAGS) -c $<

suffixArray.o: suffixArray.cpp merge.h PSRS.h
	$(PCC) $(CFLAGS) -c $<

segmentTree.o: segmentTree.cpp segmentTree.h
	$(PCC) $(CFLAGS) -c $<

LZ77_1.o: LZ77_1.cpp  test.h
	$(CC) $(CFLAGS) -c $<

lz77_1: LZ77_1.o suffixArray.o rangeMin.o ANSV.o
	$(PCC) $(CFLAGS) -o $@ $^

LZ77_2.o: LZ77_2.cpp  test.h
	$(CC) $(CFLAGS) -c $<

lz77_2: LZ77_2.o suffixArray.o rangeMin.o
	$(PCC) $(CFLAGS) -o $@ $^

LZ77_3.o: LZ77_3.cpp  test.h
	$(CC) $(CFLAGS) -c $<

lz77_3: LZ77_3.o rangeMin.o
	$(PCC) $(CFLAGS) -o $@ $^

PLZ77.o: PLZ77.cpp  test.h
	$(PCC) $(CFLAGS) -c $<
	
plz77: PLZ77.o ANSV.o rangeMin.o suffixArray.o segmentTree.o
	$(PCC) $(CFLAGS) -o $@ $^


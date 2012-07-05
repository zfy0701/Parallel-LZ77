CC = g++
CFLAGS = -O2

ifdef OPENMP
PCC = $(CC)
CFLAGS = -fopenmp -O2 -DOPENMP

else ifdef CILK
PCC = cilk++
CFLAGS = -O2 -DCILK -Wno-cilk-for

else ifdef IPPROOT
PCC = icpc
CFLAGS = -O2 -DCILKP

else 
PCC = $(CC)
CFLAGS = -fopenmp -O2 -DOPENMP
endif

all: plz77 lz771  seqLZ77

.PHONY: clean

clean: 
	rm -rf plz77 *~ *.o

ANSV.o: ANSV.cpp ANSV.h
	$(PCC) $(CFLAGS) -c $<

rangeMin.o: rangeMin.cpp rangeMin.h
	$(PCC) $(CFLAGS) -c $<

suffixArray.o: suffixArray.cpp merge.h PSRS.h
	$(PCC) $(CFLAGS) -c $<

segmentTree.o: segmentTree.cpp segmentTree.h
	$(PCC) $(CFLAGS) -c $<

PLZ77.o: PLZ77.cpp
	$(PCC) $(CFLAGS) -c $<

LZ77.o: LZ77.cpp
	$(PCC) $(CFLAGS) -c $<

lz771: LZ77.o suffixArray.o rangeMin.o
	$(PCC) $(CFLAGS) -o $@ $^

main.o: main.cpp
	$(PCC) $(CFLAGS) -c $<

seqLZ77.o: seqLZ77.cpp
	$(PCC) $(CFLAGS) -c $<

plz77: main.o PLZ77.o ANSV.o rangeMin.o suffixArray.o segmentTree.o
	$(PCC) $(CFLAGS) -o $@ $^

seqLZ77: seqLZ77.o suffixArray.o rangeMin.o
	$(PCC) $(CFLAGS) -o $@ $^

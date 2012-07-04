CC = g++
CFLAGS = -O2

ifdef OPENMP
PCC = $(CC)
CFLAGS = -fopenmp -O2 -DOPENMP

else ifdef CILK
PCC = $(CC)
CFLAGS = -O2 -lcilkrts -DCILK -Wno-cilk-for

else ifdef IPPROOT
PCC = icpc
CFLAGS = -O2 -DCILKP

else 
PCC = $(CC)
CFLAGS = -fopenmp -O2 -DOPENMP
endif


all: plz77 

.PHONY: clean

clean: 
	rm -rf plz77 *~ *.o

ANSV.o: ANSV.cpp ANSV.h
	$(PCC) $(CFLAGS) -c $<

rangeMin.o: rangeMin.cpp rangeMin.h
	$(PCC) $(CFLAGS) -c $<

suffixArray.o: suffixArray.cpp merge.h PSRS.h
	$(PCC) $(CFLAGS) -c $<

PLZ77.o: PLZ77.cpp
	$(PCC) $(CFLAGS) -c $<

LZ77.o: LZ77.cpp
	$(PCC) $(CFLAGS) -c $<

main.o: main.cpp
	$(PCC) $(CFLAGS) -c $<

plz77: main.o PLZ77.o LZ77.o ANSV.o rangeMin.o suffixArray.o
	$(PCC) $(CFLAGS) -o $@ $^

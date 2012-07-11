CC = g++
CFLAGS = -O2

ifdef OPENMP
PCC = $(CC)
CFLAGS = -fopenmp -O2 -DOPENMP
else ifdef CILK
PCC = $(CC)
CFLAGS = -O2 -lcilkrts -DCILK
else ifdef CILKP
PCC = icpc
CFLAGS = -O2 -DCILKP
else 
PCC = $(CC)
CFLAGS = -g
endif


all: plz77_1 plz77_2 lz77_1 lz77_2 lz77_3

.PHONY: clean

clean: 
	rm -rf plz77_? lz77_? *~ *.o *.exe

ANSV.o: ANSV.cpp ANSV.h
	$(PCC) $(CFLAGS) -c $<

rangeMin.o: rangeMin.cpp rangeMin.h
	$(PCC) $(CFLAGS) -c $<

suffixArray.o: suffixArray.cpp suffixArray.h merge.h PSRS.h
	$(PCC) $(CFLAGS) -c $<

mergeSuffixArrayToTree.o: mergeSuffixArrayToTree.cpp suffixTree.h 
	$(PCC) $(CFLAGS) -c $<

segmentTree.o: segmentTree.cpp segmentTree.h
	$(PCC) $(CFLAGS) -c $<

BWT.o: BWT.cpp BWT.h
	$(PCC) $(CFLAGS) -c $<

LZ77_1.o: LZ77_1.cpp  test.h
	$(PCC) $(CFLAGS) -c $<

lz77_1: LZ77_1.o suffixArray.o rangeMin.o ANSV.o BWT.o
	$(PCC) $(CFLAGS) -o $@ $^

LZ77_2.o: LZ77_2.cpp test.h
	$(PCC) $(CFLAGS) -c $<

lz77_2: LZ77_2.o suffixArray.o rangeMin.o BWT.o
	$(PCC) $(CFLAGS) -o $@ $^

LZ77_3.o: LZ77_3.cpp test.h
	$(PCC) $(CFLAGS) -c $<

lz77_3: LZ77_3.o rangeMin.o BWT.o
	$(PCC) $(CFLAGS) -o $@ $^

PLZ77_1.o: PLZ77_1.cpp test.h
	$(PCC) $(CFLAGS) -c $<
	
plz77_1: PLZ77_1.o ANSV.o rangeMin.o suffixArray.o segmentTree.o BWT.o
	$(PCC) $(CFLAGS) -o $@ $^

PLZ77_2.o: PLZ77_2.cpp test.h
	$(PCC) $(CFLAGS) -c $<
	
plz77_2: PLZ77_2.o rangeMin.o suffixArray.o mergeSuffixArrayToTree.o BWT.o
	$(PCC) $(CFLAGS) -o $@ $^

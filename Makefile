ifdef LONG
INTT = -DLONG
endif

CC = g++
CFLAGS = -O2 $(INTT)
LFLAGS =

ifdef OPENMP
PCC = $(CC)
CFLAGS = -fopenmp -O2 -DOPENMP $(INTT)
LFLAGS = -fopenmp
else ifdef CILK
# we should never use old cilk++
# PCC = cilk++
# CFLAGS = -O2 -DCILK -Wno-cilk-for $(INTT)
PCC = $(CC)
CFLAGS = -O2 -fcilkplus -lcilkrts -DCILK
LFLAGS = -O2 -fcilkplus -lcilkrts -DCILK
else ifdef CILKP
PCC = icpc
CFLAGS = -O2 -DCILKP $(INTT)
else
PCC = $(CC)
#CFLAGS = $(INTT)
#CFLAGS = -g
endif

BASIC = parallel.h utils.h
SEQUENCE = sequence.h seq.h $(BASIC)
INTSORT = intSort.h transpose.h
STRINGGEN = $(ITEMGEN) stringGen.h

all: plz77_1 lz77_1

.PHONY: clean

clean:
	rm -rf plz77_? lz77_? *~ *.o *.exe suffixArray

ANSV.o: ANSV.cpp ANSV.h
	$(PCC) $(CFLAGS) -c $<

rangeMin.o: rangeMin.cpp rangeMin.h
	$(PCC) $(CFLAGS) -c $<

suffixArray.o: suffixArray.cpp suffixArray.h merge.h PSRS.h $(INTSORT) $(SEQUENCE)
	$(PCC) $(CFLAGS) -c $<

mergeSuffixArrayToTree.o: mergeSuffixArrayToTree.cpp suffixTree.h
	$(PCC) $(CFLAGS) -c $<

segmentTree.o: segmentTree.cpp segmentTree.h
	$(PCC) $(CFLAGS) -c $<

LZ77_1.o: LZ77_1.cpp  test.h
	$(PCC) $(CFLAGS) -c $<

lz77_1: LZ77_1.o suffixArray.o rangeMin.o ANSV.o
	$(PCC) $(LFLAGS) -o $@ $^

LPF_LZ.o: LPF_LZ.cpp
	$(PCC) $(CFLAGS) -c $<

PLZ77_1.o: PLZ77_1.cpp test.h transpose.h intSort.h sequence.h
	$(PCC) $(CFLAGS) -c $<

plz77_1: PLZ77_1.o ANSV.o rangeMin.o suffixArray.o segmentTree.o LPF_LZ.o
	$(PCC) $(LFLAGS) -o $@ $^
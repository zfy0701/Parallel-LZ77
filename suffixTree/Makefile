# // This code is part of the Problem Based Benchmark Suite (PBBS)
# // Copyright (c) 2011 Guy Blelloch, Julian Shun and the PBBS team
# //
# // Permission is hereby granted, free of charge, to any person obtaining a
# // copy of this software and associated documentation files (the
# // "Software"), to deal in the Software without restriction, including
# // without limitation the rights (to use, copy, modify, merge, publish,
# // distribute, sublicense, and/or sell copies of the Software, and to
# // permit persons to whom the Software is furnished to do so, subject to
# // the following conditions:
# //
# // The above copyright notice and this permission notice shall be included
# // in all copies or substantial portions of the Software.
# //
# // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# // OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# // MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# // NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# // LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# // OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# // WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

CC = /usr/bin/g++
LFLAGS =
CFLAGS = -O2

ifdef OPENMP
PCC = /usr/bin/g++
PCFLAGS = -fopenmp -O2 -DOPENMP
PLFLAGS = -fopenmp

else ifdef CILK
PCC = cilk++
PCFLAGS = -O2 -DCILK -Wno-cilk-for
PLFLAGS =

else ifdef IPPROOT
PCC = icpc
PCFLAGS = -O2 -DCILKP
PLFLAGS =
CC = icpc
LFLAGS =
CFLAGS = -O2

else 
PCC = $(CC)
PLFLAGS = $(LFLAGS)
PCFLAGS = $(CFLAGS)
endif


BASIC = cilk.h utils.h
SEQUENCE = sequence.h $(BASIC)
INTSORT = intSort.h transpose.h
STRINGGEN = $(ITEMGEN) stringGen.h
ALL= suffixArray suffixTree

all : $(ALL)

suffixArrayTest.o: suffixArrayTest.C $(STRINGGEN) $(SEQUENCE)
	$(PCC) $(PCFLAGS) -c suffixArrayTest.C

rangeMin.o: rangeMin.C rangeMin.h $(BASIC) $(SEQUENCE)
	$(PCC) $(PCFLAGS) -c rangeMin.C

mergeSuffixArrayToTree.o: mergeSuffixArrayToTree.C suffixTree.h cartesianTree.h $(SEQUENCE) $(INTSORT) hash.h 
	$(PCC) $(PCFLAGS) -c mergeSuffixArrayToTree.C

suffixTreeTest.o: suffixTreeTest.C $(STRINGGEN) $(SEQUENCE) hash.h suffixTree.h 
	$(PCC) $(PCFLAGS) -c suffixTreeTest.C

suffixTree: suffixArray.o rangeMin.o suffixTreeTest.o mergeSuffixArrayToTree.o
	$(PCC) $(PLFLAGS) -o $@ suffixTreeTest.o suffixArray.o rangeMin.o mergeSuffixArrayToTree.o

suffixArray.o: suffixArray.C merge.h $(INTSORT) $(SEQUENCE)
	$(PCC) $(PCFLAGS) -c suffixArray.C

suffixArray : suffixArrayTest.o suffixArray.o rangeMin.o
	$(PCC) $(PLFLAGS) -o $@ suffixArrayTest.o suffixArray.o rangeMin.o

clean :
	rm -f *.o $(ALL)

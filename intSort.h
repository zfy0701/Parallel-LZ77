// This code is part of the Problem Based Benchmark Suite (PBBS)
// Copyright (c) 2011 Guy Blelloch, Julian Shun and the PBBS team
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights (to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef A_RADIX_INCLUDED
#define A_RADIX_INCLUDED

#include <iostream>
#include <math.h>
#include "parallel.h"
#include "gettime.h"
#include "sequence.h"
#include "utils.h"
#include "transpose.h"
using namespace std;

namespace intSort {

  // Cannot be greater than 8 without changing definition of bIndexT
  //    from unsigned char to unsigned int (or unsigned short)
#define MAX_RADIX 8
#define BUCKETS 256    // 1 << MAX_RADIX

  typedef intT bucketsT[BUCKETS];

  // a type that must hold MAX_RADIX bits
  typedef unsigned char bIndexT;

  template <class E, class F>
  void radixBlock(E* A, E* B, bIndexT *Tmp, bucketsT counts, bucketsT offsets,
		  intT Boffset, intT n, intT m, F extract) {
    for (intT i = 0; i < m; i++)  counts[i] = 0;
    for (intT j = 0; j < n; j++) {
      intT k = Tmp[j] = extract(A[j]);
      counts[k]++;
    }
    intT s = Boffset;
    for (intT i = 0; i < m; i++) {
      s += counts[i];
      offsets[i] = s;
    }
    for (intT j = n-1; j >= 0; j--) {
      intT x =  --offsets[Tmp[j]];
      B[x] = A[j];
    }
  }

  template <class E, class F>
  void radixStepSerial(E* A, E* B, bIndexT *Tmp, bucketsT buckets,
		       intT n, intT m, F extract) {
    radixBlock(A, B, Tmp, buckets, buckets, 0, n, m, extract);
    for (intT i=0; i < n; i++) A[i] = B[i];
    return;
  }

  // A is the input and sorted output (length = n)
  // B is temporary space for copying data (length = n)
  // Tmp is temporary space for extracting the bytes (length = n)
  // BK is an array of bucket sets, each set has BUCKETS integers
  //    it is used for temporary space for bucket counts and offsets
  // numBK is the length of BK (number of sets of buckets)
  // the first entry of BK is also used to return the offset of each bucket
  // m is the number of buckets per set (m <= BUCKETS)
  // extract is a function that extract the appropriate bits from A
  //  it must return a non-negative integer less than m
  template <class E, class F>
  void radixStep(E* A, E* B, bIndexT *Tmp, bucketsT *BK,
		 intT numBK, intT n, intT m, bool top, F extract) {

    // need 3 bucket sets per block
    int expand = (sizeof(E)<=4) ? 64 : 32;
    intT blocks = min(numBK/3,(1+n/(BUCKETS*expand)));

    if (blocks < 2) {
      radixStepSerial(A, B, Tmp, BK[0], n, m, extract);
      return;
    }
    intT nn = (n+blocks-1)/blocks;
    intT* cnts = (intT*) BK;
    intT* oA = (intT*) (BK+blocks);
    intT* oB = (intT*) (BK+2*blocks);

    parallel_grainsize_1
    parallel_for (intT i=0; i < blocks; i++) {
      intT od = i*nn;
      intT nni = min<intT>(max<intT>(n-od,0),nn);
      radixBlock(A+od, B, Tmp+od, cnts + m*i, oB + m*i, od, nni, m, extract);
    }

    transpose<intT>(cnts, oA).trans(blocks, m);

    intT ss;
    if (top)
      ss = sequence::scan(oA, oA, blocks*m, utils::addF<intT>(),(intT)0);
    else
      ss = sequence::scanSerial(oA, oA, blocks*m, utils::addF<intT>(),(intT)0);
    //utils::myAssert(ss == n, "radixStep: sizes don't match");

    blockTrans<E>(B, A, oB, oA, cnts).trans(blocks, m);

    // put the offsets for each bucket in the first bucket set of BK
    for (intT j = 0; j < m; j++) BK[0][j] = oA[j*blocks];
  }

  // a function to extract "bits" bits starting at bit location "offset"
  template <class E, class F>
    struct eBits {
      F _f;  intT _mask;  intT _offset;
      eBits(intT bits, intT offset, F f): _mask((1<<bits)-1),
					_offset(offset), _f(f) {}
      intT operator() (E p) {return _mask&(_f(p)>>_offset);}
    };

  // Radix sort with low order bits first
  template <class E, class F>
  void radixLoopBottomUp(E *A, E *B, bIndexT *Tmp, bucketsT *BK,
			 intT numBK, intT n, intT bits, bool top, F f) {
      intT rounds = 1+(bits-1)/MAX_RADIX;
      intT rbits = 1+(bits-1)/rounds;
      intT bitOffset = 0;
      while (bitOffset < bits) {
	if (bitOffset+rbits > bits) rbits = bits-bitOffset;
	radixStep(A, B, Tmp, BK, numBK, n, 1 << rbits, top,
		  eBits<E,F>(rbits,bitOffset,f));
	bitOffset += rbits;
      }
  }

  // Radix sort with high order bits first
  template <class E, class F>
  void radixLoopTopDown(E *A, E *B, bIndexT *Tmp, bucketsT *BK,
			intT numBK, intT n, intT bits, F f) {
    if (n == 0) return;
    if (bits <= MAX_RADIX) {
      radixStep(A, B, Tmp, BK, numBK, n, 1 << bits, true, eBits<E,F>(bits,0,f));
    } else if (numBK >= BUCKETS+1) {
      radixStep(A, B, Tmp, BK, numBK, n, BUCKETS, true,
		eBits<E,F>(MAX_RADIX,bits-MAX_RADIX,f));
      intT* offsets = BK[0];
      intT remain = numBK - BUCKETS - 1;
      float y = remain / (float) n;
      parallel_for (intT i=0; i < BUCKETS; i++) {
	intT segOffset = offsets[i];
	intT segNextOffset = (i == BUCKETS-1) ? n : offsets[i+1];
	intT segLen = segNextOffset - segOffset;
	intT blocksOffset = ((intT) floor(segOffset * y)) + i + 1;
	intT blocksNextOffset = ((intT) floor(segNextOffset * y)) + i + 2;
	intT blockLen = blocksNextOffset - blocksOffset;
	radixLoopTopDown(A + segOffset, B + segOffset, Tmp + segOffset,
			 BK + blocksOffset, blockLen, segLen,
			 bits-MAX_RADIX, f);
      }
    } else {
      radixLoopBottomUp(A, B, Tmp, BK, numBK, n, bits, false, f);
    }
  }

  // Sorts the array A, which is of length n.
  // Function f maps each element into an integer in the range [0,m)
  // If bucketOffsets is not NULL then it should be an array of length m
  // The offset in A of each bucket i in [0,m) is placed in location i
  //   such that for i < m-1, offsets[i+1]-offsets[i] gives the number
  //   of keys=i.   For i = m-1, n-offsets[i] is the number.
  template <class E, class F>
  void iSort(E *A, intT* bucketOffsets, intT n, long m, bool bottomUp, F f) {
    intT bits = utils::logUpLong(m);
    E *B = newA(E,n);
    bIndexT *Tmp = newA(bIndexT,n);
    intT numBK = 1+n/(BUCKETS*8);
    bucketsT *BK = newA(bucketsT,numBK);
    if (bottomUp)
      radixLoopBottomUp(A, B, Tmp, BK, numBK, n, bits, true, f);
    else
      radixLoopTopDown(A, B, Tmp, BK, numBK, n, bits, f);
    if (bucketOffsets != NULL) {
      {parallel_for (intT i=0; i < m; i++) bucketOffsets[i] = n;}
      {parallel_for (intT i=0; i < n-1; i++) {
	  intT v = f(A[i]);
	  intT vn = f(A[i+1]);
	  if (v != vn) bucketOffsets[vn] = i+1;
	}}
      bucketOffsets[f(A[0])] = 0;
      sequence::scanIBack(bucketOffsets,bucketOffsets,(intT) m,utils::minF<intT>(),n);
    }
    free(B); free(Tmp); free(BK);
  }

  template <class E, class F>
  void iSort(E *A, intT* bucketOffsets, intT n, long m, F f) {
    iSort(A, bucketOffsets, n, m, false, f);}

  // A version that uses a NULL bucketOffset
  template <class E, class Func>
  void iSort(E *A, intT n, long m, Func f) {
    iSort(A, NULL, n, m, false, f);}

  template <class E, class F>
  void iSortBottomUp(E *A, intT n, long m, F f) {
    iSort(A, NULL, n, m, true, f);}
};

static void integerSort(intT *A, intT n) {
  long maxV = sequence::reduce(A,n,utils::maxF<intT>());
  intSort::iSort(A, NULL, n, maxV,  utils::identityF<intT>());
}

template <class T>
void integerSort(pair<uintT,T> *A, intT n) {
  long maxV = sequence::mapReduce<uintT>(A,n,utils::maxF<uintT>(),
					utils::firstF<uintT,T>());
  intSort::iSort(A, NULL, n, maxV,  utils::firstF<uintT,T>());
}

#endif


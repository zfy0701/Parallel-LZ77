// This code is part of the 15418 course project: Implementation and Comparison
// of Parallel LZ77 and LZ78 Algorithms and DCC 2013 paper: Practical Parallel
// Lempel-Ziv Factorization appearing in
// Copyright (c) 2012 Fuyao Zhao, Julian Shun
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

#include <cstdio>
#include <iostream>
#include "Base.h"
#include "sequence.h"
#include "parallel.h"

using namespace std;

//this module is share by others
//not test for n < 8

pair< pair<intT, intT>*, intT> ParallelLPFtoLZ(intT *lpf, intT *prev_occ, intT n) {
  intT l2 = cflog2(n);
  intT depth = l2 + 1;
  int nn = 1 << l2;
  intT *pointers = new intT[n];

  intT *flag = new intT[n + 1];

  parallel_for (intT i = 0; i < n; i++) {
    flag[i] = 0;
    pointers[i] = min(n, i + max<intT>(lpf[i], 1));
  }
  flag[n] = 0;

  l2 = max<intT>(l2, 256);
  intT sn = (n + l2 - 1) / l2;

  intT *next = new intT[sn + 1], *next2 = new intT[sn + 1];
  intT *sflag = new intT[sn + 1];

  //build the sub tree
  parallel_for (intT i = 0; i < sn; i ++) {
    intT j;
    for (j = pointers[i * l2]; j % l2 && j != n; j = pointers[j]) ;
    if (j == n) next[i] = sn;
    else next[i] = j / l2;
    sflag[i] = 0;
  }

  next[sn] = next2[sn] = sn;
  sflag[0] = 1; sflag[sn] = 0;

  //point jump
  intT dep = getDepth(sn); ;
  for (intT d = 0; d < dep; d++) {
    parallel_for(intT i = 0; i < sn; i ++) {
      intT j = next[i];
      if (sflag[i] == 1) {
        sflag[j] = 1;
      }
      next2[i] = next[j];
    }
    std::swap(next, next2);
  }

  //filling the result
  parallel_for (intT i = 0; i < n; i += l2) {
    if (sflag[i / l2]) {
      flag[i] = 1;
      for (intT j = pointers[i]; j % l2 && j != n; j = pointers[j]) {
        flag[j] = 1;
      }
    }
  }
  delete sflag; delete next; delete next2; delete pointers;

  sequence::scan(flag, flag, n + 1, utils::addF<intT>(), (intT)0);

  intT m = flag[n];
  pair<intT, intT> *lz = new pair<intT, intT>[m];

  parallel_for(intT i = 0; i < n; i++) {
    if (flag[i] < flag[i + 1]) {
      lz[flag[i]] = make_pair(i, prev_occ[i]);
    }
  }
  delete flag;

  return make_pair(lz, m);
}

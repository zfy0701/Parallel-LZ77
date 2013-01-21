// This code is part of the 15418 course project: Implementation and Comparison
// of Parallel LZ77 and LZ78 Algorithms and DCC 2013 paper: Practical Parallel
// Lempel-Ziv Factorization
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

/*
 * The sequential algorithm for Lempel-ziv 77 compression
 */

#include <iostream>
#include <cstdio>
#include <cstring>
#include "test.h"
#include "ANSV.h"
#include "suffixArray.h"
using namespace std;

pair< pair<intT, intT>*, intT> LempelZiv(intT *s, intT n) {
  timer lzTm;
  lzTm.start();

  intT i, lpf, l, r;
  intT *leftElements = new intT[n], * rightElements = new intT[n];

  pair<intT *, intT *> res = suffixArray(s, n, 0);

  intT *SA = res.first;
  lzTm.reportNext("\tsuffix array");

  ComputeANSV_Linear(SA, n, leftElements, rightElements);

  lzTm.reportNext("\tANSV");

  intT *Rank = new intT[n];
  for (i = 0; i < n; i++) {
    Rank[SA[i]] = i;
  }

  intT k = 0;
  pair<intT, intT> *LZ = new pair<intT, intT>[n];

  for (k = i = 0; i < n; i += max<intT>(1, lpf)) {
    intT left = leftElements[Rank[i]], right = rightElements[Rank[i]];

    l = r = 0;

    if (left != -1) while (s[SA[left] + l] == s[i + l])
        l++;
    if (right != -1) while (s[SA[right] + r] == s[i + r])
        r++;

    LZ[k].first = i;
    if (l == 0 && 0 == r) {
      LZ[k].second = -1;
    } else if (l > r) {
      LZ[k].second = SA[left];
    } else {
      LZ[k].second = SA[right];
    }
    lpf = max<intT>(l, r);
    k++;
  }

  delete SA;
  delete leftElements;
  delete rightElements;
  delete Rank;
  lzTm.reportNext("\tlpf");
  return make_pair(LZ, k);
}

int parallel_main(int argc, char *argv[]) {
  return test_main(argc, argv, (char *)"Seq LZ77 with ANSV", LempelZiv);
}

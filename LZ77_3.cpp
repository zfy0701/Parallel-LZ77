// This code is part of the DCC 2013 paper: Practical Parallel Lempel-Ziv
// Factorization
// Copyright (c) 2012 Julian Shun, Fuyao Zhao
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
 * Sequential algorithm from CPM 2011 paper
 */

//Note: not working yet...

#include <iostream>
#include <cstdio>
#include <cstring>
#include "Base.h"
#include "test.h"
#include "suffixArray.h"

using namespace std;

void sop(intT i, intT l, intT j, intT *lps, intT *prev_occ, intT bot) {
  if ( j == 0 and l == 0 and i == 0 )
    return;
  if ( lps[i] == bot ) {
    lps[i] = l;
    prev_occ[i] = j;
  } else {
    if ( lps[i] < l ) {
      if ( prev_occ[i] > j )
        sop(prev_occ[i], lps[i], j, lps, prev_occ, bot);
      else
        sop(j, lps[i], prev_occ[i], lps, prev_occ, bot);
      lps[i] = l;
      prev_occ[i] = j;
    } else {
      if ( prev_occ[i] > j )
        sop(prev_occ[i], l, j, lps, prev_occ, bot);
      else
        sop(j, l, prev_occ[i], lps, prev_occ, bot);
    }
  }
}

pair<pair<intT, intT>*, intT> LempelZiv(intT *s, intT n) {
  timer lzTm;
  lzTm.start();
  //n--;
  pair<intT *, intT *> res = suffixArray(s, n, 0);

  intT *sa = res.first;
  lzTm.reportNext("\tsuffix array");

  //for(int i=0;i<n;i++) cout<<sa[i]<<" ";cout<<endl;
  //cout<<"n = "<<n<<endl;
  intT *phi = new intT [n];
  intT *prev_occ = new intT [n];
  intT *lps = new intT[n];
  for (intT i = 0; i < n; ++i)
    prev_occ[i] = s[i];
  intT to_add[2] = { -1, n - 1};
  for (intT i = 0; i < n; ++i) {
    phi[sa[i]] = sa[i + to_add[i == 0]];
  }
  free(sa);
  // sa holds now LPS
  for (intT i = 0; i < n; ++i)
    lps[i] = -1;

  intT l = 0;
  for (intT i = 0; i < n; ++i) {
    intT j = phi[i];
    while ( s[i + l] == s[j + l] ) ++l;

    if ( i > j ) {
      sop(i, l, j, lps, prev_occ, -1);
    } else {
      sop(j, l, i, lps, prev_occ, -1);
    }
    if ( l > 0 ) --l;
  }

  lzTm.reportNext("\tLPF");

  delete phi;

  //compute LZ array
  pair<intT, intT> *LZ = new pair<intT, intT>[n];

  //Comment: prev_occ array is incorrect, wtf?
  LZ[0].first = 0; LZ[0].second = -1;
  intT j = 0;
  while (LZ[j].first < n) {
    LZ[j + 1].first = LZ[j].first + max<intT>(1, lps[LZ[j].first]);
    LZ[j + 1].second = (lps[LZ[j].first] == 0) ? -1 : prev_occ[LZ[j + 1].first];
    j++;
  }

  lzTm.reportNext("\tLZ");
  delete lps; delete prev_occ;
  return make_pair(LZ, j);
}

int parallel_main(int argc, char *argv[]) {
  return test_main(argc, argv, (char *)"Seq LZ77 CPM2011", LempelZiv);
}

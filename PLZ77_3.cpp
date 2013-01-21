// This code is part of the DCC 2013 paper: Practical Parallel Lempel-Ziv
// Factorization
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
 * Parallel suffix tree + sequential searching
 * Current version doesn't work. We need to compute
 * minimum index at each internal node of suffix
 * tree to know when to stop searching
 */

#include <cstdio>
#include <iostream>

#include "suffixTree.h"
#include "sequence.h"
#include "Base.h"
#include "test.h"
#include "utils.h"

using namespace std;

// TODO: this module should be update to the new interface.
pair<int *, int> ParallelLPFtoLZ(int *lpf, int n);

pair<int *, int> ParallelLZ77(int *s, int n) {
  startTime();
  suffixTree st = buildSuffixTree(s, n);
  nextTime("\tSuffix Tree");
  //references
  stNode<int> *nodes = st.nodes;
  int root = st.root;

  int *minLabel = new int[st.m];
  parallel_for (int i = n; i < st.m; i++) {
    minLabel[i] = n;
  }

  //first round rake, only for children
  parallel_for (int i = 0; i < n; i++) {
    minLabel[i] = nodes[i].locationInOriginalArray;
    int pid = nodes[i].parentID;
    utils::writeMin(minLabel + pid, minLabel[i]);
  }

  bool changed = true;
  while (changed) {
    changed = false;
    parallel_for (int i = n; i < st.m; i++) {
      int pid = nodes[i].parentID;
      if (utils::writeMin(minLabel + pid, minLabel[i]))
        changed = true; //fix this -- don't write to global in parallel
    }
  }
  nextTime("\tTree Contraction");

  int dep = getDepth(st.m);
  int **up = new int*[dep];
  int **minup = new int*[dep];

  for (int i = 0; i < dep; i++) {
    up[i] = new int[st.m];  //this should be computed for all nodes
    minup[i] = new int[st.m];
  }

  //compute up
  parallel_for (int i = 0; i < st.m; i++) {
    up[0][i] = nodes[i].parentID;
  }
  for (int d = 1; d < dep; d++) {
    parallel_for (int i = 0; i < st.m; i++) {
      up[d][i] = up[d - 1][up[d - 1][i]];
    }
  }

  //compute minup
  parallel_for (int i = 0; i < st.m; i++) {
    minup[0][i] = min(minLabel[i], minLabel[up[0][i]]);
  }
  for (int d = 1; d < dep; d++) {
    parallel_for (int i = 0; i < st.m; i++) {
      minup[d][i] = min(minup[d - 1][i], minup[d - 1][up[d - 1][i]]);
    }
  }

  nextTime("\tget minup and up");

  //compute lpf by binary search
  int *lpf = new int[n];

  parallel_for (int i = 0; i < n; i++) {
    int cur = nodes[i].parentID;
    int pos = minLabel[cur];;

    int d = dep - 1;
    while (d > 0 && cur != root) {
      if (minup[d][cur] < minLabel[i]) {
        d--;    //scala down the scope
        if (minup[d][cur] == minLabel[i])
          cur = up[d][cur];
      } else {
        break;
      }
    }

    //adjusting
    if (minLabel[cur] == minLabel[i] && cur != root) {
      cur = nodes[cur].parentID;
    }
    pos = minLabel[cur];

    int len = nodes[cur].depth - 1; //nodes depth -1 = the length of the string path

    //fix for the un normal structure
    if (s[pos + len] == s[minLabel[i] + len] && pos + len < n && minLabel[i] + len < n) len++;

    lpf[minLabel[i]] = len;
  }
  lpf[0] = 0;

  nextTime("\tget lpf");

  delete minLabel;
  delete up;
  delete minup;
  st.del();
  pair<int *, int> r = ParallelLPFtoLZ(lpf, n);
  nextTime("\tget lz");

  delete lpf;
  return r;
}

int parallel_main(int argc, char *argv[]) {
  return test_main(argc, argv, (char *)"LZ77 using suffix tree (nlog n)", ParallelLZ77);
}


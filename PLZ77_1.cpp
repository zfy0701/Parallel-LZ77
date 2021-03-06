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
 * The parallel algirthm for Lempel-ziv 77 compression
 */

#include <cstdio>
#include <iostream>

#include "ANSV.h"
#include "suffixArray.h"
#include "rangeMin.h"
#include "sequence.h"
#include "Base.h"
#include "segmentTree.h"
#include "test.h"
#include "parallel.h"

pair< pair<intT, intT> *, intT> ParallelLPFtoLZ(intT *lpf, intT *prev_occ, intT n);

void getLPF_0(intT *s, intT *sa, intT n, intT *lcp, intT *lpf, intT *prev_occ) {
  intT d = getDepth(n);
  intT *l = new intT[n], *r = new intT[n];

  ComputeANSV(sa, n, l, r);
  nextTime("\tansv");

  myRMQ rmq(lcp, n);

  parallel_for (intT i = 0; i < n; i++) {
    intT llcp = 0, rlcp = 0;
    intT ln = l[i], rn = r[i];
    intT sai = sa[i];
    if (ln != -1) {
      llcp = lcp[rmq.query(ln + 1, i)];

    }
    if (rn != -1) {
      rlcp = lcp[rmq.query(i + 1, rn)];
    }

    if (llcp == 0 && rlcp == 0) {
      prev_occ[sai] = -1;
      lpf[sai] = 1;
    }
    // no neighbor
    else if (llcp > rlcp) {
      prev_occ[sai] = sa[ln];
      lpf[sai] = llcp;
    } else {
      prev_occ[sai] = sa[rn];
      lpf[sai] = rlcp;
    }
  }
  nextTime("\tlpf");

  delete l; delete r;
}

void getLPF_1(intT *s, intT *sa, intT n, intT *lcp, intT *lpf, intT *prev_occ) {
  intT d = getDepth(n);
  intT *leftElements = new intT[n], *rightElements = new intT[n];

  intT *leftLPF = new intT[n], *rightLPF = new intT[n];
  intT *rank = lpf; //reuse the space

  ComputeANSV(sa, n, leftElements, rightElements);
  nextTime("\tansv");

  SegmentTree st;
  st.BuildTree(lcp, n);
  nextTime("\tbuild tree");

  parallel_for (intT i = 0; i < n; i++) {
    rank[sa[i]] = i;
  }

  int size = 8196;

  //compute lpf for first element
  parallel_for (intT i = 0; i < n; i += size) {
    //int j = min(i + size, n);
    intT mid = rank[i], left = leftElements[rank[i]], right = rightElements[rank[i]];
    if (left != -1) {
      leftLPF[i] = st.Query(left + 1, mid);
    } else leftLPF[i] = 0;

    if (right != -1) {
      rightLPF[i] = st.Query(mid + 1, right);
    } else rightLPF[i] = 0;

    if (leftLPF[i] == 0 && rightLPF[i] == 0) {
      prev_occ[i] = -1;
      lpf[i] = 1;
    }
    // no neighbor
    else if (leftLPF[i] > rightLPF[i]) {
      prev_occ[i] = sa[left]; lpf[i] = leftLPF[i];
    } else {
      prev_occ[i] = sa[right];
      lpf[i] = rightLPF[i];
    }

  }
  st.DeleteTree();

  //compute lpf for rest elements
  parallel_for (intT i = 0; i < n; i += size) {
    intT j = min(i + size, n);
    for (intT k = i + 1; k < j; k++) {
      intT left = leftElements[rank[k]];
      intT right = rightElements[rank[k]];

      if (left != -1) {
        intT llcp = max<intT>(leftLPF[k - 1] - 1, 0);
        while (s[sa[left] + llcp] == s[k + llcp]) llcp++;
        leftLPF[k] = llcp;
      } else leftLPF[k] = 0;

      if (right != -1) {
        intT rlcp = max<intT>(rightLPF[k - 1] - 1, 0);
        while (s[sa[right] + rlcp] == s[k + rlcp]) rlcp++;
        rightLPF[k] = rlcp;
      } else rightLPF[k] = 0;


      if (leftLPF[k] == 0 && rightLPF[k] == 0) {
        prev_occ[k] = -1;
        lpf[k] = 1;
      }
      // no neighbor
      else if (leftLPF[k] > rightLPF[k]) {
        prev_occ[k] = sa[left]; lpf[k] = leftLPF[k];
      } else {
        prev_occ[k] = sa[right];
        lpf[k] = rightLPF[k];
      }
    }
  }

  nextTime("\tlpf");

  delete leftElements; delete rightElements;
  delete leftLPF; delete rightLPF;
}

void getLPF_2(intT *s, intT *sa, intT n, intT *lcp, intT *lpf, intT *prev_occ) {
  intT d = getDepth(n);
  intT *leftElements = new intT[n], *rightElements = new intT[n];

  intT *leftLPF = new intT[n], *rightLPF = new intT[n];
  intT *rank = lpf;

  ComputeANSV(sa, n, leftElements, rightElements);
  nextTime("\tansv");

  parallel_for (intT i = 0; i < n; i++) {
    rank[sa[i]] = i;
  }

  int p = get_threads();

  p *= 2;
  intT size = (n + p - 1) / p;

  parallel_for (intT i = 0; i < n; i += size) {
    intT j = min(i + size, n);

    //compute lpf for first element
    intT mid = rank[i], left = leftElements[rank[i]], right = rightElements[rank[i]];
    intT llcp = 0, rlcp = 0;

    if (left != -1) {
      while (s[sa[left] + llcp] == s[i + llcp]) llcp++;
      leftLPF[i] = llcp;
    } else leftLPF[i] = 0;

    if (right != -1) {
      while (s[sa[right] + rlcp] == s[i + rlcp]) rlcp++;
      rightLPF[i] = rlcp;
    } else rightLPF[i] = 0;

    if (leftLPF[i] == 0 && rightLPF[i] == 0) {
      prev_occ[i] = -1;
      lpf[i] = 1;
    }
    // no neighbor
    else if (leftLPF[i] > rightLPF[i]) {
      prev_occ[i] = sa[left];
      lpf[i] = leftLPF[i];
    } else {
      prev_occ[i] = sa[right];
      lpf[i] = rightLPF[i];
    }

    //compute lpf for rest elements
    for (intT k = i + 1; k < j; k++) {
      left = leftElements[rank[k]];
      right = rightElements[rank[k]];

      if (left != -1) {
        llcp = max<intT>(leftLPF[k - 1] - 1, 0);
        while (s[sa[left] + llcp] == s[k + llcp]) llcp++;
        leftLPF[k] = llcp;
      } else leftLPF[k] = 0;

      if (right != -1) {
        rlcp = max<intT>(rightLPF[k - 1] - 1, 0);
        while (s[sa[right] + rlcp] == s[k + rlcp]) rlcp++;
        rightLPF[k] = rlcp;
      } else rightLPF[k] = 0;

      if (leftLPF[k] == 0 && rightLPF[k] == 0) {
        prev_occ[k] = -1;
        lpf[k] = 1;
      }
      // no neighbor
      else if (leftLPF[k] > rightLPF[k]) {
        prev_occ[k] = sa[left]; lpf[k] = leftLPF[k];
      } else {
        prev_occ[k] = sa[right];
        lpf[k] = rightLPF[k];
      }

    }
  }

  nextTime("\tlpf");

  delete leftElements; delete rightElements;
  delete leftLPF; delete rightLPF;
}

int flag = 0;

pair<pair<intT, intT>*, intT> ParallelLZ77(intT *s, intT n) {
  startTime();

  pair<intT *, intT *> salcp = suffixArray(s, n, flag < 2 ? true : false);
  nextTime("\tsuffix array");

  intT *sa = salcp.first;
  intT *lcp = new intT[n];
  if (flag < 2) {
    lcp[0] = 0;
    parallel_for (intT i = 1; i < n; i++)
    lcp[i] = salcp.second[i - 1];
  }
  intT *prev_occ = new intT[n];
  intT *lpf = new intT[n];
  if (flag == 0)
    getLPF_0(s, sa, n, lcp, lpf, prev_occ);
  else if (flag == 1)
    getLPF_1(s, sa, n, lcp, lpf, prev_occ);
  else
    getLPF_2(s, sa, n, lcp, lpf, prev_occ);

  delete salcp.first;
  delete salcp.second;

  pair< pair<intT, intT>*, intT> r = ParallelLPFtoLZ(lpf, prev_occ, n);
  nextTime("\tlpf to lz");
  delete lpf; delete prev_occ;
  return r;
}

int parallel_main(int argc, char *argv[]) {
  int opt;
  while ((opt = getopt(argc, argv, "p:d:r:i:o:f:")) != -1) {
    if (opt == 'f') {
      flag = atoi(optarg);
      break;
    }
  }

  optind = 1;

  return test_main(argc, argv, (char *)"Parallel LZ77 using suffix array", ParallelLZ77);
}


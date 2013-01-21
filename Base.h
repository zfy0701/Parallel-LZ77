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

#ifndef _BASE_H_
#define _BASE_H_

#include <cmath>
#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "parallel.h"

int test_main(int argc, char *argv[], char *algoname, std::pair<intT *, intT> lz77(intT *s, intT n));

const int BSIZE = 16;

inline intT getDepth(intT i) {
  intT a = 0;
  intT b = i - 1;
  while (b > 0) {
    b = b >> 1;
    a++;
  }
  return a + 1;
}

inline intT fflog2(intT i) {
  intT res = -1;
  if (i >> 16) {
    res += 16;
    i >>= 16;
  } else i &= 0xffff;

  if (i >> 8) {
    res += 8;
    i >>= 8;
  } else i &= 0xff;

  if (i >> 4) {
    res += 4;
    i >>= 4;
  } else i &= 0xf;

  for (; i; i >>= 1) res++;
  return res;
}

inline intT cflog2(intT i) {
  intT res = 0;

  i--;
  if (i >> 16) {
    res += 16;
    i >>= 16;
  } else i &= 0xffff;

  for (; i; i >>= 1) res++;
  return res;
}

inline char *itoa(intT val, int base = 10) {
  static char buf[32] = {0};
  int i = 30;
  for (; val && i ; --i, val /= base)
    buf[i] = "0123456789abcdef"[val % base];
  return &buf[i + 1];
}

#endif

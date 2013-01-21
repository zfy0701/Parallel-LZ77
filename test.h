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

#ifndef _TEST_H
#define _TEST_H

#include "Base.h"
#include "parallel.h"
#include "utils.h"
#include "gettime.h"
#include "stringGen.h"

inline intT get_file_size(char *path) {
  struct stat info;
  stat(path, &info);
  return info.st_size;
}


inline void generateText(intT *a, intT n, int sigma)  {
  srand(time(NULL));
  for (intT i = 0; i < n; i++)
    a[i] = rand() % sigma + 1;
}

inline void Usage(char *program) {
  printf("Usage: %s [options]\n", program);
  printf("-p <num>\tNumber of processors to use\n");
  printf("-d <num>\t2^n of character will be processed\n");
  printf("-r <num>\tGenerete random string with the specified alphabet size\n");
  printf("-i <file>\tInput file name\n");
  printf("-o <file>\tOutput file name\n");
  printf("-f <file>\tChoose different algorithm for LPF\n");
  printf("-h \t\tDisplay this help\n");
}

inline int test_main(int argc, char *argv[], char *algoname, std::pair< std::pair<intT, intT>*, intT> lz77(intT *s, intT n)) {
  int opt;
  int p = 1, d = -1; intT n = -1;
  intT sigma = -1;
  char path[1025] = {};
  //int t = 0;

  while ((opt = getopt(argc, argv, "p:d:r:i:o:f:t:")) != -1) {
    switch (opt) {
    case 'p': {
      p = atoi(optarg);
      break;
    }
    case 'd': {
      d = atoi(optarg);
      n = 1 << d;
      break;
    }
    case 'r': {
      sigma = atoi(optarg);
      break;
    }
    case 'i': {
      strncpy(path, optarg, 1024);
      if (dup2(open(optarg, O_RDONLY), STDIN_FILENO) < 0) {
        perror("Input file error");
        exit(EXIT_FAILURE);
      }
      break;
    }
    case 'o': {
      if (dup2(open(optarg, O_CREAT | O_WRONLY, 0644), STDOUT_FILENO) < 0) {
        perror("Output file error");
        exit(EXIT_FAILURE);
      } else {
        //TODO
      }
      break;
    }
    case 'f': { //DON'T DO ANYTHING HERE!
      break;
    }

    default: {
      Usage(argv[0]);
      exit(1);
    }
    }
  }

  if (sigma < 0 && path[0] == 0) {
    perror("No input file specified / Random string genereted.");
    exit(1);
  }

  if (d < 0) {
    if (sigma < 0) {
      n = get_file_size(path);
    } else {
      perror("Random data size not specified");
      exit(1);
    }
  }

  set_threads(p);
  printf("***************** TEST BEGIN *****************\n");

  intT *s = newA(intT, n + 3);

  if (sigma >= 1) {
    cout << " * Data generated randomly with alphabet size: " << sigma << endl;
    generateText(s, n, sigma);
  } else {
    printf(" * Data from file: %s\n", path);
    //int size =  get_file_size(path);

    seq<char> str = dataGen::readCharFile(path);
    intT size = str.size();

    if (n > size) {
      perror("The file is not as large as the size specified.");
      exit(1);
    }

    for (intT i = 0; i < n; i++) s[i] = (unsigned char) str[i];
    str.del();
  }

  cout << " * Data size: " << n << endl;
  printf(" * Algorithm: %s\n", algoname);
  printf(" * Threads num: %d\n", p);

  timer testTm;
  s[n] = s[n + 1] = s[n + 2] = 0;
  testTm.start();

  std::pair< std::pair<intT, intT>*, intT> res = lz77(s, n);
  intT maxoffset = n - res.first[res.second - 1].first;
  for (intT i = 0; i < res.second - 1; i++) {
    maxoffset = std::max(maxoffset, res.first[i + 1].first - res.first[i].first);
  }

  cout << " * result: size = " << res.second << ", max offset = " << maxoffset << endl;
  testTm.reportNext(" * Total time:");
  printf("***************** TEST ENDED *****************\n\n");
  free(res.first);
  free(s);
  return 0;
}

#endif

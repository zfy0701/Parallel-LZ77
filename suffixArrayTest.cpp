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

#include <iostream>
#include <cstdlib>
#include "gettime.h"
#include "sequence.h"
#include "stringGen.h"
#include <string.h>
using namespace std;

int* suffixArrayNoLCP(int* s, int n);

bool isPermutation(int *SA, int n) {
  bool *seen = new bool[n];
  for (int i = 0;  i < n;  i++) seen[i] = 0;
  for (int i = 0;  i < n;  i++) seen[SA[i]] = 1;
  for (int i = 0;  i < n;  i++) if (!seen[i]) return 0;
  return 1;
}

bool sleq(int *s1, int *s2) {
  if (s1[0] < s2[0]) return 1;
  if (s1[0] > s2[0]) return 0;
  return sleq(s1+1, s2+1);
} 

bool isSorted(int *SA, int *s, int n) {
  for (int i = 0;  i < n-1;  i++) {
    if (!sleq(s+SA[i], s+SA[i+1])) {
      cout << "not sorted at i = " << i+1 << " : " << SA[i] 
	   << "," << SA[i+1] << endl;
      return 0;
    }
  }
  return 1;  
}

int cilk_main(int argc, char **argv) {
  if (argc < 2 || argc > 3){
    cout<<"Usage: ./suffixArrayTest <filename>\n";
    cout<<"Give a second argument of \"-c\" to suffixArrayTest to check for correctness\n";
  }
  else {
    char* filename = (char*)argv[1];
    seq<char> str = dataGen::readCharFile(filename);
    int n = str.size();
    int* s = newA(int,n+1);
    for (int i = 0; i < n; i++) s[i] = (unsigned char) str[i];
    s[n] = 0;
    str.del();
    startTime();
    int* SA = suffixArrayNoLCP(s, n);
    stopTime(1,"Suffix Array (from file)");
    if(argc == 3 && !strcmp(argv[2],"-c")) {
      if(isPermutation(SA,n) && isSorted(SA,s,n)) 
	cout<<"Correctness check passed\n";
      else
	cout<<"Correctness check failed\n";
    }
    free(s); free(SA);
    return 0;
  }


}

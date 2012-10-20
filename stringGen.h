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

#ifndef _STRINGGEN_INCLUDED
#define _STRINGGEN_INCLUDED

#include <iostream>
#include <fstream>
#include "parallel.h"
#include "seq.h"
#include "itemGen.h"
#include <math.h>

#define TRIGRAM_FILE "trigrams.txt"

namespace dataGen {
using namespace std;

seq<char> readCharFile(const char* fname) {
  ifstream file (fname, ios::in|ios::binary|ios::ate);
  intT n;
  char *Str;
  utils::myAssert(file.is_open(),"Unable to open file");
  n = file.tellg();
  Str = newA(char,n+2);
  file.seekg (0, ios::beg);
  file.read (Str,n);
  file.close();
  //cout << "read size = " << n << endl;
  return seq<char>(Str,n);
}

pair<intT*,intT> readIntFile(const char* fname) {
  ifstream file (fname, ios::in|ios::binary|ios::ate);  
  intT nchar;
  intT n = 0;
  intT number;
  utils::myAssert(file.is_open(),"Unable to open file");
  nchar = file.tellg();
  intT* arr = newA(intT,nchar+2);
  file.seekg (0, ios::beg);
  while(file >> number) arr[n++] = number;
  file.close();
  intT* arr2 = newA(intT,n+2);
  for(intT i = 0;i < n;i++) arr2[i] = arr[i];
  free(arr);
  return make_pair(arr2,n);
}

struct tableEntry {
  char str[10];
  intT len;
  char chars[27];
  float probs[27];
};

struct nGramTable {
  intT len;
  tableEntry S[27][27];

  int index(char c) {
    if (c=='_') return 26;
    else return (c-'a');
  }

  nGramTable() {
    ifstream ifile(TRIGRAM_FILE);
    if (ifile.is_open()) {
      intT i=0;
      while (! ifile.eof()) {
	tableEntry x;
	ifile >> x.str >> x.len;
	float probSum = 0.0;
	for (intT j=0; j < x.len; j++) {
	  float prob;
	  ifile >> x.chars[j] >> prob;
	  probSum += prob;
	  if (j == x.len-1) x.probs[j] = 1.0;
	  else x.probs[j] = probSum;
	}
	intT i0 = index(x.str[0]);
	intT i1 = index(x.str[1]);
	if (i0 > 26 || i1 > 26) abort();
	S[i0][i1] = x;
	i++;
      }
      len = i;
    }
  }

  char next(char c0, char c1, intT i) {
    intT j=0;
    tableEntry E = S[index(c0)][index(c1)];
    double x = hash<double>(i);
    while (x > E.probs[j]) j++;
    return E.chars[j];
  }

  intT word(intT i, char* a, intT maxLen) {
    a[0] = next('_','_',i);
    a[1] = next('_',a[0],i+1);
    intT j = 1;
    while (a[j] != '_' && j < maxLen-1) {
      j++;
      a[j] = next(a[j-2],a[j-1],i+j);
    }
    a[j] = 0;
    return j+1;
  }

  intT wordLength(intT i, intT maxLen) {
    char a0 = next('_','_',i);
    char a1 = next('_',a0,i+1);
    intT j = 1;
    while (a1 != '_' && j < maxLen-1) {
      j++;
      char tmp = next(a0,a1,i+j);
      a0 = a1; a1 = tmp;
    }
    return j+1;
  }

  char* word(intT i) {
    intT MAX_LEN = 100;
    char a[MAX_LEN+1];
    intT l = word(i, a, MAX_LEN);
    char* out = newA(char,l);
    for(intT j=0; j < l; j++) out[j] = a[j];
    return out;
  }

  char* string(intT s, intT e) {
    intT n = e - s;
    char* a = newA(char,n+1);
    intT j=0;
    while (j < n) {
      intT l = word(j+s,a+j,n-j);
      a[j+l-1] = ' ';
      j += l;
    }
    a[n] = 0;
    return a;
  }
};

  char* trigramString(intT s, intT e) { 
    nGramTable T = nGramTable();
    return T.string(s, e);
  }

};

#endif // _STRINGGEN_INCLUDED

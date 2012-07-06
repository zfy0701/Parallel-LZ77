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

#ifndef A_HASH_INCLUDED
#define A_HASH_INCLUDED

#include <iostream>
#include <cstdlib>
#include "gettime.h"
#include "seq.h"
#include "utils.h"
using namespace std;

template <class ET, class CMP, class HASH>
class Table {
 private:
  int m;
  int mask;
  CMP cmp; 
  HASH hash;
  ET least;
  ET* TA;
  int* compactL;

  // needs to be in separate routine due to Cilk bugs
  static void clearA(ET* A, int n, ET v) {
    parallel_grainsize_256
    parallel_for (int i=0; i < n; i++) A[i] = v;
  }

  struct addF { int operator() (int a, int b) {return a+b;}};
  struct emptyF { 
    ET least; emptyF(ET ll) : least(ll) {} 
    int operator() (ET a) {return least != a;}};

  int hashToRange(int h) {return h & mask;}
  int firstHash(ET v) {return hashToRange(hash(v));}
  int incHash(int h) {return hashToRange(h+1);}

 public:
  // Size is the maximum number of values the hash table will hold.
  // Overfilling the table could put it into an infinite loop.
  Table(int size, CMP cmpF, HASH hashF, ET leastv) :
    m(utils::nextPower(2*size)), 
    mask(m-1),
    cmp(cmpF), 
    hash(hashF), 
    least(leastv),     
    TA(newA(ET,m)),
    compactL(NULL) 
      { clearA(TA,m,leastv); }

  // Deletes the allocated arrays
  void del() {
    free(TA); 
    if (compactL != NULL) free(compactL);
  }

  bool insert(ET v) {
    int h = firstHash(v);
    do {
      ET c;
      bool r = 0;
      do {
	c = TA[h];
	int b = (c==least) ? 1 : cmp(v,c);
	if (!b) return 1;
	while (b==1 && !(r=utils::CAS(&TA[h],c,v))) {
	  if (!(b=cmp(v,c = TA[h]))) return 1;
	}
	h = incHash(h);
      } while (!r);
      v = c;
    } while (least < v);
    return 0;
  }

  // Returns the value if an equal value is found in the table
  // otherwise returns the "least" element.
  ET find(ET v) {
    int h = firstHash(v);
    ET c = TA[h]; 
    int b = 1;
    while (c != least && (1==(b=cmp(c,v)))) {
      h = incHash(h);
      c = TA[h];
    }
    return (!b) ? c : least;
  }

  template <class SCMP>
  ET findF(int hashVal, SCMP scmp) {
    int h = hashToRange(hashVal);
    ET c = TA[h]; 
    int b = 1;
    while (c != least && (1==(b=scmp(c)))) {
      h = incHash(h);
      c = TA[h];
    }
    return (!b) ? c : least;
  }

  // If called after entries have been added gives each entry a label
  // in the range [0..n], where n is the number of entries.
  int compactLabels() {
    compactL = newA(int,m);
    parallel_for (int i=0; i < m; i++) compactL[i] = (least <TA[i]);
    int r = sequence::scan(compactL,compactL,m,addF(),0);
    return r;
  }

  // returns a unique integer name in the range [0...m-1] for each
  // distinct element, or -1 if not found.
  // If compactLabels has been run, then returns the labels in
  // range [0..l], where l is the number of entries in the table
  int findLabel(ET v) {
    int h = firstHash(v);
    ET c = TA[h]; 
    int b = 1;
    while (c != least && (1==(b=cmp(c,v)))) c = TA[h = incHash(h)];
    if (compactL == NULL) return (!b) ? h : -1;
    else return (!b) ? compactL[h] : -1;
  }

  // returns the number of entries
  int count() {
    int *A = newA(int,m);
    parallel_grainsize_256
    parallel_for (int i=0; i < m; i++) A[i] = (least <TA[i]);
    int r = sequence::reduce(A,0,m,addF());
    free(A);
    return r;
  }

  // returns all the current entries compacted into a sequence
  seq<ET> entries() {
    seq<ET> R = seq<ET>(TA,m).filter(emptyF(least));
    return R;
  }

  // prints the current entries along with the index they are stored at
  void print() {
    cout << "vals = ";
    for (int i=0; i < m; i++) 
      if (TA[i]!=least) cout << i << ":" << TA[i] << ",";
    cout << endl;
  }
};

// returns a unique name for each distinct element
// second return value is the number of unique values
template <class ET, class CMP, class HASH>
pair<int*,int> name(seq<ET> A, int m, CMP cmpF, HASH hashF, ET least) {
  Table<ET,CMP,HASH> T(m,cmpF,hashF,least);
  {parallel_for(int i=0;i<A.size();i++) T.insert(A[i]);}
  int n = T.compactLabels();
  int *R = newA(int,A.size());
  {parallel_for(int i=0;i<A.size();i++) R[i] = T.findLabel(A[i]);}
  T.del(); 
  return pair<int*,int>(R,n);
}

template <class ET, class CMP, class HASH>
seq<ET> removeDuplicates(seq<ET> A, int m, CMP cmpF, HASH hashF, ET least) {
  //startTime();
  Table<ET,CMP,HASH> T(m,cmpF,hashF,least);
  //nextTime("make table");
  parallel_grainsize_256
  parallel_for(int i=0;i<A.size();i++) T.insert(A[i]);
  //nextTime("insert");
  seq<ET> R = T.entries();
  //nextTime("entries");
  T.del(); 
  return R;
}

template <class ET, class CMP, class HASH>
seq<ET> removeDuplicates(seq<ET> A, CMP cmpF, HASH hashF, ET least) {
  return removeDuplicates(A, A.size(), cmpF,hashF,least);
}

struct intHash{unsigned int operator() (int a) {return utils::hash(a);}};
struct intCmp {
  int operator() (int a, int b) {
    return (a > b) ? 1 : ((a == b) ? 0 : -1);}
};

// works for non-negative integers (uses -1 to mark cell as empty)
static seq<int> removeDuplicates(seq<int> A) {
    return removeDuplicates(A,intCmp(),intHash(),-1);
}

static pair<int*,int> name(seq<int> A, int m) {
  return name(A,m, intCmp(),intHash(),-1);
}

typedef Table<int,intCmp,intHash> IntTable;

static IntTable makeIntTable(int m) {
  return IntTable(m,intCmp(),intHash(),-1);
}

struct strHash {
  unsigned int operator() (char* s) {
    unsigned int hash = 0;
    while (*s) hash = *s++ + (hash << 6) + (hash << 16) - hash;
    return hash;
  }
};

struct strCmp {
  int operator() (char* s1, char* s2) {
    while (*s1 && *s1==*s2) {s1++; s2++;};
    return (*s1 > *s2) ? 1 : ((*s1 == *s2) ? 0 : -1);
  }
};

static seq<char*> removeDuplicates(seq<char*> S) {
  return removeDuplicates(S,strCmp(),strHash(),(char*) NULL);}

static seq<char*> removeDuplicates(seq<char*> S, int m) {
  return removeDuplicates(S,m,strCmp(),strHash(),(char*) NULL);}

typedef Table<char*,strCmp,strHash> StrTable;

static StrTable makeStrTable(int m) {
  return StrTable(m,strCmp(),strHash(),(char*) NULL);
}

#endif // _A_HASH_INCLUDED

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

#ifndef __SUFFIX_UTILS__
#define __SUFFIX_UTILS__
#include "hash.h"

struct notNeg {
  bool operator () (int i) {
    return (i >= 0);
  }
};

template <class strElt>
struct stNode {
  int parentID;     //my feeling is that the parentID is not working for stNode
  strElt edgeFirstChar;
  int pointingTo;
  int locationInOriginalArray;
  int edgeLength;

  void setValues(int _parentID, strElt _edgeFirstChar,
                 int _pointingTo, int _location, int _edgeLength) {
    parentID = _parentID;
    edgeFirstChar = _edgeFirstChar;
    pointingTo = _pointingTo;
    locationInOriginalArray = _location;
    edgeLength = _edgeLength;
  }
};

struct stNodeHash {
  unsigned int operator() (stNode<int> *stn) {
    unsigned int seed = utils::hash(stn->parentID);
    seed ^= utils::hash(stn->edgeFirstChar) +
            0x9e3779b9 + (seed << 6) + (seed >> 2);
    return seed;
  }
};

struct stNodeCmp {
  int operator() (stNode<int> *a, stNode<int> *b) {
    if (a->parentID > b->parentID) return 1;
    else if (a->parentID < b->parentID) return -1;
    else return (a->edgeFirstChar > b->edgeFirstChar)
                  ? 1 : ((a->edgeFirstChar == b->edgeFirstChar) ? 0 : -1);
  }
};

typedef Table<stNode<int>*, stNodeCmp, stNodeHash> stNodeTable;

static stNodeTable makeStNodeTable(int m) {
  return stNodeTable(m, stNodeCmp(), stNodeHash(), (stNode<int> *) NULL);
}

//TODO: delay the stNodeTable computation
struct suffixTree {
  int n; // number of leaves
  int m; // total number of nodes (leaves and internal)
  int root;
  int *s;
  stNode<int> *nodes;
suffixTree(int _n, int _m, int *_s, stNode<int> *_nodes,int _root) :
  n(_n), m(_m), s(_s), nodes(_nodes), root(_root) {}
  void del() {
//    free(s);
    free(nodes);
  }

  bool isLeaf(int i) {
    return i < n;
  }
};

suffixTree suffixArrayToTree (int *SA, int *LCP, int n, int *s);
pair<int *, int *> suffixArray(int *s, int n, bool findLCPs);

inline suffixTree buildSuffixTree(int *s, int n) {
  pair<int *, int *> SA_LCP = suffixArray(s, n, true);
  suffixTree T = suffixArrayToTree(SA_LCP.first, SA_LCP.second, n, s);
  return T;
}

#endif

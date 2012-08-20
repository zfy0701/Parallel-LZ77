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
#include "gettime.h"
#include "intSort.h"
#include "sequence.h"
#include "utils.h"
#include "hash.h"
#include "suffixTree.h"
#include "cartesianTree.h"

using namespace std;
#define CHECK 0

inline int getRoot(node* nodes, int i) {
  int root = nodes[i].parent;
  while (root != 0 && nodes[nodes[root].parent].value == nodes[root].value)
    root = nodes[root].parent;
  return root;
}

inline void setNode(node *nodes, stNode<int>* stnodes, int *s,
		    int i, int j, int position) {
  int parent = nodes[j].parent;
  int parentDepth = nodes[parent].value;
  int offset = position + parentDepth;
  int length = nodes[j].value - parentDepth;
  stnodes[i].setValues(parent,s[offset],j,offset,length);
}

suffixTree suffixArrayToTree (int* SA, int* LCP, int n, int* s){
  //initialize nodes
  node* nodes = new node[2*n];
  parallel_for(int i=1; i<n; i++){ //internal
    nodes[2*i].value = LCP[i-1];
    nodes[2*i+1].value = n-SA[i]+1;
    nodes[2*i].parent = nodes[2*i+1].parent = 0;
  }
  nodes[0].value = 0;
  nodes[1].value = n-SA[0]+1;
  nodes[0].parent = nodes[1].parent = 0;
  delete [] SA;  delete [] LCP;

  cartesianTree(nodes,1,2*n-1);

  // Filter out non-root internal nodes
  int* flags = new int[n];
  flags[0] = -1;
  parallel_for(int i=1; i<n;i++){
    int j = 2*i;
    int p = nodes[j].parent;
    if (nodes[j].value > nodes[p].value) flags[i] = j;
    else flags[i] = -1;
  }  
  int* oout = new int[n];
  int nm = sequence::filter(flags,oout,n,notNeg());
  delete flags;

  int * newid = new int[2*n];

  // shortcut to roots of each cluster
  parallel_for(int i=1;i<2*n;i++) {
    nodes[i].parent = getRoot(nodes, i);
  }

  // copy leaves to hash structure
  stNode<int>* stnodes = new stNode<int>[n+nm+1];
  newid[0] = n+nm;
  stnodes[n+nm].parentID = n+nm;

  parallel_for(int i=0;i<n;i++){
    int j = 2*i+1;
    setNode(nodes,stnodes,s,i,j,(n-nodes[j].value+1));
    newid[j] = i;
  }

  // copy internal nodes to hash structure
  parallel_for(int i=0;i<nm;i++){
    int j = oout[i];
    newid[j] = n + i;
    setNode(nodes,stnodes,s,n+i,j,(n-nodes[j-1].value+1));
  }

  // for (int i = 0; i < n; i++) {
  //   printf("%d ", s[i]);
  // }
  // printf("end str\n");
  // for (int i = 0; i < n + nm; i++) {
  //   printf("%d ", newid[i]);
  // }
  // printf("end\n");

  parallel_for(int i = 0; i < n+nm; i++) {
    int pid = stnodes[i].parentID;
    stnodes[i].parentID = newid[stnodes[i].parentID];
    if ( stnodes[i].parentID == -1) {
      printf("@%d %d %d\t", i, pid, stnodes[i].parentID);
    }
  }
//    printf("\n");

  delete newid;
  delete oout;  delete nodes;

  // insert into hash table
  stNodeTable table = makeStNodeTable(3*(n+nm)/4);
  stNodeTable* ST = &table;
  parallel_for(int i=0; i<n+nm; i++) {
    ST->insert(stnodes+i);
  }

  return suffixTree(n,n+nm+1,table,s,stnodes);
}

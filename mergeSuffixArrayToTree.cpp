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

void printTree(stNode<int>* stnodes, int nnm){
  for(int i=0;i<nnm;i++)cout<<"("<<i<<","<<stnodes[i].parentID<<") ";
  cout<<endl;
}

//check
void verifyTree(stNode<int>* stnodes, int nnm, int n){
  //one root
  int numRoots = 0;
  for(int i=0;i<nnm;i++) if(stnodes[i].parentID == -1) numRoots++;
  if(numRoots != 1) {cout<<"screwed up\n";abort();}

  //everyone has a parent in tree
  for(int i=0;i<nnm;i++)
    if(stnodes[i].parentID < -1 || stnodes[i].parentID >= nnm){cout<<"screwed up again\n"; abort();}

  int* numChildren = new int[nnm]; 
  for(int i=0;i<nnm;i++) numChildren[i]=0;
  for(int i=0;i<nnm;i++) {if(stnodes[i].parentID != -1) {
      numChildren[stnodes[i].parentID]++;
      if(stnodes[i].parentID < n) cout<<i<<" "<<stnodes[i].parentID<<" can't point to child\n"; }
  }
  for(int i=n;i<nnm;i++) if(numChildren[i]==0) {cout<<"internal node"<< i<<" has no children\n";abort();} 
  cout<<"tree verified\n";
  delete [] numChildren;
}


inline int getRoot(node* nodes, int i, int realRoot) {
  int root = nodes[i].parent;
  while (root != realRoot && nodes[nodes[root].parent].value == nodes[root].value)
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

  nodes[1].value = n-SA[0]+1;
  nodes[1].parent = 0;
  delete [] SA;  delete [] LCP;

  cartesianTree(nodes,1,2*n-1);
  int realRoot = -1;

  parallel_for(int i=1;i<2*n;i++) 
    if(nodes[i].parent == 0) {
      realRoot = i;
      nodes[i].parent = i;
  }

  // shortcut to roots of each cluster
  parallel_for(int i=1;i<2*n;i++) {
    nodes[i].parent = getRoot(nodes, i, realRoot);
  }

  // Filter out non-root internal nodes
  int* flags = new int[n];
  flags[0] = -1;
  parallel_for(int i=1; i<n;i++){
    int j = 2*i;
    int p = nodes[j].parent;
    if (nodes[j].value > nodes[p].value) flags[i] = j;
    else flags[i] = -1;
  }

  nodes[realRoot].parent = -1;
  flags[realRoot/2] = realRoot; //keep the real root

  int* oout = new int[n];
  int nm = sequence::filter(flags,oout,n,notNeg());
  delete flags;
  int * newid = new int[2*n];


  // copy leaves to stnodes
  stNode<int>* stnodes = new stNode<int>[n+nm];
  //newid[0] = n+nm;
  //stnodes[n+nm].parentID = n+nm;
  for(int i=0;i<2*n;i++)newid[i]=-1000;
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
  cout<<"root = "<<realRoot<<", new id = "<<newid[realRoot]<<endl;
  cout<<"n+nm="<<n+nm<<endl;
  parallel_for(int i = 0; i < n+nm; i++) {
    int pid = stnodes[i].parentID;
    stnodes[i].parentID = (pid == -1) ? -1 : newid[pid];
    if(newid[pid] == -1000) {cout<<"screwed up "<< i<<" "<<pid<<endl;abort();}
    // if ( stnodes[i].parentID == -1) {
    //   printf("@%d %d %d\t", i, pid, stnodes[i].parentID);
    // }
  }

  realRoot = newid[realRoot];
  delete newid;
  delete oout;  delete nodes;
  verifyTree(stnodes,n+nm,n); //comment this out later 

  return suffixTree(n,n+nm,s,stnodes,realRoot);
}

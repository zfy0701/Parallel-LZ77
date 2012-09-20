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

using namespace std;

int prevMatchLength(suffixTree st, int currIndex){
  int* s = st.s;
  int matchLength = 0;
  stNode<int> currentNode;
  int position = 0;
  currentNode.parentID = 0;

  while (1) {
    currentNode.edgeFirstChar = s[currIndex+position];
    stNode<int>* b = st.table.find(&currentNode);
    if (b == NULL) break;
    else {
      cout<<currentNode.edgeFirstChar<<" found ";
      int length = b->edgeLength;
      int stLocation = b->locationInOriginalArray;
      cout<<"st location="<<stLocation<<" ";
      if(stLocation >= currIndex) break;
      matchLength++;
      // don't need to test first position since matched in hash table
      for (int i=1; i<length; i++) {
	if (s[currIndex+position+i] == 0 || 
	    (s[currIndex+position+i] != s[stLocation+i])){
	  break;
	}
	matchLength++;
      }
      if (s[currIndex+position+length] == 0) break;
      position += length;
      currentNode.parentID = b->pointingTo; 
    }
  }

  return matchLength;
}

pair<int *, int> ParallelLZ77(int *s, int n) {
  startTime();
  for(int i=0;i<=n;i++)cout<<s[i]<<" ";

  suffixTree st = buildSuffixTree(s, n);
  nextTime("\tSuffix Tree");
	
  int m = 1;
  int * LZ = new int[n]; LZ[0] = 0;
  int currIndex = 1;
  
  while(currIndex < n){
    cout<<currIndex<<" ";
    int matchLength = prevMatchLength(st,currIndex);
    cout<<matchLength<<endl;
    int dist = max(1,matchLength);
    LZ[m] = LZ[m-1] + dist;
    m++;
    currIndex += dist;
  }

  return make_pair(LZ, m);
}

int main(int argc, char *argv[]) {
    return test_main(argc, argv, (char *)"LZ77 using suffix tree + sequential searching", ParallelLZ77);
}


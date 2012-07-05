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
#include "stringGen.h"
#include "suffixTree.h"
#include <fstream>
#include <string.h>
#include "utils.h"
using namespace std;

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

//check if string appears in s
bool containsString(int* s, int* string, int n,int strLength){
  for(int i=0;i<= n-strLength;i++){
    for(int j=0;j<strLength;j++){
      if(string[j] == 0) return true;
      else if (string[j] != s[i+j]) break;
    }
  }
  return false;
}

int computeLCP(int *s1, int *s2, int matches){
  if(s1[0]==s2[0]) return computeLCP(s1+1,s2+1,matches+1);
  else return matches;
}

bool isLCPcorrect(int *SA, int *LCP, int *s, int n){
  for(int i=0;i<n-1;i++){
    //compute LCP
    if(computeLCP(s+SA[i],s+SA[i+1],0) != LCP[i]){
      cout << "LCP incorrect at i=" << i << " : " << 
	"LCP in in fact: " << computeLCP(s+SA[i],s+SA[i+1],0) <<
	" , but algorithm returns LCP = " << LCP[i] << endl;
      return 0;
    }
  }
  return 1;
}

int cilk_main(int argc, char **argv) {
  if(argc < 2 || argc > 5) {
    cout<<"Usage: ./suffixTreeTest <filename> [-s <search string>] [-c]\n";
    cout<<"To search for a string in the suffix tree pass the flag \"-s\" followed by the search string. To search for n/20 random substrings from the input file in the suffix tree, pass the flag \"-c\".\n";
  }
  else {
    char* filename = (char*)argv[1];
    seq<char> s = dataGen::readCharFile(filename);
    int n = s.size();
    int* str = newA(int,n+1);
    for (int i = 0; i < n; i++) str[i] = (char) s[i];
    str[n] = 0;
    s.del();

    startTime();
    pair<int*,int*> SA_LCP = suffixArray(str, n, true);
    int* SA = SA_LCP.first;
    int* LCP = SA_LCP.second;
    nextTime("Time for suffix array with input as a file");
    
    suffixTree T = suffixArrayToTree(SA,LCP,n,str);
    nextTime("Time for converting to suffix tree");

    char* searchString;
    if(searchString = utils::getOptionValue(argc,argv,(char*)"-s")){      
      int* string = new int[strlen(searchString)+1];
      string[strlen(searchString)] = 0;
      for(int i=0;i<strlen(searchString);i++) 
	string[i]=(unsigned char) searchString[i];
      int result = T.search(string);
      if(-1 == result) cout << "\"" << searchString << "\" not found\n";
      else cout << "\"" << searchString << "\" found at offset "<<result<<endl;
    }
    if(utils::getOption(argc,argv,(char*)"-c")){
      // generate n/STR_LEN random offsets and copy into strings
      int STR_LEN = 20;
      int m = n/STR_LEN;
      int* strings = new int[(STR_LEN+1)*m];
      int* offsets = new int[m];
      parallel_for (int i=0;i<m;i++) {
	int k = dataGen::hash<int>(i)%n;
	int j = 0;
	for (j=0; j<STR_LEN && k+j < n; j++) 
	  strings[(STR_LEN+1)*i+j] = str[k+j];
	strings[(STR_LEN+1)*i+j] = 0;
      }
      startTime();

      parallel_for (int i=0; i<m; i++) { 
	int* s = strings + (STR_LEN+1)*i;
	offsets[i] = T.search(s);
      }
      nextTime("Time to search n/20 strings");

      for (int i=0; i<m; i++) {
	int matches = 0;
	int* s = strings + (STR_LEN+1)*i;
	if (offsets[i] == -1) {
	  cout << "not found ith string : " << i << endl;
	  abort();
	} else {
	  int* s2 = str + offsets[i];
	  for (int j=0; s[j] != 0; j++)
	    if (s[j] != s2[j]) {
	      cout << "bad match at i=" << i << ": found location " << offsets[i]
		   << " does not match string at " 
		   << dataGen::hash<int>(i)%n << endl;
	      for (int k=0; k < STR_LEN; k++) cout << s[k] << ":";
	      cout << endl;
	      for (int k=0; k < STR_LEN; k++) cout << s2[k] << ":";
	      cout << endl;
	      abort();
	    }
	}
      }
    }
    T.del();
  }
}

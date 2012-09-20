//LZ77 implementation of "A simple algorithm for computing the
//Lempel-Ziv factorization", DCC 2008

#include <iostream>
#include <cstdio>
#include <cstring>
#include "test.h"
#include "suffixArray.h"
using namespace std;

pair<int*,int> compute(int* A, int n){
  timer lzTm;
  lzTm.start();

  A[n] = 128; //let last one be the max one, so SA[n] = n, and LCP[n] = 0
  pair<int*,int*> SA_LCP = suffixArray(A,n + 1,false);

  int* SA = SA_LCP.first;
  lzTm.reportNext("\tsuffix array time:");
  int *LCP = GetLCP(A, n + 1, SA);
  lzTm.reportNext("\tlcp time:");
  SA[n]=-1;
  A[n] = '\0';
  
  int* LPF = newA(int,n);
  int top = 0;
  int* stack = newA(int,n);
  stack[0] = 0;
  for(int i=1;i<=n;i++){ //compute LPF array
    while(top != -1 && 
      ((SA[i] < SA[stack[top]]) || 
       ((SA[i] > SA[stack[top]]) && (LCP[i] <= LCP[stack[top]])))) {
      int stack_top = stack[top];
      if(SA[i] < SA[stack_top]){
    LPF[SA[stack_top]] = max(LCP[i],LCP[stack_top]);
    LCP[i] = min(LCP[i],LCP[stack_top]);
      } else {
    LPF[SA[stack_top]] = LCP[stack_top];
      }
      top--;
    }
    if(i < n) stack[++top] = i;
  }

  free(stack); free(SA); free(LCP);

  //compute LZ array
  int* LZ = newA(int,n);
  LZ[0] = 0;
  int j = 0;
  while(LZ[j] < n){
    LZ[j+1] = LZ[j] + max(1,LPF[LZ[j]]);
    j++;
  }
  free(LPF);
  lzTm.reportNext("\tlpf && lz");
  return pair<int*,int>(LZ,j);
}

int parallel_main(int argc, char *argv[]) {
    return test_main(argc, argv, (char *)"Seq LZ77 DCC 2008", compute);
}


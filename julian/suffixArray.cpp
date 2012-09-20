// // This code is part of the Problem Based Benchmark Suite (PBBS)
// // Copyright (c) 2011 Guy Blelloch, Julian Shun and the PBBS team
// //
// // Permission is hereby granted, free of charge, to any person obtaining a
// // copy of this software and associated documentation files (the
// // "Software"), to deal in the Software without restriction, including
// // without limitation the rights (to use, copy, modify, merge, publish,
// // distribute, sublicense, and/or sell copies of the Software, and to
// // permit persons to whom the Software is furnished to do so, subject to
// // the following conditions:
// //
// // The above copyright notice and this permission notice shall be included
// // in all copies or substantial portions of the Software.
// //
// // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// // OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// // MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// // NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// // LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// // OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// // WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

// #include <iostream>
// #include "gettime.h"
// #include "sequence.h"
// #include "intSort.h"
// #include "parallel.h"
// #include "merge.h"
// #include "rangeMin.h"
// using namespace std;

// #ifdef OPENMP
// #include "PSRS.h"
// #endif

// bool isSorted(int *SA, int *s, int n);

// // Radix sort a pair of integers based on first element
// inline void radixSortPair(pair<int,int> *A, int n, long m) {
//   intSort::iSort(A,n,m,utils::firstF<int,int>());
// }

// inline bool leq(int a1, int a2,   int b1, int b2) {
//   return(a1 < b1 || a1 == b1 && a2 <= b2); 
// }                                                  

// inline bool leq(int a1, int a2, int a3, int b1, int b2, int b3) {
//   return(a1 < b1 || a1 == b1 && leq(a2,a3, b2,b3)); 
// }

// struct compS {
//   int* _s;
//   int* _s12;
//   compS(int* s, int* s12) : _s(s), _s12(s12) {}
//   int operator () (int i, int j) {
//     if (i%3 == 1 || j%3 == 1) 
//       return leq(_s[i],_s12[i+1], _s[j],_s12[j+1]);
//     else
//       return leq(_s[i],_s[i+1],_s12[i+2], _s[j],_s[j+1],_s12[j+2]);
//   }
// };

// #ifdef OPENMP
// struct compQ {
//     int *_s;
//     int i;

//     int operator < (const compQ &b) const {
//         int j = b.i;
//         if (_s[i] == _s[j] &&_s[i+1] == _s[j+1] && _s[i+2] == _s[j+2]) return i < j;

//         return leq(_s[i], _s[i + 1], _s[i + 2], _s[j], _s[j + 1], _s[j + 2]);
//     }
//     int operator > (const compQ &b) const {
//          int j = b.i;
//         if (_s[i] == _s[j] &&_s[i+1] == _s[j+1] && _s[i+2] == _s[j+2]) return i > j;

//          return !leq(_s[i], _s[i + 1], _s[i + 2], _s[j], _s[j + 1], _s[j + 2]);
//     }
// };
// #endif

// struct mod3is1 { bool operator() (int i) {return i%3 == 1;}};

// inline int computeLCP(int* LCP12, int* rank, myRMQ & RMQ, 
// 		      int j, int k, int* s, int n){
 
//   int rank_j=rank[j]-2;
//   int rank_k=rank[k]-2;
//   if(rank_j > rank_k) {swap(rank_j,rank_k);} //swap for RMQ query

//   int l = ((rank_j == rank_k-1) ? LCP12[rank_j] 
// 	   : LCP12[RMQ.query(rank_j,rank_k-1)]);

//   int lll = 3*l;
//   if (s[j+lll] == s[k+lll]) {
//     if (s[j+lll+1] == s[k+lll+1]) return lll + 2;
//     else return lll + 1;
//   } 
//   return lll;
// }

// // This recursive version requires s[n]=s[n+1]=s[n+2] = 0
// // K is the maximum value of any element in s
// pair<int*,int*> suffixArrayRec(int* s, int n, int K, bool findLCPs) {
//   n = n+1;
//   int n0=(n+2)/3, n1=(n+1)/3, n12=n-n0;
//   pair<int,int> *C = (pair<int,int> *) malloc(n12*sizeof(pair<int,int>));

//   int bits = utils::logUp(K);

//   // if 3 chars fit into an int then just do one radix sort
//   if (bits < 11) {
//     parallel_for (int i=0; i < n12; i++) {
//       int j = 1+(i+i+i)/2;
//       C[i].first = (s[j] << 2*bits) + (s[j+1] << bits) + s[j+2];
//       C[i].second = j;}

// #ifdef OPENMP
//     ParallelSortRS(C, n12);
//     //ParallelMergeSort(C, n12, less<pair<int, int> >());
// #else
//     radixSortPair(C, n12, 1 << 3*bits);
// #endif

//   // otherwise do 3 radix sorts, one per char
//   } else {
//     parallel_for (int i=0; i < n12; i++) {
//       int j = 1+(i+i+i)/2;
//       C[i].first = s[j+2]; 
//       C[i].second = j;}

// #ifdef OPENMP
//         compQ *tmp = new compQ[n12];
//         parallel_for (int i = 0; i < n12; i++) {
//             int j = 1 + (i + i + i) / 2;
//             tmp[i].i = j;
//             tmp[i]._s = s;
//         }
//         ParallelSortRS(tmp, n12);
//         //ParallelMergeSort(tmp, n12, less<compQ>());
//         parallel_for (int i = 0; i < n12; i++) {
//             C[i].second = tmp[i].i;
//         }
//         delete tmp;
// #else

//     // radix sort based on 3 chars
//     radixSortPair(C, n12, K);
//     parallel_for (int i=0; i < n12; i++) C[i].first = s[C[i].second+1];
//     radixSortPair(C, n12, K);
//     parallel_for (int i=0; i < n12; i++) C[i].first = s[C[i].second];
//     radixSortPair(C, n12, K);
// #endif

//   }

//   // copy sorted results into sorted12
//   int* sorted12 = newA(int,n12); 
//   parallel_for (int i=0; i < n12; i++) sorted12[i] = C[i].second;
//   free(C);

//   // generate names based on 3 chars
//   int* name12 = newA(int,n12);
//   parallel_for (int i = 1;  i < n12;  i++) {
//     if (s[sorted12[i]]!=s[sorted12[i-1]] 
// 	|| s[sorted12[i]+1]!=s[sorted12[i-1]+1] 
// 	|| s[sorted12[i]+2]!=s[sorted12[i-1]+2]) 
//       name12[i] = 1;
//     else name12[i] = 0;
//   }
//   name12[0] = 1;
//   sequence::scanI(name12,name12,n12,utils::addF<int>(),0);
//   int names = name12[n12-1];
  
//   pair<int*,int*> SA12_LCP;
//   int* SA12;
//   int* LCP12 = NULL;
//   // recurse if names are not yet unique
//   if (names < n12) {
//     int* s12  = newA(int,n12 + 3);  
//     s12[n12] = s12[n12+1] = s12[n12+2] = 0;

//     // move mod 1 suffixes to bottom half and and mod 2 suffixes to top
//     parallel_for (int i= 0; i < n12; i++)
//       if (sorted12[i]%3 == 1) s12[sorted12[i]/3] = name12[i];
//       else s12[sorted12[i]/3+n1] = name12[i];
//     free(name12);  free(sorted12);
//     //for (int i=0; i < n12; i++) cout << s12[i] << " : ";
//     //cout << endl;

//     SA12_LCP = suffixArrayRec(s12, n12, names+1, findLCPs); 
//     SA12 = SA12_LCP.first;
//     LCP12 = SA12_LCP.second;
//     free(s12);

//     // restore proper indices into original array
//     parallel_for (int i = 0;  i < n12;  i++) {
//       int l = SA12[i]; 
//       SA12[i] = (l<n1) ? 3*l+1 : 3*(l-n1)+2;
//     }
//   } else {
//     free(name12); // names not needed if we don't recurse
//     SA12 = sorted12; // suffix array is sorted array
//     if (findLCPs) {
//       LCP12 = newA(int,n12+3);
//       parallel_for(int i=0;i<n12+3;i++) 
// 	LCP12[i]=0; //LCP's are all 0 if not recursing
//     }
//   }

//   // place ranks for the mod12 elements in full length array
//   // mod0 locations of rank will contain garbage
//   int* rank  = newA(int,n + 2);  
//   rank[n]=1; rank[n+1] = 0;
//   parallel_for (int i = 0;  i < n12;  i++) {rank[SA12[i]] = i+2;}

  
//   // stably sort the mod 0 suffixes 
//   // uses the fact that we already have the tails sorted in SA12
//   int* s0  = newA(int,n0);
//   int x = sequence::filter(SA12,s0,n12,mod3is1());
//   pair<int,int> *D = (pair<int,int> *) malloc(n0*sizeof(pair<int,int>));
//   D[0].first = s[n-1]; D[0].second = n-1;
//   parallel_for (int i=0; i < x; i++) {
//     D[i+n0-x].first = s[s0[i]-1]; 
//     D[i+n0-x].second = s0[i]-1;}
//   radixSortPair(D,n0, K);
//   int* SA0  = s0; // reuse memory since not overlapping
//   parallel_for (int i=0; i < n0; i++) SA0[i] = D[i].second;
//   free(D);

//   compS comp(s,rank);
//   int o = (n%3 == 1) ? 1 : 0;
//   int *SA = newA(int,n); 
//   merge(SA0+o,n0-o,SA12+1-o,n12+o-1,SA,comp);
//   free(SA0); free(SA12);
//   int* LCP = NULL;

//   //get LCP from LCP12
//   if(findLCPs){
//     LCP = newA(int,n);  
//     LCP[n-1] = LCP[n-2] = 0; 
//     myRMQ RMQ(LCP12,n12+3); //simple rmq
//     parallel_for(int i=0;i<n-2;i++){ 
//       int j=SA[i];
//       int k=SA[i+1];
//       int CLEN = 16;
//       int ii;
//       for (ii=0; ii < CLEN; ii++) 
// 	if (s[j+ii] != s[k+ii]) break;
//       if (ii != CLEN) LCP[i] = ii;
//       else {
//       	if (j%3 != 0 && k%3 != 0)  
// 	  LCP[i] = computeLCP(LCP12, rank, RMQ, j, k, s, n); 
// 	else if (j%3 != 2 && k%3 != 2)
// 	  LCP[i] = 1 + computeLCP(LCP12, rank, RMQ, j+1, k+1, s, n);
// 	else 
// 	  LCP[i] = 2 + computeLCP(LCP12, rank, RMQ, j+2, k+2, s, n);
// 	  }
//     }
//     free(LCP12);
//   }
//   free(rank);
//   return make_pair(SA,LCP);
// }

// pair<int*,int*> suffixArray(int* s, int n, bool findLCPs) {
//   int *ss = newA(int,n+3); 
//   ss[n] = ss[n+1] = ss[n+2] = 0;
//   parallel_for (int i=0; i < n; i++) ss[i] = s[i]+1;
//   int k = 1 + sequence::reduce(ss,n,utils::maxF<int>());

//   pair<int*,int*> SA_LCP = suffixArrayRec(ss,n,k, findLCPs);
//   free(ss);
//   return SA_LCP;
// }

// int* suffixArrayNoLCP(int* s, int n) { 
//   return suffixArray(s,n,false).first;
// }

int *GetLCP(int * s, int n, int * SA) {   
    int i, j, h;  
    int * Rank = new int[n];
    int *Hgt = new int[n];

    Hgt[0] = 0;
    for (i = 0; i < n; i++) Rank[SA[i]] = i;
    for (h = 0, i = 0; i < n; i++) {
        if (Rank[i] > 0){   
            j = SA[Rank[i]-1];   
            while (s[i+h] == s[j+h]) ++h;   
            Hgt[Rank[i]] = h;   
            if (h > 0) --h;     
        }
    }
    return Hgt;
}   




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
#include "sequence.h"
#include "intSort.h"
#include "parallel.h"
#include "merge.h"
#include "rangeMin.h"
using namespace std;

bool isSorted(intT *SA, intT *s, intT n);

// Radix sort a pair of integers based on first element
void radixSortPair(pair<intT,intT> *A, intT n, long m) {
  intSort::iSort(A,n,m,utils::firstF<intT,intT>());
}

inline bool leq(intT a1, intT a2,   intT b1, intT b2) {
  return(a1 < b1 || a1 == b1 && a2 <= b2); 
}                                                  

inline bool leq(intT a1, intT a2, intT a3, intT b1, intT b2, intT b3) {
  return(a1 < b1 || a1 == b1 && leq(a2,a3, b2,b3)); 
}

struct compS {
  intT* _s;
  intT* _s12;
  compS(intT* s, intT* s12) : _s(s), _s12(s12) {}
  int operator () (intT i, intT j) {
    if (i%3 == 1 || j%3 == 1) 
      return leq(_s[i],_s12[i+1], _s[j],_s12[j+1]);
    else
      return leq(_s[i],_s[i+1],_s12[i+2], _s[j],_s[j+1],_s12[j+2]);
  }
};

struct mod3is1 { bool operator() (intT i) {return i%3 == 1;}};

inline intT computeLCP(intT* LCP12, intT* rank, myRMQ & RMQ, 
		      intT j, intT k, intT* s, intT n){
 
  intT rank_j=rank[j]-2;
  intT rank_k=rank[k]-2;
  if(rank_j > rank_k) {swap(rank_j,rank_k);} //swap for RMQ query

  intT l = ((rank_j == rank_k-1) ? LCP12[rank_j] 
	   : LCP12[RMQ.query(rank_j,rank_k-1)]);

  intT lll = 3*l;
  if (s[j+lll] == s[k+lll]) {
    if (s[j+lll+1] == s[k+lll+1]) return lll + 2;
    else return lll + 1;
  } 
  return lll;
}


// This recursive version requires s[n]=s[n+1]=s[n+2] = 0
// K is the maximum value of any element in s
pair<intT*,intT*> suffixArrayRec(intT* s, intT n, int K, bool findLCPs) {
  n = n+1;
  intT n0=(n+2)/3, n1=(n+1)/3, n12=n-n0;
  pair<intT,intT> *C = (pair<intT,intT> *) malloc(n12*sizeof(pair<intT,intT>));

  int bits = utils::logUp(K);
  // if 3 chars fit into an int then just do one radix sort
  if (bits < 11) {
    parallel_for (intT i=0; i < n12; i++) {
      intT j = 1+(i+i+i)/2;
      C[i].first = (s[j] << 2*bits) + (s[j+1] << bits) + s[j+2];
      C[i].second = j;}
    radixSortPair(C, n12, 1 << 3*bits);

  // otherwise do 3 radix sorts, one per char
  } else {
    parallel_for (intT i=0; i < n12; i++) {
      intT j = 1+(i+i+i)/2;
      C[i].first = s[j+2]; 
      C[i].second = j;}
    // radix sort based on 3 chars
    radixSortPair(C, n12, K);
    parallel_for (intT i=0; i < n12; i++) C[i].first = s[C[i].second+1];
    radixSortPair(C, n12, K);
    parallel_for (intT i=0; i < n12; i++) C[i].first = s[C[i].second];
    radixSortPair(C, n12, K);
  }

  // copy sorted results into sorted12
  intT* sorted12 = newA(intT,n12); 
  parallel_for (intT i=0; i < n12; i++) sorted12[i] = C[i].second;
  free(C);

  // generate names based on 3 chars
  intT* name12 = newA(intT,n12);
  parallel_for (intT i = 1;  i < n12;  i++) {
    if (s[sorted12[i]]!=s[sorted12[i-1]] 
	|| s[sorted12[i]+1]!=s[sorted12[i-1]+1] 
	|| s[sorted12[i]+2]!=s[sorted12[i-1]+2]) 
      name12[i] = 1;
    else name12[i] = 0;
  }
  name12[0] = 1;
  sequence::scanI(name12,name12,n12,utils::addF<intT>(),(intT)0);
  intT names = name12[n12-1];
  
  pair<intT*,intT*> SA12_LCP;
  intT* SA12;
  intT* LCP12 = NULL;
  // recurse if names are not yet unique
  if (names < n12) {
    intT* s12  = newA(intT,n12 + 3);  
    s12[n12] = s12[n12+1] = s12[n12+2] = 0;

    // move mod 1 suffixes to bottom half and and mod 2 suffixes to top
    parallel_for (intT i= 0; i < n12; i++)
      if (sorted12[i]%3 == 1) s12[sorted12[i]/3] = name12[i];
      else s12[sorted12[i]/3+n1] = name12[i];
    free(name12);  free(sorted12);
    //for (int i=0; i < n12; i++) cout << s12[i] << " : ";
    //cout << endl;

    SA12_LCP = suffixArrayRec(s12, n12, names+1, findLCPs); 
    SA12 = SA12_LCP.first;
    LCP12 = SA12_LCP.second;
    free(s12);

    // restore proper indices into original array
    parallel_for (intT i = 0;  i < n12;  i++) {
      intT l = SA12[i]; 
      SA12[i] = (l<n1) ? 3*l+1 : 3*(l-n1)+2;
    }
  } else {
    free(name12); // names not needed if we don't recurse
    SA12 = sorted12; // suffix array is sorted array
    if (findLCPs) {
      LCP12 = newA(intT,n12+3);
      parallel_for(intT i=0;i<n12+3;i++) 
	LCP12[i]=0; //LCP's are all 0 if not recursing
    }
  }

  // place ranks for the mod12 elements in full length array
  // mod0 locations of rank will contain garbage
  intT* rank  = newA(intT,n + 2);  
  rank[n]=1; rank[n+1] = 0;
  parallel_for (intT i = 0;  i < n12;  i++) {rank[SA12[i]] = i+2;}

  
  // stably sort the mod 0 suffixes 
  // uses the fact that we already have the tails sorted in SA12
  intT* s0  = newA(intT,n0);
  intT x = sequence::filter(SA12,s0,n12,mod3is1());
  pair<intT,intT> *D = (pair<intT,intT> *) malloc(n0*sizeof(pair<intT,intT>));
  D[0].first = s[n-1]; D[0].second = n-1;
  parallel_for (intT i=0; i < x; i++) {
    D[i+n0-x].first = s[s0[i]-1]; 
    D[i+n0-x].second = s0[i]-1;}
  radixSortPair(D,n0, K);
  intT* SA0  = s0; // reuse memory since not overlapping
  parallel_for (intT i=0; i < n0; i++) SA0[i] = D[i].second;
  free(D);

  compS comp(s,rank);
  int o = (n%3 == 1) ? 1 : 0;
  intT *SA = newA(intT,n); 
  merge(SA0+o,n0-o,SA12+1-o,n12+o-1,SA,comp);
  free(SA0); free(SA12);
  intT* LCP = NULL;

  //get LCP from LCP12
  if(findLCPs){
    LCP = newA(intT,n);  
    LCP[n-1] = LCP[n-2] = 0; 
    myRMQ RMQ(LCP12,n12+3); //simple rmq
    parallel_for(intT i=0;i<n-2;i++){ 
      intT j=SA[i];
      intT k=SA[i+1];
      int CLEN = 16;
      intT ii;
      for (ii=0; ii < CLEN; ii++) 
	if (s[j+ii] != s[k+ii]) break;
      if (ii != CLEN) LCP[i] = ii;
      else {
      	if (j%3 != 0 && k%3 != 0)  
	  LCP[i] = computeLCP(LCP12, rank, RMQ, j, k, s, n); 
	else if (j%3 != 2 && k%3 != 2)
	  LCP[i] = 1 + computeLCP(LCP12, rank, RMQ, j+1, k+1, s, n);
	else 
	  LCP[i] = 2 + computeLCP(LCP12, rank, RMQ, j+2, k+2, s, n);
	  }
    }
    free(LCP12);
  }
  free(rank);
  return make_pair(SA,LCP);
}

pair<intT*,intT*> suffixArray(intT* s, intT n, bool findLCPs) {
  startTime();
  intT *ss = newA(intT,n+3); 
  ss[n] = ss[n+1] = ss[n+2] = 0;
  parallel_for (intT i=0; i < n; i++) ss[i] = s[i]+1;
  intT k = 1 + sequence::reduce(ss,n,utils::maxF<intT>());

  pair<intT*,intT*> SA_LCP = suffixArrayRec(ss,n,k, findLCPs);
  free(ss);
  return SA_LCP;
}

intT* suffixArrayNoLCP(intT* s, intT n) { 
  return suffixArray(s,n,false).first;}

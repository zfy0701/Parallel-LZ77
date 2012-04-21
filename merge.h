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

#include "cilk.h"
#include "Base.h"
#include <queue>

#define _MERGE_BSIZE 8192

template <class ET, class F> 
int binSearchOld(ET* S, int n, ET v) {
  if (n == 0) return 0;
  else { 
    int mid = n/2;
   // if (f(v,S[mid])) return binSearch(S, mid, v, f);
    if (v < S[mid]) return binSearch(S, mid, v);
    else return mid + 1 + binSearch(S+mid+1, (n-mid)-1, v);
  }
}

template <class ET, class F> 
int binSearch(ET* S, int n, ET v, F f) {
  ET* T = S;
  while (n > 0) {
    int mid = n/2;
    if (f(v,T[mid])) n = mid;
    else {
      n = (n-mid)-1;
      T = T + mid + 1;
    }
  }
  return T-S;
}


template <class ET, class F> 
inline void SeqMerge(ET* S1, int l1, ET* S2, int l2, ET* R, F f) {
  ET* pR = R; 
  ET* pS1 = S1; 
  ET* pS2 = S2;
  ET* eS1 = S1+l1; 
  ET* eS2 = S2+l2;
  while (true) {
    *pR++ = f(*pS2,*pS1) ? *pS2++ : *pS1++;
    if (pS1==eS1) {std::copy(pS2,eS2,pR); break;}
    if (pS2==eS2) {std::copy(pS1,eS1,pR); break;}
  }
}

#define SPLIT_BSIZE (_MERGE_BSIZE >> 2)

template <class ET, class F> 
void ParMerge(ET* S1, int l1, ET* S2, int l2, ET* R, F f) {
 int lr = l1 + l2;
  if (lr > _MERGE_BSIZE) {
    if (l2>l1)  ParMerge(S2,l2,S1,l1,R,f);
    else {
      int ll1 = (l1 + SPLIT_BSIZE - 1) / SPLIT_BSIZE;

      int *pos1 = new int[ll1+1]; // pos1[i] means the position of s1[i*Splite] in s2
      *(pos1++) = 0;

      cilk_for (int i = 0; i < ll1; i++) {
        pos1[i] = binSearch(S2, l2, S1[min((i+1)*SPLIT_BSIZE, l1)-1], f);
      }

      pos1[ll1-1] = l2;        
      cilk_for (int i = 0; i < ll1; i++) {
        int start1 = i * SPLIT_BSIZE;
        int n1 = min(SPLIT_BSIZE, l1 - start1);
        int start2 = pos1[i-1];
        int n2 = pos1[i] - start2;
        SeqMerge(S1 + start1, n1, S2 + start2, n2, R + start1 + start2, f);
      }

      delete --pos1;
    }
  } else {
    SeqMerge(S1, l1, S2, l2, R, f);
  }
}

#define KSIZE 16
#define HSIZE 128

// template <class ET, class F> 
// class Cmp {
// //private:
// public:
//   static F f;

//   bool operator () (const pair<ET, int>  & x, const pair<ET,int> & y) {
//     return !f(x.first, y.first);
//   }
// };

template <class ET>
void kmerge(ET **st, ET **ed, ET *r, int k) {

  int depth = getDepth(k);
  int nthread = omp_get_max_threads();

  int i;

  for (i = 0; i < k; ) {
    if (st[i] == ed[i]) {
      st[i] = st[k-1];
      ed[i] = ed[k-1];
      k--;
    } else i++;
  }
  if (k == 0) return;
    
  if (k > KSIZE) {
    std::priority_queue<pair<ET,int>, vector<pair<ET,int> >, greater<pair<ET,int> > > pq;
    for (i = 0; i < k; i++) pq.push( make_pair(*st[i], i) ); 

    // pair<ET, int> heap[HSIZE];
    // for (i = 0; i < k; i++) heap[i] = make_pair(*st[i], i);
    // std::make_heap(heap, heap + k, greater<pair<ET, int> >() );

    int kk = k; 
    while (kk > KSIZE) {
      pair<ET,int> v = pq.top(); pq.pop();
      //pair<ET, int> v = heap[0];
      //pop_heap (heap, heap + kk); 

      *(r++) = v.first;
      
      if (++st[v.second] != ed[v.second]) {
        pq.push(make_pair(*(st[v.second]), v.second)); 
        // heap[kk-1] = make_pair(*(st[v.second]), v.second);
        // push_heap(heap, heap + kk);
      } else {
        kk--;
      }
    }

    // while (kk > 0) {
    //   pair<ET,int> v = pq.top(); pq.pop();
    //   *(r++) = v.first;
    //   ++st[v.second];
    //   kk--;
    // }

   // delete heap;
  }

  for (i = 0; i < k; ) {
    if (st[i] == ed[i]) {
      st[i] = st[k-1];
      ed[i] = ed[k-1];
      k--;
    } else i++;
  }
 if (k == 0) return;


  while (k > 2) {
    ET min_val = *st[0];
    int min_ind = 0;

    for (i = 1; i < k; i++) { //chose the minimum by brute force
      if (*st[i] < min_val) {
        min_val = *st[i];
        min_ind = i;
      }
    } 

    st[min_ind]++;
    if (st[min_ind] == ed[min_ind]) {
      st[min_ind] = st[k-1];
      ed[min_ind] = ed[k-1];
      k--;
    }
    *(r++) = min_val;
  } 

  if (k == 2) {
     std::merge(st[0], ed[0], st[1], ed[1], r);
  } else 
     while (st[0] != ed[0]) *(r++) = *(st[0]++);
}

template <class ET, class F> 
void merge(ET* S1, int l1, ET* S2, int l2, ET* R, F f) {
  ParMerge(S1, l1, S2, l2, R, f);
  return;

  int lr = l1 + l2;
  if (lr > _MERGE_BSIZE) {
    if (l2>l1)  merge(S2,l2,S1,l1,R,f);
    else {
      int m1 = l1/2;
      int m2 = binSearch(S2,l2,S1[m1],f);
      cilk_spawn merge(S1,m1,S2,m2,R,f);
      merge(S1+m1,l1-m1,S2+m2,l2-m2,R+m1+m2,f);
      cilk_sync;
    }
  } else {
    ET* pR = R; 
    ET* pS1 = S1; 
    ET* pS2 = S2;
    ET* eS1 = S1+l1; 
    ET* eS2 = S2+l2;
    while (true) {
      *pR++ = f(*pS2,*pS1) ? *pS2++ : *pS1++;
      if (pS1==eS1) {std::copy(pS2,eS2,pR); break;}
      if (pS2==eS2) {std::copy(pS1,eS1,pR); break;}
    }
  }
}

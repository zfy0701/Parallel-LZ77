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

// -*- C++ -*-

#ifndef A_SEQ_INCLUDED
#define A_SEQ_INCLUDED

#include <iostream>
#include "parallel.h"
#include "sequence.h"

using namespace std;

typedef void *voidp;

#define _BBSIZE 2048

template <class ET>
class seq {
private:
  static intT _nextPow(intT i) {
    intT a=0;
    intT b=i-1;
    while (b > 0) {b = b >> 1; a++;}
    return (1 << a);
  }

public:
  ET* S;
  intT sz;
  seq(ET* seq, intT size) : sz(size),S(seq) {}
  seq() {sz = 0; S = NULL; }
  seq(ET v) { sz = 1; S = newA(ET,1); S[0] = v;}

  seq copy() {
    ET *A = newA(ET,sz);
    parallel_for(intT i=0;i<sz;i++) A[i] = S[i];
    return seq(A,sz);
  }

   template <class F> 
   static void tabulate(ET* I, intT s, intT e, F f) {
     intT n = e-s;
     if (n > _BSIZE) {
       parallel_spawn tabulate(I, s, s+n/2, f);
       tabulate(I, s+n/2, e, f);
       parallel_sync;
     } else {
       for (intT i=s; i<e; i++) I[i] = f(i);
     }
  }

  template <class F> seq(intT n, F f) {
    sz = n; S = newA(ET,sz);
    tabulate(S,0,n,f);
  }

  ET& operator[] (const intT i) { return S[i]; }

  void del() {if (S!=NULL) free(S);}
  intT size() {return sz;} 
  ET nth(intT i) {return S[i];}

  template <class F> 
  pair<seq,seq> split(F f) {
    ET* S1 = newA(ET,sz); ET* S2 = newA(ET,sz);
    intT i,j;
    for (j=0, i=0; i<sz; i++)
      if (f(S[i])) S1[j++] = S[i]; else S2[i-j] = S[i];
    return pair<seq,seq>(seq(S1,j),seq(S2,sz-j));
  }

  template <class F> ET reduce(F f) {return sequence::reduce(S,sz,f);}

  template <class OT, class FM, class FR> OT mapReduce(FM fm, FR fr) {
    return sequence::mapReduce<ET,OT,FR,FM>(S,sz,fr,fm);}

  template <class F> 
  intT maxIndex(F f) {return sequence::maxIndex(S,sz,f); }

  seq append(seq S2) {
    intT rsize = S2.size()+sz;
    ET* R = newA(ET,rsize);
    parallel_for (intT i=0; i<sz; i++) R[i] = S[i];
    parallel_for (intT ii=0; ii<S2.size(); ii++) R[ii+sz] = S2.S[ii];
    return seq(R,rsize);
  }

  seq appendD(seq S2) {
    intT rsize = S2.size()+sz;
    ET* R = newA(ET,rsize);
    parallel_for (intT i=0; i<sz; i++) R[i] = S[i];
    parallel_for (intT ii=0; ii<S2.size(); ii++) R[ii+sz] = S2.S[ii];
    free(S2.S); free(S);
    return seq(R,rsize);
  }

  template <class OT, class F> 
  void mapR(OT* R, intT s, intT n, F f) {
    if (n > _BSIZE) {
      parallel_spawn mapR(R,s,n/2,f); 
      mapR(R,s+n/2,n-n/2,f);
      parallel_sync;
    } else for (intT i=s; i<s+n; i++) R[i] = f(S[i]);
  }

  template <class OT, class F> 
  seq<OT> map(F f) {
    OT* R = newA(OT,sz);
    mapR(R,0,sz,f);
    return seq<OT>(R,sz);
  }

  template <class F> 
  seq scan(F f, ET zero) {
    ET *Out = newA(ET,sz);
    sequence::scan(S,Out,sz,f,zero);
    return seq(Out,sz);
  }

  template <class F> 
  seq scanI(F f, ET zero) {
    ET *Out = newA(ET,sz);
    sequence::scanI(S,Out,sz,f,zero);
    return seq(Out,sz);
  }

  template <class F>
  static seq<intT> packIndex(intT start, intT n, F f) {
    intT *Out = newA(intT,n);
    intT m = sequence::packIndex(Out,start,start+n,f);
    return seq(Out,m);
  }

  seq<ET> pack(seq<bool> Fl) {
    ET *Out = newA(ET,sz);
    intT m = sequence::pack(S,Out,Fl.S,sz);
    return seq(Out,m);
  }

  template <class F> 
  seq filter(F f) {
    ET *Out = newA(ET,sz);
    intT n = sequence::filter(S,Out,sz,f);
    return seq(Out,n);
  }

};

template <class ETYPE>
struct printSeqElementFG {
  ostream &os;
  printSeqElementFG(ostream &o) : os(o) {}
  void operator()(ETYPE e) { os << "," << e;}
};

template <class ETYPE>
ostream &operator<<(ostream &os, seq<ETYPE> S) {
  os << "[";
  if (S.size() > 0) {
    os << S.nth(0);
    for (intT i=1; i < S.size(); i++)
      os << "," << S.nth(i);
  }
  os << "]";
  return os;
}





#endif // _A_SEQ_INCLUDED

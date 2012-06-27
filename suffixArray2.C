#include <iostream>
#include <algorithm>
#include "gettime.h"
#include "sequence.h"
#include "intSort.h"
#include "quickSort.h"
#include "merge.h"
#include "cilk.h"
using namespace std;

#define printInfo

#ifdef printInfo
#define nextTimeM(_str) nextTime(_str)
#else
#define nextTimeM(_str) 
#endif

typedef unsigned int uint;
typedef unsigned char uchar;
typedef pair<uint,uint> intpair;

struct seg {
  int start;
  int length;
  seg(int s, int l) : start(s), length(l) {}
};

struct isSeg {bool operator() (seg s) {return s.length > 1;}};

inline int grabChars(uchar *s, int bits, int nChars) {
  int r = s[0];
  for (int i=1; i < nChars; i++) r = r<<bits | s[i];
  return r; 
}

inline int grabCharsEnd(uchar *s, int bits, int nChars, int end) {
  int r = s[0];
  for (int i=1; i < nChars; i++) 
    r = r<<bits | ((i < end) ? s[i] : 0);
  return r; 
}

struct pairCompF {
  bool operator() (intpair A, intpair B) { return A.first < B.first;}};

void splitSegment(seg *segOut, int start, int l, int* ranks, intpair *Cs,
		  bool addRanks) {
  if (l < 0) cilk_spawn printf("ouch");
  if (l < 1000) { // sequential version

    if (addRanks) {
      // if following two loops are fused performance goes way down?
      int name = 0;
      ranks[Cs[0].second] = name + start + 1;
      for (int i=1; i < l; i++) {
	if (Cs[i-1].first != Cs[i].first) name = i;
	ranks[Cs[i].second] = name + start + 1;
      }
    }

    int name = 0;
    for (int i=1; i < l; i++) {
      if (Cs[i-1].first != Cs[i].first) {
	segOut[i-1] = seg(name+start,i-name);
	name = i;
      } else segOut[i-1] = seg(0,0);
    }
    segOut[l-1] = seg(name+start,l-name);

  } else { // parallel version
    int *names = newA(int,l);

    cilk_for (int i = 1;  i < l;  i++) 
      names[i] = (Cs[i].first != Cs[i-1].first) ? i : 0;
    names[0] = 0;
    //nextTimeM("names");

    sequence::scanI(names,names,l,utils::maxF<int>(),0);
    //nextTimeM("scan");

    if (addRanks) 
      cilk_for (int i = 0;  i < l;  i++) 
	ranks[Cs[i].second] = names[i]+start+1;
    //nextTimeM("scatter");

    cilk_for (int i = 1;  i < l;  i++)
      if (names[i] == i) 
	segOut[i-1] = seg(start+names[i-1],i-names[i-1]);
      else segOut[i-1] = seg(0,0);
    segOut[l-1] = seg(start+names[l-1],l-names[l-1]);
    //nextTimeM("segout");

    free(names);
  }
}  

void brokenCilk(int nSegs, seg *segments, intpair *C, int offset, int n, int* ranks, seg *segOuts, int* offsets) {
  if (n < 0) cilk_spawn printf("ouch");
  cilk_for (int i=0; i < nSegs; i++) {
    int start = segments[i].start;
    intpair *Ci = C + start;
    int l = segments[i].length;
    _cilk_grainsize_256
    cilk_for (int j=0; j < l; j++) {
      int o = Ci[j].second+offset;
      Ci[j].first = (o >= n) ? n-o : ranks[o];
    }
    if (l >= 256) 
      intSort::iSort(Ci, l, n ,utils::firstF<int,int>());
    else
      quickSort(Ci,l,pairCompF());
  }

  nextTimeM("sort");

  cilk_for (int i=0; i < nSegs; i++) {
    int start = segments[i].start;
    splitSegment(segOuts + offsets[i], start, segments[i].length, 
		 ranks, C + start, 1);
  }
  nextTimeM("split");
}

int* suffixArray(uchar* ss, int n) {
  if (n < 0) cilk_spawn printf("ouch");
  //for (int i=0; i < n; i++) cout << "str[" << i << "] = " << s[i] << endl;
  intpair *C = newA(intpair,n);
  int *ranks = newA(int,n);
  uchar *s = newA(uchar,n);

  int flags[256];
  for (int i=0; i < 256; i++) flags[i] = 0;
  cilk_for (int i=0; i < n; i++) 
    if (!flags[ss[i]]) flags[ss[i]] = 1;

  int m = sequence::scan(flags,flags,256,utils::addF<int>(),0);
  cilk_for (int i=0; i < n; i++) 
    s[i] = flags[ss[i]];
  #ifdef printInfo
  cout << "m = " << m << endl;
  #endif

  int bits = max(1,utils::log2(m));
  int nchars = 32/bits;
  int *foobar = ranks;

  startTime();
  cilk_for (int i=0; i < n-nchars+1; i++) {
    C[i].first = grabChars(s+i,bits,nchars); 
    //foobar[i] = C[i].first;
    C[i].second = i;
  }

  for (int i=max(n-nchars+1,0); i < n; i++) {
    C[i].first = grabCharsEnd(s+i,bits,nchars,n-i); 
    //foobar[i] = C[i].first;
    C[i].second = i;
  }
  free(s);

  nextTimeM("copy");
  intSort::iSort(C,n,((long) 1) << bits*nchars,utils::firstF<int,int>());
  nextTimeM("sort");

  seg *segOuts = newA(seg,n);
  seg *segments= newA(seg,n);
  int *offsets = newA(int,n);
  splitSegment(segOuts, 0, n, ranks, C, 1);
  nextTimeM("split");

  int offset = nchars;
  
  int round =0;
  int nKeys = n;
  while (1) {
    utils::myAssert(round++ < 40, "Suffix Array:  Too many rounds");
    int nSegs = sequence::filter(segOuts,segments,nKeys,isSeg());
    if (nSegs == 0) break;

    cilk_for (int i=0; i < nSegs; i++)
      offsets[i] = segments[i].length;

    nKeys = sequence::scan(offsets,offsets,nSegs,utils::addF<int>(),0);
    #ifdef printInfo
    cout << "nSegs = " << nSegs << " nKeys = " << nKeys 
	 << " common length = " << offset << endl;
    #endif
    nextTimeM("filter and scan");    

    // cilk_for breaks the loop
    brokenCilk(nSegs, segments, C, offset, n, ranks, segOuts, offsets);

    offset = 2 * offset;
  }
  for (int i=0; i < n; i++) ranks[i] = C[i].second;
  free(C); free(segOuts); free(segments); free(offsets); 
  //for (int i=0; i < n; i++) cout << "SA[" << i << "] = " << ranks[i] << endl;
  return ranks;
}

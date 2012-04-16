#ifndef _BASE_H_
#define _BASE_H_

#include <math.h>

static int mymem[1 << 28];
static int bm = 0;

inline int * myalloc(int n) {
	int i = bm;
	bm += n;
	return mymem + i;
}

inline void mydealloc(int n) {
	bm -= n;
}


const int BSIZE = 16;

inline int getDepth(int i) {
  int a=0;
  int b=i-1;
  while (b > 0) {b = b >> 1; a++;}
  return a+1;
}

inline int fflog2(int i) {
	int res = -1;
	if (i >> 16) {
		res += 16;
		i >>= 16;
	} else i &= 0xffff;
	
	for (; i; i >>= 1) res++;
	return res;
}

inline int cflog2(int i) {
	int res = 0;

	i--;
	if (i >> 16) {
		res += 16;
		i >>= 16;
	} else i &= 0xffff;
	
	for (; i; i >>= 1) res++;
	return res;
}

//#define max(x, y) (((x) > (y))?(x) : (y))
//#define min(x, y) (((x) < (y))?(x) : (y))

//#define max(x, y) (((x) > (y))?(x) : (y))
//#define min(x, y) (((x) < (y))?(x) : (y))


#endif
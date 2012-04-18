#ifndef _BASE_H_
#define _BASE_H_

#include <math.h>
#include <stdio.h>

const int BSIZE = 16;

inline int getDepth(int i) {
  int a=0;
  int b=i-1;
  while (b > 0) {b = b >> 1; a++;}
  return a+1;
}

static int mylog2[1<<16];


inline int fflog2(int i) {
	int res = -1;
	if (i >> 16) {
		res += 16;
		i >>= 16;
	} else i &= 0xffff;
	
	if (i >> 8) {
		res += 8;
		i >>= 8;
	} else i &= 0xff;
		
	if (i >> 4) {
		res += 4;
		i >>= 4;
	} else i &= 0xf;
	
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
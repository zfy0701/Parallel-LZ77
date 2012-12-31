#ifndef _BASE_H_
#define _BASE_H_

#include <cmath>
#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "parallel.h"

int test_main(int argc, char *argv[], char * algoname, std::pair<intT *, intT> lz77(intT *s, intT n));

const int BSIZE = 16;

inline intT getDepth(intT i) {
  intT a=0;
  intT b=i-1;
  while (b > 0) {b = b >> 1; a++;}
  return a+1;
}

static intT mylog2[1<<16];

inline intT fflog2(intT i) {
	intT res = -1;
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


inline intT cflog2(intT i) {
	intT res = 0;

	i--;
	if (i >> 16) {
		res += 16;
		i >>= 16;
	} else i &= 0xffff;
	
	for (; i; i >>= 1) res++;
	return res;
}


inline intT superfflog2(intT i) {
	if (i >> 16) {
		return 16 + mylog2[i>>16];
	} else {
		return mylog2[i];
	}
}

inline void initlog2() {
	intT i;
	for (i = 0; i <= 0xffff; i++) {
		intT j = i, k = -1;
		while (j) {j >>= 1; k++;}
		mylog2[i] = k;
		//if (mylog2[i] != check(i)) printf("(%d %d) ", mylog2[i], check(i)); 
	}
	//for (int i = 0; i <= 0xffff; i++) printf("(%d %d) ", mylog2[i], check(i)); printf("\n");
}

inline char* itoa(intT val, int base = 10){
	static char buf[32] = {0};
	int i = 30;
	for(; val && i ; --i, val /= base)
		buf[i] = "0123456789abcdef"[val % base];
	return &buf[i+1];	
}

#endif

#include "RMQ.h"
#include "Base.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>

using namespace std;

//const double ln2 = log(2.0);

inline int superfflog2(int i) {
	if (i >> 16) {
		return 16 + mylog2[i>>16];
	} else {
		return mylog2[i];
	}
}

inline void initlog2() {
	int i;
	for (i = 0; i <= 0xffff; i++) {
		int j = i, k = -1;
		while (j) {j >>= 1; k++;}
		mylog2[i] = k;
		//if (mylog2[i] != check(i)) printf("(%d %d) ", mylog2[i], check(i)); 
	}
	//for (int i = 0; i <= 0xffff; i++) printf("(%d %d) ", mylog2[i], check(i)); printf("\n");
}

int queryRMQ(int **table, int i, int j) {
	//int k = (int)floor(log(1.0 + j - i) / ln2); //TODO: optimize here
	//int k = 0;	for (int t = (1 + j - i) >> 1; t; t >>= 1) k++;
	int k = superfflog2(1 + j - i);
	
	return min(table[k][i], table[k][j - (1 << k) + 1]);
}

void buildRMQ(int *a, int n, int **table) {
	//int tot = 0;
	initlog2();

	int depth = getDepth(n);
	
	//printf("%d %d\n", n, depth);

	#pragma omp parallel for 
	for(int i = 0; i < n; i++) {
      table[0][i] = a[i];
  }
  
	#pragma omp parallel 
	{
		#pragma omp for 
		for(int i = 0; i < n; i++) {
      		table[0][i] = a[i];
  		}
	
		int m = n, dist = 1;
		for (int d = 1; d < depth; d++) {
			#pragma omp for nowait
			for(int i = 0; i < n - dist; i++) {
				table[d][i] = min(table[d-1][i], table[d-1][i+dist]);
			}
			
			#pragma omp for 
			for (int i = n - dist; i < n; i++) {
				table[d][i] = table[d-1][i];
			}
			dist <<= 1;	
			#pragma omp barrier
		}
	}
}

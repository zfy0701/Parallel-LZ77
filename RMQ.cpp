#include "RMQ.h"
#include "Base.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>

using namespace std;

//const double ln2 = log(2.0);

int queryRMQ(int **table, int i, int j) {
	//int k = (int)floor(log(1.0 + j - i) / ln2); //TODO: optimize here
	//int k = 0;	for (int t = (1 + j - i) >> 1; t; t >>= 1) k++;
	int k = fflog2(1 + j - i);
	
	return min(table[k][i], table[k][j - (1 << k) + 1]);
}

void buildRMQ(int *a, int n, int **table) {
	//int tot = 0;
	
	int depth = getDepth(n);
	
	printf("%d %d\n", n, depth);

	#pragma omp parallel for 
	for(int i = 0; i < n; i++) {
      table[0][i] = a[i];
  }
  
/* 
	int m = n, dist = 1;
	for (int d = 1; d < depth; d++) {
		#pragma omp parallel for
		for(int i = 0; i < n - dist; i++) {
    	table[d][i] = min(table[d-1][i], table[d-1][i+dist]);
		}
		
		#pragma omp parallel for 
		for (int i = n - dist; i < n; i++) {
			table[d][i] = table[d-1][i];
		}
		dist <<= 1;		
	}
 */

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

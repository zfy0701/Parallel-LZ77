#include <stdio.h>
#include <iostream>
#include "gettime.h"

#include "ANSV.h"

#include "rangeMin.h"

#include "sequence.h"
#include "intSort.h"
#include "utils.h"
#include "Base.h"
//#include "mysort.h"
#include "merge.h"
#include "cilk.h"



int LempelZiv(int *s, int n, int *LZ);
int ParallelLZ77(int *s, int n, int *lz);

int getLZ(int *lpf, int n, int *lz);
pair<int *, int *> suffixArray(int *s, int n, bool findLCPs);

#ifdef OPENMP
void setThreads(int p) {
  omp_set_num_threads(p);
}
#endif

// void checkLZ(int d) {
// 	int n = 1 << d;
// 	int *a = new int[n + 1], *ql = new int[n + 1], *qr = new int[n + 1];
// 	int i;
// 	for ( i = 0; i < n; i++)
// 		a[i] = min(rand() % n, n - 1 - i);
// 	a[0] = 0;

// 	startTime();
// 	for (int i = 1; i < n; i++) {
// 		ql[i] = i + max(1, a[ql[i - 1]]);
// 	}
// 	nextTime("lz seq");

// 	printf("lz par:\n");
// 	getLZ(a, n, ql);

// 	printf("\n");
// }



void checkAll(int np, int d, int sigma) {
	int n = 1 << d;
	int *a = new int[n + 3];
	int i;
	int *lz = new int[n];

	for (int s = 4; s <= sigma; s *= 2) {
		printf("***************** TEST ON APLPHABET SIZE = %d *****************\n", s);
		for ( i = 0; i < n; i++)
			a[i] = rand() % s + 1;
		a[n] = a[n + 1] = a[n + 2] = 0;

		startTime();

		printf("#result %d\n", ParallelLZ77(a, n, lz));
		reportTime("parallel total time:");
		printf("\nParallel:\n");
		// for (int p = 1; p <= np; p *= 2) {
		// 	setThreads(p);

		// 	startTime();

		// 	printf("#procs %d, #result %d\n", p, ParallelLZ77(a, n, lz));
		// 	reportTime("parallel total time:");
		// 	printf("\n");
		// }

		printf("***************** END OF TEST ON ALLPHABET SIZE = %d *****************\n\n", s);
	}

	delete lz; delete a;
	printf("ALL TEST DONE!\n");
}

void getText(int n, char *path, int *dst) {
	char *buf = new char[n+1];
	FILE *fptr = fopen(path, "r");
	int nn = fread(buf, 1, n, fptr);

	printf("size %d %d\n", n, nn);

	for (int i = 0; i < n; i++)
		dst[i] = ((unsigned int)buf[i]) % 127 + 1;

	fclose(fptr);
	delete buf;
}

void checkSourceFile(int np, int d, char *path) {
	//printf("%s\n", path);
	//flush();


	int n = 1 << d;
	
	//n = 128;
	
	int *a = new int[n + 3];
	int i;
	int *lz = new int[n];

	printf("***************** TEST ON LINX CODE *****************\n");

	getText(n, path, a);
	a[n] = a[n + 1] = a[n + 2] = 0;

	
	printf("\nParallel:\n");
	printf("#result %d\n", ParallelLZ77(a, n, lz));
	nextTime("parallel total time:");

	
	// for (int p = 1; p <= np; p *= 2) {
	// 	setThreads(p);

	// 	startTime();

	// 	printf("#procs %d, #result %d\n", p, ParallelLZ77(a, n, lz));
	// 	reportTime("parallel total time:");
	// 	printf("\n");
	// }

	printf("***************** END TEST LINX CODE *****************\n\n");
	
	 delete lz; delete a; 
	//printf("ALL TEST DONE!\n");
}



char *path;

int cilk_main(int argc, char *argv[]) {
	int p = 2, d = 28;
	int sigma = 4;
	for (int i = 1; i < argc; i++) {
		//printf("%s\n", argv[i]);
		if (i == 1) p = atoi(argv[i]);
		if (i == 2) d = atoi(argv[i]);
		if (i == 3) sigma = atoi(argv[i]);
		if (i == 4) path = argv[i];
	}

	//printf("%d %d %d %s\n", p, d, sigma, path);
	#ifdef OPENMP
	setThreads(p);
	#endif

	checkSourceFile(p, d, path);

	//checkAll(p, d, sigma);

}

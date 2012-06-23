#include <stdio.h>
#include <iostream>
#include "gettime.h"

#include "ANSV.h"
#include "RMQ.h"
//#include "PrefixSum.h"
#include "rangeMin.h"

#include "sequence.h"
#include "intSort.h"
#include "utils.h"
#include "Base.h"
#include "mysort.h"
#include "merge.h"

//#include "ansv2.h"

int LempelZiv(int *s, int n, int *LZ);
int ParallelLZ77(int *s, int n, int *lz);

int getLZ(int *lpf, int n, int *lz);
pair<int *, int *> suffixArray(int *s, int n, bool findLCPs);

void setThreads(int p) {
	#ifdef OPENMP
	omp_set_num_threads(p);
	#endif

	//TODO add cilk support
}

// void checkANSV(int d) {
// 	int n = 1 << d;
// 	int *a = new int[n];

// 	for ( int i = 0; i < n; i++)
// 		a[i] = min(rand() % n, n - 1 - i);

// 	startTime();
// 	int *ql = new int[n], *qr = new int[n];
// 	//int **table = new int*[d+1];
// 	//   for (int i = 0; i < d+1; i++) table[i] = new int[n];

// 	ComputeANSV(a, n, ql, qr);

// 	nextTime("ansv");

// 	ANSV an(a, n);
// 	nextTime("ansv2");
// 	sequence::scan(a, a, n, utils::addF<int>(), 0);
// 	nextTime("prefix sum");

// 	int *l = an.getLeftNeighbors();
// 	int *r = an.getRightNeighbors();
// 	int i;
// 	for (i = 0; i < n; i++) {
// 		if (l[i] != ql[i]) {
// 			printf("l: %d %d %d\n", i, l[i], ql[i]);
// 			break;
// 		}
// 		if (r[i] != qr[i]) {
// 			printf("l: %d %d %d\n", i, r[i], qr[i]);
// 			break;
// 		}
// 	}
// 	if (i == n) printf("pass\n");

// 	printf("\n");

// 	//for (int i = 0; i < d+1; i++) delete table[i];
// 	//delete table;

// 	delete ql;
// 	delete qr;

// 	delete a;
// }

void checkLZ(int d) {
	int n = 1 << d;
	int *a = new int[n + 1], *ql = new int[n + 1], *qr = new int[n + 1];
	int i;
	for ( i = 0; i < n; i++)
		a[i] = min(rand() % n, n - 1 - i);
	a[0] = 0;

	startTime();
	for (int i = 1; i < n; i++) {
		ql[i] = i + max(1, a[ql[i - 1]]);
	}
	nextTime("lz seq");

	printf("lz par:\n");
	getLZ(a, n, ql);

	printf("\n");
}



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

		//printf("Sequential:\n");
		//startTime();
		//printf("lz single: %d\n", LempelZiv(a, n, lz));
		//reportTime("seq total time:");

		printf("\nParallel:\n");
		for (int p = 1; p <= np; p *= 2) {
			setThreads(p);

			startTime();

			printf("#procs %d, #result %d\n", p, ParallelLZ77(a, n, lz));
			reportTime("parallel total time:");
			printf("\n");
		}

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

	printf("Sequential:\n");
	startTime();
	printf("lz single: %d\n", LempelZiv(a, n, lz));
	reportTime("seq total time:");

	printf("\nParallel:\n");
	for (int p = 1; p <= np; p *= 2) {
		setThreads(p);

		startTime();

		printf("#procs %d, #result %d\n", p, ParallelLZ77(a, n, lz));
		reportTime("parallel total time:");
		printf("\n");
	}

	printf("***************** END TEST LINX CODE *****************\n\n");
	
	 delete lz; delete a; 
	//printf("ALL TEST DONE!\n");
}

void checkSuffix(int p, int d) {
	int n = 1 << d;
	int *a = new int[n + 3];
	int i;
	for ( i = 0; i < n; i++)
		a[i] = rand() % n + 1;
	a[n] = a[n + 1] = a[n + 2] = 0;

	int *SA = new int[n];

	//int *SA2 = SuffixArray(a, n);

	//suffixArray(a, SA, n, 2048);
	a[n] = a[n + 1] = a[n + 2] = 0;

	for (int k = 1; k <= p; k <<= 1) {
		setThreads(k);

		printf("num of procs: %d\n", k);
		startTime();
		int  *sa = suffixArray(a, n, false).first;
		delete sa;
		reportTime();
	}

	//for (int i = 0; i< n; i++) printf("%d ", a[i]); printf("\n");
	// for (int i = 0; i < n; i++) {
	//     if (sa[i] != SA[i])
	//         printf("sa mismatch %d %d | %d %d\n", i, a[i], sa[i], SA[i]);
	// }
	delete SA;
	printf("\n");
}

// void checkRMQ( int d) {
// 	int n = 1 << d;
// 	int *a = new int[n], *ql = new int[n], *qr = new int[n];
// 	int **table = new int*[d + 1];

// 	int i;
// 	for (i = 0; i < d + 1; i++) table[i] = new int[n];

// 	for ( i = 0; i < n; i++) {
// 		a[i] = min(rand() % n, n - 1 - i);

// 		int l = rand() % n;
// 		int r = rand() % n;
// 		if (l == n - 1) l--;
// 		if (r <= l) r = l + 1;
// 		ql[i] = l;
// 		qr[i] = r;
// 	}

// 	startTime();
// 	buildRMQ(a, n, table);
// 	nextTime("rmq1 build");
// 	#pragma omp parallel for schedule(dynamic, 256)
// 	for (int i = 0; i < n; i++) {
// 		queryRMQ(table, ql[i], qr[i]);
// 		queryRMQ(table, ql[i], qr[i]);
// 	}
// 	nextTime("rmq1 query");

// 	startTime();
// 	myRMQ rq(a, n);
// 	rq.precomputeQueries();
// 	nextTime("rmq2 build");

// 	#pragma omp parallel for schedule(dynamic, 256)
// 	for (int i = 0; i < n; i++) {
// 		rq.query(ql[i], qr[i]);
// 		rq.query(ql[i], qr[i]);
// 	}
// 	nextTime("rmq2 query");
// 	printf("\n");

// 	//for (int i = 0; i < n; i++) {     printf("%d ", a[i]);    } printf("\n");
// 	for (int i = 0; i < n; i++) {
// 		int r1 = queryRMQ(table, ql[i], qr[i]);
// 		int r2 = a[rq.query(ql[i], qr[i])];
// 		if (r1 != r2) printf("dismatch [%d %d] %d %d\n", ql[i], qr[i], r1, r2);
// 		//else printf("match");
// 	}

// 	for (i = 0; i < d + 1; i++) delete table[i];
// 	delete table;
// 	delete a;
// 	delete ql;
// 	delete qr;
// }

// void checkScan( int d) {
// 	int n = 1 << d;
// 	int *a = new int[n], *b = new int[n], *c = new int[n];

// 	int i;

// 	for ( i = 0; i < n; i++) {
// 		a[i] = rand() % n;
// 	}
// 	startTime();
// 	b[0] = 0;
// 	for ( i = 1; i < n; i++)
// 		b[i] = a[i - 1] + b[i - 1];
// 	nextTime("seq  prefix sum");

// 	sequence::scan(a, c, n, utils::addF<int>(), 0);
// 	nextTime("par prefix sum2");

// 	exclusiveScan(a, n);
// 	nextTime("par prefix sum1");

// 	for ( i = 1; i < n; i++)
// 		if (c[i] != b[i] || b[i] != a[i]) break;

// 	if (i == n) printf("prefix sum pass\n");
// 	else {
// 		printf("prefix sum fail\n");
// 		for (i = 1; i < 10; i++) {
// 			printf("(%d %d %d)", a[i], b[i], c[i]);
// 		}
// 		printf("\n");
// 	}

// 	delete a;
// 	delete b;
// 	delete c;
// }

int cal(int i) {
	int k = 0;
	for (int j = 0; j < 1000; j++) {
		k += j;
	}
	return k;
}

// void checkSimple( int d) {
// 	int n = 1 << d;
// 	int *a = new int[128];

// 	printf("tot: %d\n", n);

// 	startTime();

// 	for (int i = 0; i < n; i++) {
// 		a[i % 128] = cal(i);
// 	}
// 	nextTime("seq");

// 	startTime();
// 	#pragma omp parallel for
// 	for (int i = 0; i < n; i++) {
// 		a[i % 128] = cal(i);
// 	}
// 	nextTime("par");

// 	startTime();
// 	int p = omp_get_num_threads();
// 	#pragma omp parallel
// 	{
// 		int tid = omp_get_thread_num();
// 		int work = n / p;
// 		int l = tid * work;
// 		int r = min(n, l + work);
// 		//printf("%d %d %d\n", tid, l, r);
// 		for (int i = l; i < r; i++) {
// 			a[i % 128] = cal(i);
// 		}
// 	}
// 	nextTime("par2");

// 	startTime();
// 	for (int i = 0; i < n; i++) {
// 		a[i % 128] = cal(i);
// 	}
// 	nextTime("seq");

// 	delete a;
// }

#define MAXN 64

int sa[MAXN] =  {8, 9, 3, 12, 10, 0, 4, 13, 7, 2, 11, 6, 1, 5};
int lcp[MAXN] = {0, 2, 3, 1,  2,  2, 3, 0,  1, 3, 2,  1, 4, 2};

int lpf[MAXN];
//int lz[MAXN];
int *lz = NULL;

void checkCorrect() {

	char *a = (char *)"abbaabbbaaabab";
	int n = strlen(a);
	int *s = new int[n];

	for (int i = 0; i < n; i++) {
		s[i] = a[i];
	}

	int m = ParallelLZ77(s, n, lz);

	// int n = 14;

	// getLPF(sa, n, lcp, lpf);
	// printf("lpf:\n"); for (int i = 0; i < n; i++) printf("%2d ", lpf[i]); printf("\n");

	// int m = getLZ(lpf, n, lz);

	printf("lz: "); for (int i = 0; i < m; i++) printf("%2d ", lz[i]); printf("\n");
	printf("st:  0  1  2  3  4  7 10 12\n");

}
int a [10000000] = {8, 15, 21, 54, 64, 75, 88, 91, 12, 22, 47, 50, 54, 65, 66, 72, 0, 66, 67, 70, 82, 83, 98, 99};
char *path;

int main(int argc, char *argv[]) {
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

	setThreads(p);
	//    omp_set_nested(true);
	//    initlog2();

	//  printf("%d %d\n", p, d);
	//checkSimple(p,d);
	//checkScan(p, d);
	//checkRMQ(d);
	//checkLZ(d);
	//checkANSV(d);
	// checkSuffix(p, d);
	//checkCorrect();
	//printf("Path: %s\n", path);

	checkSourceFile(p, d, path);

	checkAll(p, d, sigma);

	//  for (int i = 0; i < 17; i++)
	//      printf("%d %d\n", (int)log2(i), fflog2(i));

	// int n = 10240;
	// for (int i = 24; i < n; i++) a[i] = 100;

	// //ParallelSortRS(a, n);

	// ParallelMergeSort(a, n, less<int>());

	// //for (int i = 0; i < n; i++) printf("%d ", a[i]); printf("\n");

	//  n = 10000000;
	//  cilk_for (int i = 0; i < n; i++) a[i] = rand() % n;
	// //  ParallelSortRS(a, n);
	// ParallelMergeSort(a, n, less<int>());


	return 0;
}


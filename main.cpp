#include <stdio.h>
#include <iostream>
#include "gettime.h"

#include "ANSV.h"

#include "rangeMin.h"

#include "sequence.h"
#include "intSort.h"
#include "utils.h"
#include "Base.h"
#include "merge.h"
#include "cilk.h"
#include <unistd.h>
#include <fcntl.h>

int LempelZiv(int *s, int n, int *LZ);
int ParallelLZ77(int *s, int n, int *lz);

int getLZ(int *lpf, int n, int *lz);
pair<int *, int *> suffixArray(int *s, int n, bool findLCPs);

char* itoa(int val, int base = 10){
	static char buf[32] = {0};
	int i = 30;
	for(; val && i ; --i, val /= base)
		buf[i] = "0123456789abcdef"[val % base];
	return &buf[i+1];	
}

void checkAll(int np, int n, int sigma) {
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
		//  setThreads(p);

		//  startTime();

		//  printf("#procs %d, #result %d\n", p, ParallelLZ77(a, n, lz));
		//  reportTime("parallel total time:");
		//  printf("\n");
		// }

		printf("***************** END OF TEST ON ALLPHABET SIZE = %d *****************\n\n", s);
	}

	delete lz; delete a;
	printf("ALL TEST DONE!\n");
}

int checkAll_wrapper(void *args) {
	long long *pt = (long long *) args;
	checkAll(pt[0], pt[1], pt[2]);
}

void getText(int n, char *path, int *dst) {
	char *buf = new char[n + 1];
	FILE *fptr = fopen(path, "r");
	int nn = fread(buf, 1, n, fptr);

	printf("size %d %d\n", n, nn);

	for (int i = 0; i < n; i++)
		dst[i] = ((unsigned int)buf[i]) % 127 + 1;

	fclose(fptr);
	delete buf;
}



void checkSourceFile(int np, int n, char *path) {
	//printf("%s\n", path);
	//flush();

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
	//  setThreads(p);

	//  startTime();

	//  printf("#procs %d, #result %d\n", p, ParallelLZ77(a, n, lz));
	//  reportTime("parallel total time:");
	//  printf("\n");
	// }

	printf("***************** END TEST LINX CODE *****************\n\n");

	delete lz; delete a;
	//printf("ALL TEST DONE!\n");
}

int checkSourceFile_wrapper(void *args) {
	long long *pt = (long long *) args;
	checkSourceFile(pt[0], pt[1], (char *)pt[2]);
}

extern "C++"
inline void Usage(char *program) {
	printf("Usage: %s [options]\n", program);
	printf("-p <num>\tNumber of processors to use\n");
	printf("-d <num>\t2^n of character will be processed\n");
	printf("-r <num>\tGenerete random string with the specified alphabet size\n");
	printf("-i <file>\tInput file name\n");
	printf("-o <file>\tOutput file name\n");
	printf("-h \t\tDisplay this help\n");
}

int main(int argc, char *argv[]) {
	int opt;
	int p = 1, d = -1, n = -1;
	int sigma = -1;
	char path[1025] = {};

	while ((opt = getopt(argc, argv, "p:d:r:i:o:")) != -1) {
		switch (opt) {
			case 'p': {
				p = atoi(optarg);
				break;
			}
			case 'd': {
				d = atoi(optarg);
				n = 1 << d;
				break;
			}
			case 'r': {
				sigma = atoi(optarg);
				break;
			}
			case 'i': {
				strncpy(path, optarg, 1024);
				// if (dup2(open(optarg, O_RDONLY), STDIN_FILENO) < 0) {
				// 	perror("Input file error");
				// 	exit(EXIT_FAILURE);
				// }
				break;
			}

			case 'o': {
				if (dup2(open(optarg, O_CREAT | O_WRONLY, 0644), STDOUT_FILENO) < 0) {
					perror("Output file error");
					exit(EXIT_FAILURE);
				}

				break;
			}
			default: {
				Usage(argv[0]);
				exit(1);
			}
		}
	}

	if (d < 0) {
		perror("Input file size not specified");
		exit(1);
	}

	if (sigma < 0 && path[0] == 0) {
		perror("No input file specified / Random string genereted.");
		exit(1);
	}

#ifdef CILK
	cilk::context ctx;
	ctx.set_worker_count(p);
#elif OPENMP
	omp_set_num_threads(p);
#endif

#ifdef CILK
	long long args[3];
	args[0] = p;
	args[1] = n;
	if (sigma > 1) {
		args[2] = sigma;
		ctx.run(checkAll_wrapper, (void *)args);
	} else {
		args[2] = (long long)path;
		ctx.run(checkSourceFile_wrapper, (void *)args);
	}
#else
	if (sigma > 1) {
		checkAll(p, n, sigma);
	} else {
		checkSourceFile(p, n, path);
	}
#endif
}


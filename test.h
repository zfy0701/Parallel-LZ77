#ifndef _TEST_H
#define _TEST_H

#include "Base.h"
#include "parallel.h"
#include "utils.h"
#include "gettime.h"

inline int get_file_size(char * path) {
	struct stat info;
	stat(path, &info);
	return info.st_size;
}

inline void readText(int *a, int n, char *path) {
	char *buf = new char[n + 1];
	FILE *fptr = fopen(path, "r");
	int nn = fread(buf, 1, n, fptr);

	for (int i = 0; i < n; i++)
		a[i] = ((unsigned int)buf[i]) % 127 + 1;

	fclose(fptr);
	delete buf;
}

inline void generateText(int *a, int n, int sigma)  {
	srand(time(NULL));
	for (int i = 0; i < n; i++)
		a[i] = rand() % sigma + 1;
}

inline void Usage(char *program) {
	printf("Usage: %s [options]\n", program);
	printf("-p <num>\tNumber of processors to use\n");
	printf("-d <num>\t2^n of character will be processed\n");
	printf("-r <num>\tGenerete random string with the specified alphabet size\n");
	printf("-i <file>\tInput file name\n");
	printf("-o <file>\tOutput file name\n");
	printf("-f <file>\tChoose different algorithm for LPF\n");
	printf("-h \t\tDisplay this help\n");
}

inline int test_main(int argc, char *argv[], char * algoname, std::pair<int *, int> lz77(int *s, int n)) {
	int opt;
	int p = 1, d = -1, n = -1;
	int sigma = -1;
	char path[1025] = {};

	while ((opt = getopt(argc, argv, "p:d:r:i:o:f:")) != -1) {
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
				 if (dup2(open(optarg, O_RDONLY), STDIN_FILENO) < 0) {
				 	perror("Input file error");
				 	exit(EXIT_FAILURE);
				}
				break;
			}
			case 'o': {
				if (dup2(open(optarg, O_CREAT | O_WRONLY, 0644), STDOUT_FILENO) < 0) {
					perror("Output file error");
					exit(EXIT_FAILURE);
				}

				break;
			}
			case 'f': {
				//flag = atoi(optarg);
				//this should be done in the caller
				break;
			}
			default: {
				Usage(argv[0]);
				exit(1);
			}
		}
	}

	if (sigma < 0 && path[0] == 0) {
		perror("No input file specified / Random string genereted.");
		exit(1);
	}

	if (d < 0) {
		if (sigma < 0) {
	    	n = get_file_size(path);
	    } else {
			perror("Random data size not specified");
	    	exit(1);
		}
	}

	set_threads(p);

	printf("***************** TEST BEGIN *****************\n");

	int *a = new int[n + 3];

	if (sigma >= 1) {
		printf(" * Data generated randomly with alphabet size: %d.\n", sigma);
		generateText(a, n, sigma);
	} else {
		printf(" * Data from file: %s\n", path);
		readText(a, n, path);
	}

	printf(" * Data size: %d\n", n);
	printf(" * Algorithm: %s\n", algoname);
	printf(" * Threads num: %d\n", p);

	timer testTm;
	a[n] = a[n + 1] = a[n + 2] = 0;
	testTm.start();
	std::pair<int *, int> res = lz77(a, n);
	int maxoffset = 0;
	for (int i = 0; i < res.second - 1; i++) {
		maxoffset = std::max(maxoffset, res.first[i+1] - res.first[i]);
	}
	printf(" * result: size = %d, max offset = %d\n", res.second, maxoffset);
	testTm.reportNext(" * Total time:");
	printf("***************** TEST ENDED *****************\n\n");

	delete a;
	return 0;
}

#endif
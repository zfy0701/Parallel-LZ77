/*
 * The sequential algirthm for Lempel-ziv 77 compression
 */

#include <iostream>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "cilk.h"

#include "gettime.h"

using namespace std;
pair<int*,int*> suffixArray(int* s, int n, bool findLCPs);

void calNearestElement(int a[], int n, int leftElements[], int rightElements[]) {
    int i, top;
    int *stack = new int[n];

    for (i = 0, top = -1; i < n; i++) {
        while (top > -1 && a[stack[top]] > a[i]) top--;
        if (top == -1) leftElements[i] = -1;
        else leftElements[i] = stack[top];
        stack[++top] = i;
    }

    for (i = n - 1, top = -1; i >= 0; i--) {
        while (top > -1 && a[stack[top]] > a[i]) top--;
        if (top == -1) rightElements[i] = -1;
        else rightElements[i] = stack[top];
        stack[++top] = i;
    }
    delete stack;
}

int LempelZiv(int *s, int n, int *LZ) {
    int i, lpf, l, r;
    int *leftElements = new int[n], * rightElements = new int[n];
    startTime();

    pair<int*,int*> res = suffixArray(s,n,0);
    int* SA = res.first;
    nextTime("seq suffix array");

    calNearestElement(SA, n, leftElements, rightElements);

    nextTime("seq ANSV");

    int *Rank = new int[n];
    for (i = 0; i < n; i++) {
        Rank[SA[i]] = i;
    }

    int k = 0;
    for (k = i = 0; i < n; i += max(1, lpf)) {
        int left = leftElements[Rank[i]], right = rightElements[Rank[i]];

        l = r = 0;

        if (left != -1) while (s[SA[left] + l] == s[i + l])
                l++;
        if (right != -1) while (s[SA[right] + r] == s[i + r])
                r++;

        LZ[k++] = i;
        lpf = max(l, r);
    }
    //printf("%d\n", k);
    //for (i = 0;i < k;i++) printf("%d ", LZ[i] + 1);
    nextTime("seq lpf");
    delete SA;
    delete Rank;
    delete leftElements;
    delete rightElements;
    return k;
}

int LempelZiv2(int *s, int n, int *LZ) {
    int i, lpf, l, r;
    int *leftElements = new int[n], * rightElements = new int[n];
    int *leftLPF = new int[n], *rightLPF = new int[n], *LPF = new int[n];
    //startTime();

    pair<int*,int*> res = suffixArray(s,n,0);
    int* SA = res.first;
    nextTime("seq suffix array");

    calNearestElement(SA, n, leftElements, rightElements);

    nextTime("seq ANSV");

    int *Rank = new int[n];
    for (i = 0; i < n; i++) {
        Rank[SA[i]] = i;
    }

    LPF[0] = leftLPF[0] = rightLPF[0] = 0;
    for (int i = 1; i < n; i++) {
        int left = leftElements[Rank[i]], right = rightElements[Rank[i]];
        int l = max(leftLPF[i - 1] - 1, 0), r = max(rightLPF[i - 1] - 1, 0);
        //l = r = 0;

        if (left != -1) {
            while (s[SA[left] + l] == s[i + l]) l++;
            leftLPF[i] = l;
        } else leftLPF[i] = 0;

        if (right != -1) {
             while (s[SA[right] + r] == s[i + r]) r++;
             rightLPF[i] = r;
        } else rightLPF[i] = 0;

        LPF[i] = max(l,r);
   }

   // for (int i = 0; i < n; i++) {
   //      printf("[%d %d] ", leftLPF[i], rightLPF[i]);
   // }

    int k = 0;
    for (k = i = 0; i < n; i += max(1, lpf)) {
        lpf = LPF[i];
        k++;
    }
    //printf("%d\n", k);
    //for (i = 0;i < k;i++) printf("%d ", LZ[i] + 1);
    nextTime("seq lpf");
    delete SA;
    delete Rank;
    delete leftElements;
    delete rightElements;
    return k;
}

void checkAll(int np, int n, int sigma) {
    int *a = new int[n + 3];
    int i;
    int *lz = new int[n];

    printf("***************** TEST ON APLPHABET SIZE = %d *****************\n", sigma);
    for ( i = 0; i < n; i++)
        a[i] = rand() % sigma + 1;
    a[n] = a[n + 1] = a[n + 2] = 0;

    startTime();

    printf("#result1 %d\n", LempelZiv(a, n, lz));

    reportTime("lz77 1:");

        startTime();
    printf("#result2 %d\n", LempelZiv2(a, n, lz));
    reportTime("lz77 2:");
    printf("\nParallel:\n");

    printf("***************** END OF TEST ON ALLPHABET SIZE = %d *****************\n\n", sigma);

    delete lz; delete a;
    printf("ALL TEST DONE!\n");
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
    int *a = new int[n + 3];
    int i;
    int *lz = new int[n];

    printf("***************** TEST ON LINX CODE *****************\n");

    getText(n, path, a);
    a[n] = a[n + 1] = a[n + 2] = 0;

    startTime();
    printf("\nParallel:\n");
    printf("#result %d\n", LempelZiv(a, n, lz));
    reportTime("parallel total time:");

    printf("***************** END TEST LINX CODE *****************\n\n");

    delete lz; delete a;
}

void Usage(char *program) {
    printf("Usage: %s [options]\n", program);
    printf("-d <num>\t2^n of character will be processed\n");
    printf("-r <num>\tGenerete random string with the specified alphabet size\n");
    printf("-i <file>\tInput file name\n");
    printf("-o <file>\tOutput file name\n");
    printf("-h \t\tDisplay this help\n");
}

int main(int argc, char *argv[]) {
    int opt;
    int d = -1, n = -1;
    int sigma = -1;
    char path[1025] = {};

    while ((opt = getopt(argc, argv, "p:d:r:i:o:")) != -1) {
        switch (opt) {
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
            default: {
                Usage(argv[0]);
                exit(1);
            }
        }
    }


                #ifdef CILK
                __cilkrts_set_param("nworkers", "1");
                #elif OPENMP
                omp_set_num_threads(1);
                #endif

    if (sigma < 0 && path[0] == 0) {
        perror("No input file specified / Random string genereted.");
        exit(1);
    }

    if (d < 0) {
        if (sigma < 0) {
            struct stat info;
            stat(path, &info);
            n = info.st_size;
        } else {
            perror("Random data size not specified");
            exit(1);
        }
    }

    if (sigma > 1) {
        checkAll(1, n, sigma);
    } else {
        checkSourceFile(1, n, path);
    }
}



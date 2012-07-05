/*
 * The sequential algirthm for Lempel-ziv 77 compression
 */

#include <iostream>
#include <cstdio>
#include <cstring>
#include "test.h"
#include "ANSV.h"
#include "suffixArray.h"
 
using namespace std;

pair<int*,int> LempelZiv(int *s, int n) {
    int i, lpf, l, r;
    int *leftElements = new int[n], * rightElements = new int[n];
    startTime();

    pair<int*,int*> res = suffixArray(s,n,0);
    int* SA = res.first;
    nextTime("\tsuffix array");

    ComputeANSV_Linear(SA, n, leftElements, rightElements);

    nextTime("\tANSV");

    int *Rank = new int[n];
    for (i = 0; i < n; i++) {
        Rank[SA[i]] = i;
    }

    int k = 0;
    int * LZ = Rank;
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
    nextTime("\tlpf");
    delete SA;
    delete leftElements;
    delete rightElements;

    return make_pair(LZ, k);
}

int LempelZiv2(int *s, int n, int *LZ) {
    int i, lpf, l, r;
    int *leftElements = new int[n], * rightElements = new int[n];
    int *leftLPF = new int[n], *rightLPF = new int[n], *LPF = new int[n];
    //startTime();

    pair<int*,int*> res = suffixArray(s,n,0);
    int* SA = res.first;
    nextTime("\tsuffix array");

    ComputeANSV_Linear(SA, n, leftElements, rightElements);

    nextTime("\tANSV");

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


    int k = 0;
    for (k = i = 0; i < n; i += max(1, lpf)) {
        lpf = LPF[i];
        k++;
    }
    //printf("%d\n", k);
    //for (i = 0;i < k;i++) printf("%d ", LZ[i] + 1);
    nextTime("\tlpf");
    delete SA;
    delete Rank;
    delete leftElements;
    delete rightElements;
    return k;
}

int main(int argc, char *argv[]) {
    return test_main(argc, argv, (char *)"Seq LZ77 with ANSV", LempelZiv);
}

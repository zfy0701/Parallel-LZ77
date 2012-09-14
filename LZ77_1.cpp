/*
 * The sequential algorithm for Lempel-ziv 77 compression
 */

#include <iostream>
#include <cstdio>
#include <cstring>
#include "test.h"
#include "ANSV.h"
#include "suffixArray.h"
using namespace std;

pair<int *, int> LempelZiv(int *s, int n) {
    timer lzTm;
    lzTm.start();

    int i, lpf, l, r;
    int *leftElements = new int[n], * rightElements = new int[n];

    pair<int *, int *> res = suffixArray(s, n, 0);

    int *SA = res.first;
    lzTm.reportNext("\tsuffix array");

    ComputeANSV_Linear(SA, n, leftElements, rightElements);

    lzTm.reportNext("\tANSV");

    int *Rank = new int[n];
    for (i = 0; i < n; i++) {
        Rank[SA[i]] = i;
    }

    int k = 0;
    int *LZ = Rank;
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
    delete SA;
    delete leftElements;
    delete rightElements;
    lzTm.reportNext("\tlpf");
    return make_pair(LZ, k);
}

int main(int argc, char *argv[]) {
    return test_main(argc, argv, (char *)"Seq LZ77 with ANSV", LempelZiv);
}

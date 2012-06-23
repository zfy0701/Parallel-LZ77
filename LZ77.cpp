/*
 * The sequential algirthm for Lempel-ziv 77 compression
 */

#include <iostream>
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
    
    pair<int*,int*> res = suffixArray(s,n,0);
    int* SA = res.first;
    //nextTime("seq suffix array");

    calNearestElement(SA, n, leftElements, rightElements);

    //nextTime("seq ANSV");

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
    //nextTime("seq lpf");
    delete SA;
    delete Rank;
    delete leftElements;
    delete rightElements;
    return k;
}

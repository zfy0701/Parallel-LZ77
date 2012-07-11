/*
 * The sequential algirthm for Lempel-ziv 77 compression
 */

#include <iostream>
#include <cstdio>
#include <cstring>
#include "Base.h"
#include "test.h"

using namespace std;

const int WINDOW_SIZE = 4096; 

pair<int*,int> LempelZiv(int *s, int n) {
    int k  = 0;
    int * LZ = new int[n];
    return make_pair(LZ, k);
}

int main(int argc, char *argv[]) {
    return test_main(argc, argv, (char *)"Seq LZ77 with ANSV", LempelZiv);
}

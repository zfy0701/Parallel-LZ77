#ifndef _SUFFIX_ARRAY_
#define _SUFFIX_ARRAY_

std::pair<int*,int*> suffixArray(int* s, int n, bool findLCPs);
int* suffixArrayNoLCP(int* s, int n);
int *GetLCP(int * s, int n, int * SA);

#endif
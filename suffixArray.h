#ifndef _SUFFIX_ARRAY_
#define _SUFFIX_ARRAY_
#include "parallel.h"
std::pair<intT*,intT*> suffixArray(intT* s, intT n, bool findLCPs);
intT* suffixArrayNoLCP(intT* s, intT n);
intT *GetLCP(intT * s, intT n, intT * SA);

#endif

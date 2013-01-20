#ifndef _ANSV_H
#define _ANSV_H

#include "parallel.h"
void ComputeANSV(intT * a, intT n, intT *left, intT *right);
void ComputeANSV_Linear(intT a[], intT n, intT leftElements[], intT rightElements[], intT offset = 0);

#endif

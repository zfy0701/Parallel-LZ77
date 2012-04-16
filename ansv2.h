#ifndef _ANSV_hpp_
#define _ANSV_hpp_

#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <iostream>
#include "cilk.h"
using namespace std;

typedef unsigned int uint;
class ANSV{
public:
  ANSV(int* a, int n);  
  pair<uint,uint>** computeTree();
  int* computeANSVleft();
  int* computeANSVright();
  int* getLeftNeighbors();
  int* getRightNeighbors();
  ~ANSV();
protected:
  int *a;
  int n;
  pair<uint,uint>** tree;
  int* leftNeighbors;
  int* rightNeighbors;
  int depth;
};
#endif

#include "ANSV.h"
#include "Base.h"
#include "parallel.h"

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <iostream>

using namespace std;

#define LEFT(i) ((i) << 1)
#define RIGHT(i) (((i) << 1) | 1)
#define PARENT(i) ((i) >> 1)

const int BLOCK_SIZE = 8192;

inline intT getLeft_opt(intT **table, intT depth, intT n, intT index, intT start) {
  intT value = table[0][index];
  if (value == table[depth - 1][0]) return -1;

  intT cur = PARENT(start), d, dist = 2;
  for (d = 1; d < depth; d++) {
    if ((cur + 1) * dist > index + 1) cur --;
    if (cur < 0) return -1;

    if (table[d][cur] >= value) cur = PARENT(cur);
    else break;

    dist <<= 1;
  }

  for ( ; d > 0; d--) {
    if (table[d - 1][RIGHT(cur)] < value) cur = RIGHT(cur);
    else cur = LEFT(cur);
  }
  return cur;
}

inline intT getRight_opt(intT **table, intT depth, intT n, intT index, intT start) {
  intT value = table[0][index];
  if (value == table[depth - 1][0]) return -1;

  intT cur = PARENT(start), d, dist = 2;
  for (d = 1; d < depth; d++) {
    if (cur * dist < index) cur ++;
    if (cur * dist >= n) return -1;

    if (table[d][cur] >= value) cur = PARENT(cur);
    else break;

    dist <<= 1;
  }

  for ( ; d > 0; d--) {
    if (table[d - 1][LEFT(cur)] < value) cur = LEFT(cur);
    else cur = RIGHT(cur);
  }
  return cur;
}


void ComputeANSV_Linear(intT a[], intT n, intT leftElements[], intT rightElements[], intT offset) {
  intT i, top;
  intT *stack = new intT[n];

  for (i = 0, top = -1; i < n; i++) {
    while (top > -1 && a[stack[top]] > a[i]) top--;
    if (top == -1) leftElements[i] = -1;
    else leftElements[i] = stack[top] + offset;
    stack[++top] = i;
  }

  for (i = n - 1, top = -1; i >= 0; i--) {
    while (top > -1 && a[stack[top]] > a[i]) top--;
    if (top == -1) rightElements[i] = -1;
    else rightElements[i] = stack[top] + offset;
    stack[++top] = i;
  }
  delete stack;
}

void ComputeANSV(intT *a, intT n, intT *left, intT *right) {
  intT l2 = cflog2(n);
  intT depth = l2 + 1;
  intT **table = new intT*[depth];

  table[0] = a;
  intT m = n;
  for (intT i = 1; i < depth; i++) {
    m = (m + 1) / 2;
    table[i] = new intT[m];
  }

  m = n;
  for (intT d = 1; d < depth; d++) {
    intT m2 = m / 2;

    parallel_for (intT i = 0; i < m2; i++) {
      table[d][i] = min(table[d - 1][LEFT(i)], table[d - 1][RIGHT(i)]);
    }

    if (m % 2) {
      table[d][m2] = table[d - 1][LEFT(m2)];
    }

    m = (m + 1) / 2;
  }

  parallel_for (intT i = 0; i < n; i += BLOCK_SIZE) {
    intT j = min(i + BLOCK_SIZE, n);
    ComputeANSV_Linear(a + i, j - i, left + i, right + i, i);

    intT tmp = i;
    for (intT k = i; k < j; k++) {
      if (left[k] == -1) {
        if ( tmp != -1 && a[tmp] >= a[k]) {
          tmp = getLeft_opt(table, depth, n, k, tmp);
        }
        left[k] = tmp;
      }
    }

    tmp = j - 1;
    for (intT k = j - 1; k >=  i; k--) {
      if (right[k] == -1) {
        if (tmp != -1 && a[tmp] >= a[k]) {
          tmp = getRight_opt(table, depth, n, k, tmp);
        }
        right[k] = tmp;
      }
    }
  }

  for (intT i = 1; i < depth; i++) delete table[i];
  delete table;
}

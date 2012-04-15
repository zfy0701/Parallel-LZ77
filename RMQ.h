#ifndef _RMQ_H
#define _RMQ_H


void buildRMQ(int * a, int n, int **table);

int queryRMQ(int **table, int i, int j);

#endif
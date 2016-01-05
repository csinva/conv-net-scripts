#ifndef TESTLIB_H
#define TESTLIB_H

#include <iostream>
#include <list>

std::list<int> connectedComponentsCPP(double * conn, double * nhood, int dimX, int dimY, int dimZ, double * outputComp, std::list<int> * l);
int eval_c(int dx,int dy, int dz, int dcons, uint32_t * gt, float * affs,std::list<int> * threshes);

#endif
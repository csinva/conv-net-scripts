#ifndef TESTLIB_H
#define TESTLIB_H

#include <iostream>
#include <list>
#include <string>

int eval_c(int dx,int dy, int dz, int dcons, uint32_t * gt, float * affs,std::list<int> * threshes, std::list<std::string> * funcs,int save_seg, std::string* out);

#endif
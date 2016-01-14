#ifndef TESTLIB_H
#define TESTLIB_H

#include <iostream>
#include <list>
#include <string>
#include <map>
#include <vector>

std::map<std::string,std::vector<double>> eval_c(int dx,int dy, int dz, int dcons, uint32_t * gt, float * affs,std::list<int> * threshes, std::list<std::string> * funcs,std::list<int> * save_threshes, std::string* out);

#endif
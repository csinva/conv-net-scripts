#ifndef TESTLIB_H
#define TESTLIB_H

#include <iostream>
#include <list>
#include <string>
#include <map>
#include <vector>
#include <utility>

std::map<std::string,std::vector<double>> eval_c(int dx,int dy, int dz, int dcons, uint32_t * gt, float * affs,std::list<int> * threshes, std::list<std::string> * funcs,std::list<int> * save_threshes, std::string* out);

std::map<std::string,std::vector<double>> oneThresh(int dx,int dy, int dz, int dcons, uint32_t * gt, float * affs,int thresh,int eval);

std::map<std::string,std::vector<double>> oneThresh_no_gt(int dimX, int dimY, int dimZ, int dcons, float* affs, int thresh,int eval);

double* calc_region_graph(int dimX, int dimY, int dimZ, int dcons, uint32_t* gt, float* affs,std::list<int> * threshes, std::list<int> * save_threshes);

#endif

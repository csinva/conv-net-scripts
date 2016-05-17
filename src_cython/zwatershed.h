#ifndef TESTLIB_H
#define TESTLIB_H

#include <iostream>
#include <list>
#include <string>
#include <map>
#include <vector>
#include <utility>

std::map<std::string,std::vector<double>> oneThresh(int dx,int dy, int dz, int dcons, uint32_t * gt, float * affs,int thresh,int eval);

std::map<std::string,std::vector<double>> oneThresh_no_gt(int dimX, int dimY, int dimZ, int dcons, float* affs, int thresh,int eval);

std::list<double> calc_region_graph(int dimX, int dimY, int dimZ, int dcons, uint32_t* gt, float* affs);

#endif

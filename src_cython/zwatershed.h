#ifndef TESTLIB_H
#define TESTLIB_H

#include <iostream>
#include <list>
#include <string>
#include <map>
#include <vector>
#include <utility>

std::list<float> calc_region_graph(int dx, int dy, int dz, int dcons, uint32_t* gt, float* affs);

std::map<std::string,std::vector<double>> oneThresh(int dx,int dy, int dz, int dcons, uint32_t * gt, float * affs, float * rgn_graph, int rgn_graph_len, int thresh,int eval);

std::map<std::string,std::vector<double>> oneThresh_no_gt(int dimX, int dimY, int dimZ, int dcons, float* affs, int thresh,int eval);

#endif

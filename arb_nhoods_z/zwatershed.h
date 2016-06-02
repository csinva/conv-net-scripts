#ifndef TESTLIB_H
#define TESTLIB_H

#include <iostream>
#include <list>
#include <string>
#include <map>
#include <vector>
#include <utility>

std::map<std::string,std::list<float>> zwshed_initial_c_arb(int dx, int dy, int dz, uint32_t*node1,
                                               uint32_t*node2, float*edgeWeight, int n_edge);

std::map<std::string,std::vector<double>> merge_with_stats_arb(int dx,int dy, int dz, uint32_t * gt, float * rgn_graph,
                                        int rgn_graph_len, uint32_t * seg_in, uint32_t*counts, int counts_len, int thresh);

std::map<std::string,std::vector<double>> merge_no_stats_arb(int dx,int dy, int dz, float * rgn_graph,
                                        int rgn_graph_len, uint32_t * seg_in, uint32_t*counts, int counts_len, int thresh);


#endif
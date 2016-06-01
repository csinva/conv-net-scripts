/* Connected components
 * developed and maintained by Srinivas C. Turaga <sturaga@mit.edu>
 * do not distribute without permission.
 */
#include "zwatershed.h"
//#pragma once
#include "zwatershed_util/agglomeration.hpp"
#include "zwatershed_util/region_graph.hpp"
#include "zwatershed_util/basic_watershed.hpp"
#include "zwatershed_util/limit_functions.hpp"
#include "zwatershed_util/types.hpp"
#include "zwatershed_util/main_helper.hpp"


#include <memory>
#include <type_traits>

#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstddef>
#include <cstdint>
#include <queue>
#include <vector>
#include <algorithm>
#include <tuple>
#include <map>
#include <list>
#include <set>
#include <vector>
#include <chrono>
#include <fstream>
#include <string>
#include <boost/make_shared.hpp>
using namespace std;
// these values based on 5% at iter = 10000
double LOW=  .0001;
double HIGH= .9999;
bool RECREATE_RG = true;

std::map<std::string,std::list<float>> calc_region_graph(int dimX, int dimY, int dimZ, uint32_t*node1,
                                               uint32_t*node2, float*edgeWeight, int n_edge){
    // read data
    std::cout << "\ncalculating basic watershed..." << std::endl;
    volume_ptr<uint32_t> seg_ref;
    std::vector<std::size_t> counts_ref;
    std::tie(seg_ref , counts_ref) = watershed<uint32_t>(dimX,dimY,dimZ,node1, node2, edgeWeight, n_edge, LOW, HIGH);
    auto seg = *seg_ref;

    // calculate region graph
    std::cout << "\ncalculating rgn graph..." << std::endl;
    auto rg = get_region_graph(node1, node2, edgeWeight, n_edge, seg_ref , counts_ref.size()-1);

    // save and return
    std::map<std::string,std::list<float>> returnMap;

    std::list<float> rg_data = * (new std::list<float>());
    for ( const auto& e: *rg ){
        rg_data.push_back(std::get<1>(e));
        rg_data.push_back(std::get<2>(e));
        rg_data.push_back(std::get<0>(e));
    }
    std::list<float> seg_data = * (new std::list<float>());
    std::list<float> counts_data = * (new std::list<float>());
    for(int i=0;i<dimX*dimY*dimZ;i++){
        seg_data.push_back(seg_ref->data()[i]);
    }
    for (const auto& x:counts_ref){
        counts_data.push_back(x);
    }
    returnMap["rg"]=rg_data;
    returnMap["seg"]=seg_data;
    returnMap["counts"]=counts_data;

    return returnMap;
 }


std::map<std::string,std::vector<double>> oneThresh_with_stats(int dimX,int dimY, int dimZ, uint32_t * gt, float * rgn_graph,
int rgn_graph_len, uint32_t * seg_in, uint32_t*counts_in, int counts_len, int thresh){

    //read data
    volume_ptr<uint32_t> gt_ptr(new volume<uint32_t> (boost::extents[dimX][dimY][dimZ], boost::fortran_storage_order()));
    volume_ptr<uint32_t> seg(new volume<uint32_t> (boost::extents[dimX][dimY][dimZ]));
    std::vector<std::size_t> counts = * new std::vector<std::size_t>();
    region_graph_ptr<uint32_t,float> rg( new region_graph<uint32_t,float> );
    for(int i=0;i<dimX*dimY*dimZ;i++){
        gt_ptr->data()[i] = gt[i];
        seg->data()[i] = seg_in[i];
    }
    for(int i=0;i<counts_len;i++)
        counts.push_back(counts_in[i]);
    for(int i=0;i<rgn_graph_len;i++)
        (*rg).emplace_back(rgn_graph[i*3+2],rgn_graph[i*3],rgn_graph[i*3+1]);

    // merge
    std::cout << "thresh: " << thresh << "\n";
	merge_segments_with_function(seg, rg, counts, square(thresh), 10,RECREATE_RG);

    // save
    std::map<std::string,std::vector<double>> returnMap;
    std::vector<double> seg_vector;
    std::vector<double> r;
    std::vector<double> rg_data; // = * (new std::list<float>());
    std::vector<double> counts_data; // = * (new std::list<float>());
    for(int i=0;i<dimX*dimY*dimZ;i++)
        seg_vector.push_back(((double)(seg->data()[i])));
    auto x = compare_volumes_arb(*gt_ptr, *seg, dimX,dimY,dimZ);
    r.push_back(x.first);
    r.push_back(x.second);
    returnMap["stats"] = r;

    for ( const auto& e: *rg ){
        rg_data.push_back(std::get<1>(e));
        rg_data.push_back(std::get<2>(e));
        rg_data.push_back(std::get<0>(e));
    }
    for (const auto& x:counts)
        counts_data.push_back(x);
    returnMap["seg"] = seg_vector;
    returnMap["rg"]=rg_data;
    returnMap["counts"] = counts_data;
    return returnMap;
}
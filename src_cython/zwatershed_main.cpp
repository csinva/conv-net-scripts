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
#include "zwatershed_util/utils.hpp"
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
double LOW=  .0001;// 0.003785; //.00001; //default = .3
double HIGH= .9999;// 0.999971; //.99988; //default = .99
bool RECREATE_RG = false;



std::map<std::string,std::list<float>> calc_region_graph(int dimX, int dimY, int dimZ, int dcons, uint32_t* seg,
float* affs)
{
    std::cout << "calculating rgn graph..." << std::endl;

    volume_ptr<uint32_t> seg_ref = read_volumes<uint32_t>("", dimX, dimY, dimZ);


    int totalDim = dimX*dimY*dimZ;
    for(int i=0;i<totalDim;i++){
        seg_ref->data()[i] = seg[i];
    }

    totalDim*=dcons;
    affinity_graph_ptr<float> aff = read_affinity_graphe<float>("", dimX, dimY, dimZ, dcons);
    for(int i=0;i<totalDim;i++){
        aff->data()[i] = affs[i];
    }

    std::vector<std::size_t> counts_ref;
    std::tie(seg_ref , counts_ref) = watershed<uint32_t>(aff, LOW, HIGH);
    auto rg = get_region_graph(aff, seg_ref , counts_ref.size()-1);

     std::list<float> data = * (new std::list<float>());

    for ( const auto& e: *rg )
    {
        data.push_back(std::get<1>(e));
        data.push_back(std::get<2>(e));
        data.push_back(std::get<0>(e));
    }

    std::map<std::string,std::list<float>> returnMap;

    returnMap["rg"]=data;
    return returnMap;
    //return data;

 }


std::map<std::string,std::vector<double>> oneThresh(int dimX, int dimY, int dimZ, int dcons, uint32_t* gt,
float* affs, float* rgn_graph, int rgn_graph_len, int thresh,int eval)
{
    std::cout << "evaluating..." << std::endl;

    volume_ptr<uint32_t> gt_ptr = read_volumes<uint32_t>("", dimX, dimY, dimZ);

    int totalDim = dimX*dimY*dimZ;
    for(int i=0;i<totalDim;i++){
        gt_ptr->data()[i] = gt[i];
    }

    affinity_graph_ptr<float> aff = read_affinity_graphe<float>("", dimX, dimY, dimZ, dcons);
    totalDim*=dcons;
    for(int i=0;i<totalDim;i++){
        aff->data()[i] = affs[i];
    }

    std::cout << "dims:  " << aff->shape()[0] << " " << aff->shape()[1] << " " << aff->shape()[2] << " " << aff->shape()[3] << "\n";

    volume_ptr<uint32_t>     seg_ref   ;
    std::vector<std::size_t> counts_ref;

    std::tie(seg_ref , counts_ref) = watershed<uint32_t>(aff, LOW, HIGH);

    // construct rg from rgn_graph
    region_graph_ptr<uint32_t,float> rg( new region_graph<uint32_t,float> );
    region_graph<uint32_t,float>& rg_temp = *rg;
    for(int i=0;i<rgn_graph_len;i++){
        int i3 = i*3;
        rg_temp.emplace_back(rgn_graph[i3+2],rgn_graph[i3],rgn_graph[i3+1]);
    }

    volume_ptr<uint32_t> seg   ;
    std::map<std::string,std::vector<double>> returnMap;

 std::vector<double> r;

    int thold = thresh;

     std::cout << "THOLD: " << thold << "\n";
	 seg.reset(new volume<uint32_t>(*seg_ref));
	 std::vector<std::size_t> counts(counts_ref);
	 merge_segments_with_function(seg, rg, counts, square(thold), 10,RECREATE_RG);
	    //copy seg to a 1d vector and return it
	    std::vector<double> seg_vector;
	    for(int i=0;i<dimX*dimY*dimZ;i++)
		seg_vector.push_back(((double)(seg->data()[i])));
	    std::cout << "seg_vector: ";// << seg_vector;
	returnMap["seg"] = seg_vector; 
	//}
	if(eval==1){	 
		auto x = compare_volumes_arb(*gt_ptr, *seg, dimX,dimY,dimZ);
		 r.push_back(x.first);
		 r.push_back(x.second);

		returnMap["stats"] = r;
	}
     return returnMap;

 }
std::map<std::string,std::vector<double>> oneThresh_no_gt(int dimX, int dimY, int dimZ, int dcons, float* affs, int thresh,int eval)
{
    std::cout << "evaluating..." << std::endl;

    int totalDim = dimX*dimY*dimZ;

    affinity_graph_ptr<float> aff = read_affinity_graphe<float>("", dimX, dimY, dimZ, dcons);

    totalDim*=dcons;
    for(int i=0;i<totalDim;i++){
        aff->data()[i] = affs[i];
    }

    std::cout << "dims:  " << aff->shape()[0] << " " << aff->shape()[1] << " " << aff->shape()[2] << " " << aff->shape()[3] << "\n";

    std::list<int>::const_iterator iterator;
    //std::list<int> thresh_list = *threshes;

    std::map<std::string,std::vector<double>> returnMap;
    volume_ptr<uint32_t>     seg_ref   ;
    std::vector<std::size_t> counts_ref;
    std::tie(seg_ref , counts_ref) = watershed<uint32_t>(aff, LOW, HIGH);
    auto rg = get_region_graph(aff, seg_ref , counts_ref.size()-1);
    volume_ptr<uint32_t>     seg   ;

 std::vector<double> r;

 int thold = thresh;

     std::cout << "THOLD: " << thold << "\n";
	 seg.reset(new volume<uint32_t>(*seg_ref));
	 std::vector<std::size_t> counts(counts_ref);
	 merge_segments_with_function(seg, rg, counts, square(thold), 10,RECREATE_RG);
	    std::vector<double> seg_vector;
	    for(int i=0;i<dimX*dimY*dimZ;i++)
		seg_vector.push_back(((double)(seg->data()[i])));
	returnMap["seg"] = seg_vector;
	//}
     return returnMap;

 }

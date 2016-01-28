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

template< typename ID, typename F >
inline region_graph_ptr<ID,F>
get_merge_tree( const region_graph<ID,F>& rg, std::size_t max_segid )
{
    zi::disjoint_sets<ID>      sets(max_segid+1);
    std::vector<std::list<ID>> edges(max_segid+1);
    region_graph_ptr<ID,F>     mt_ptr( new region_graph<ID,F> );

    for ( const auto& e: rg )
    {
        ID v1 = std::get<1>(e);
        ID v2 = std::get<2>(e);

        ID s1 = sets.find_set(v1);
        ID s2 = sets.find_set(v2);

        if ( s1 != s2 && s1 && s2 )
        {
            mt_ptr->push_back(e);
            sets.join(s1, s2);

            edges[v1].push_back(v2);
            edges[v2].push_back(v1);

        }
    }

    std::vector<ID> order(max_segid+1);
    ID curr = 0;

    for ( ID i = 0; i <= max_segid; ++i )
    {
        if ( order[i] == 0 )
        {
            std::deque<ID> queue;
            queue.push_back(i);
            order[i] = ++curr;

            while ( queue.size() )
            {
                ID x = queue.front();
                queue.pop_front();

                for ( auto& y: edges[x] )
                {
                    if ( order[y] == 0 )
                    {
                        order[y] = ++curr;
                        queue.push_back(y);
                    }
                }
            }
        }
    }

    for ( auto& e: *mt_ptr )
    {
        if ( order[std::get<2>(e)] < order[std::get<1>(e)] )
        {
            std::swap(std::get<2>(e), std::get<1>(e));
        }
    }

    return mt_ptr;
}



template< typename ID,
          typename F,
          typename L,
          class = typename std::enable_if<is_numeric<F>::value>::type,
          class = typename std::enable_if<std::is_integral<ID>::value>::type,
          class = typename std::enable_if<std::is_convertible<L,F>::value>::type >
inline void yet_another_watershed( const volume_ptr<ID>& seg_ptr,
                                   const region_graph_ptr<ID,F> rg_ptr,
                                   std::vector<std::size_t>& counts,
                                   const L& lowl)
{
    F low = static_cast<F>(lowl);

    std::vector<std::size_t> new_counts({0});
    std::vector<ID>          remaps(counts.size());

    zi::disjoint_sets<ID>    sets(counts.size());
    std::vector<F>           maxs(counts.size());


    region_graph<ID,F>& rg  = *rg_ptr;

    ID next_id = 1;
    ID merged = 0;


    for ( auto& it: rg )
    {
        if ( std::get<0>(it) <= low )
        {
            break;
        }

        ID s1 = std::get<1>(it);
        ID s2 = std::get<2>(it);

        F f = std::get<0>(it);

        if ( s1 && s2 )
        {
            if ( (remaps[s1] == 0) || (remaps[s2] == 0) )
            {
                if ( remaps[s1] == 0 )
                {
                    std::swap(s1,s2);
                }

                if ( remaps[s1] == 0 )
                {
                    maxs[next_id] = f;
                    remaps[s1] = remaps[s2] = next_id;
                    new_counts.push_back(counts[s1]+counts[s2]);
                    ++next_id;
                }
                else
                {
                    ID actual = sets.find_set(remaps[s1]);
                    remaps[s2] = remaps[s1];
                    new_counts[actual] += counts[s2];
                }
            }
            else
            {
                ID a1 = sets.find_set(remaps[s1]);
                ID a2 = sets.find_set(remaps[s2]);

                if ( 0 && a1 != a2 && ((maxs[a1]==f)||(maxs[a2]==f)) )
                {
                    ++merged;
                    new_counts[a1] += new_counts[a2];
                    new_counts[a2] = 0;
                    maxs[a1] = std::max(maxs[a1],maxs[a2]);
                    maxs[a2] = 0;
                    ID a = sets.join(a1,a2);
                    std::swap(new_counts[a], new_counts[a1]);
                    std::swap(maxs[a], maxs[a1]);
                }
            }
        }
    }

    next_id -= merged;

    std::vector<ID> remaps2(counts.size());

    next_id = 1;

    for ( ID id = 0; id < counts.size(); ++id )
    {
        ID s = sets.find_set(remaps[id]);
        if ( s && (remaps2[s]==0) )
        {
            remaps2[s] = next_id;
            new_counts[next_id] = new_counts[s];
            ++next_id;
        }
    }

    new_counts.resize(next_id);

    std::ptrdiff_t xdim = seg_ptr->shape()[0];
    std::ptrdiff_t ydim = seg_ptr->shape()[1];
    std::ptrdiff_t zdim = seg_ptr->shape()[2];

    std::ptrdiff_t total = xdim * ydim * zdim;

    ID* seg_raw = seg_ptr->data();

    for ( std::ptrdiff_t idx = 0; idx < total; ++idx )
    {
        seg_raw[idx] = remaps2[remaps[seg_raw[idx]]];
    }

    region_graph<ID,F> new_rg;

    for ( auto& it: rg )
    {
        ID s1 = remaps2[remaps[std::get<1>(it)]];
        ID s2 = remaps2[remaps[std::get<2>(it)]];

        if ( s1 != s2 && s1 && s2 )
        {
            auto mm = std::minmax(s1,s2);
            new_rg.emplace_back(std::get<0>(it), mm.first, mm.second);
        }
    }

    rg.swap(new_rg);

    counts.swap(new_counts);

    std::cout << "New count: " << counts.size() << std::endl;

    std::cout << "Done with updating the region graph, size: "
              << rg.size() << std::endl;
}


std::size_t get_rand_idx( std::vector<uint32_t>& v)
{
    if ( v.size() <= 1 )
    {
        return 0;
    }

    std::size_t r = 0;

    for ( int i = 0; i < v.size() - 1; ++i )
    {
        for ( int j = i+1; j < v.size(); ++j )
            r += v[i]*v[j];
    }

    return r;
}

double square_sum( std::vector<uint32_t>& v)
{
    double r = 0;

    for ( const auto& x: v )
    {
        r += x*x;
    }

    return r;
}

template< typename ID >
void fill_void( ID* arr, std::size_t len )
{
    ID maxi = 0;
    for ( std::size_t i = 0; i < len; ++i )
    {
        maxi = std::max(maxi, arr[i]);
    }

    for ( std::size_t i = 0; i < len; ++i )
    {
        if ( arr[i] == 0 ) arr[i] = ++maxi;
    }
}

std::vector<double> reduce( const std::vector<double>& v )
{
    std::vector<double> ret;
    ret.push_back(v[0]);
    ret.push_back(v[1]);

    for ( std::size_t i = 2; i + 4 < v.size(); i += 2 )
    {
        double oldx = v[i-2];
        double oldy = v[i-1];

        double x = v[i];
        double y = v[i+1];

        double nextx = v[i+2];
        double nexty = v[i+3];

        if ( std::abs(oldx-x) > 0.0001 ||
             std::abs(oldy-y) > 0.0001 ||
             std::abs(nextx-x) > 0.0001 ||
             std::abs(nexty-y) > 0.0001 )
        {
            ret.push_back(x);
            ret.push_back(y);
        }
    }

    ret.push_back(v[v.size()-2]);
    ret.push_back(v[v.size()-1]);

    return ret;
}



std::pair<double,double>
compare_volumes_arb(
                 volume<uint32_t>& gt,
                 volume<uint32_t>& ws, int dimX, int dimY, int dimZ )
{
    //ws is seg
    //std::map<uint32_t, std::map<uint32_t, uint32_t>> map;
    //std::map<uint32_t, std::map<uint32_t, uint32_t>> invmap;
    //std::map<uint32_t, uint32_t> setg, setw;

    double rand_split = 0;
    double rand_merge = 0;

    double t_sq = 0;
    double s_sq = 0;

    double total = 0;
    std::map<uint32_t, std::map<uint32_t, std::size_t>> p_ij;

    std::map<uint32_t, std::size_t> s_i, t_j;

    for ( std::ptrdiff_t z = 0; z < dimX; ++z )
        for ( std::ptrdiff_t y = 0; y < dimY; ++y )
            for ( std::ptrdiff_t x = 0; x < dimZ; ++x )
            {
                uint32_t wsv = ws[x][y][z];
                uint32_t gtv = gt[x][y][z];

                if ( gtv )
                {
                    ++total;

                    ++p_ij[gtv][wsv];
                    ++s_i[wsv];
                    ++t_j[gtv];
                }
            }

    double sum_p_ij = 0;
    for ( auto& a: p_ij )
    {
        for ( auto& b: a.second )
        {
            sum_p_ij += b.second * b.second;
        }
    }

    double sum_t_k = 0;
    for ( auto& a: t_j )
    {
        sum_t_k += a.second * a.second;
    }


    double sum_s_k = 0;
    for ( auto& a: s_i )
    {
        sum_s_k += a.second * a.second;
    }

    //std::cout << sum_p_ij << "\n";
    std::cout << "Rand Split: " << (sum_p_ij/sum_t_k) << "\n";
    std::cout << "Rand Merge: " << (sum_p_ij/sum_s_k) << "\n";
    std::cout << "Rand alpha: " << (sum_p_ij*2/(sum_t_k+sum_s_k)) << "\n";

    return std::make_pair(sum_p_ij/sum_t_k,
                          sum_p_ij/sum_s_k);
}

using namespace std;

struct Vertex
{
    uint32_t first, second;
    float value;
};

typedef vector<Vertex> VertexList;

std::map<std::string,std::vector<double>> eval_c(int dimX, int dimY, int dimZ, int dcons, uint32_t* gt,
float* affs,std::list<int> * threshes, std::list<std::string> * funcs, std::list<int> * save_threshes, std::string* out_ptr)
{
/////////////////////////////////////////// LOAD DATA ///////////////////////////////////////////////////////////////
    bool write_dats =0; // save_seg!=0;
    bool recreate_rg = false;
    bool debug = 1;
    // these values based on 5% at iter = 10000
    double LOW=  .0001;// 0.003785; //.00001; //default = .3
    double HIGH= .9999;// 0.999971; //.99988; //default = .99

    std::string out = *out_ptr;
    std::cout << "evaluating..." << std::endl;
    std::cout << "output: "<< out << std::endl;
    std::string experiment_result_folder = ".";

    volume_ptr<uint32_t> gt_ptr = read_volumes<uint32_t>("", dimX, dimY, dimZ);

    int totalDim = dimX*dimY*dimZ;
    for(int i=0;i<totalDim;i++){
        gt_ptr->data()[i] = gt[i];
    }

    std::cout << std::endl;

    affinity_graph_ptr<float> aff = read_affinity_graphe<float>("", dimX, dimY, dimZ, dcons);

    totalDim*=dcons;
    for(int i=0;i<totalDim;i++){
        aff->data()[i] = affs[i];
    }

    std::cout << "0_dim of affinity " << aff->shape()[0] << "\n";
    std::cout << "1_dim of affinity " << aff->shape()[1] << "\n";
    std::cout << "2_dim of affinity " << aff->shape()[2] << "\n";
    std::cout << "3_dim of affinity " << aff->shape()[3] << "\n";

    std::list<int>::const_iterator iterator;
    std::list<int> thresh_list = *threshes;

    std::map<std::string,std::vector<double>> returnMap;
    volume_ptr<uint32_t>     seg_ref   ;
    std::vector<std::size_t> counts_ref;
    std::tie(seg_ref , counts_ref) = watershed<uint32_t>(aff, LOW, HIGH);
    auto rg = get_region_graph(aff, seg_ref , counts_ref.size()-1);
    volume_ptr<uint32_t>     seg   ;

/////////////////////////////////////////// SQUARE ///////////////////////////////////////////////////////////////

     if ( std::find(funcs->begin(), funcs->end(), "square") != funcs->end() )
     {
         std::cout << "\n\n\nsquare" << "\n";
         std::vector<double> r;

         for (iterator = thresh_list.begin(); iterator != thresh_list.end(); ++iterator) {
                 int thold = *iterator;

             std::cout << "THOLD: " << thold << "\n";
             {
                 seg.reset(new volume<uint32_t>(*seg_ref));
                 std::vector<std::size_t> counts(counts_ref);
                 merge_segments_with_function(seg, rg, counts, square(thold), 10,recreate_rg);
                 if ( std::find(save_threshes->begin(), save_threshes->end(), thold) != save_threshes->end() ){
                    //copy seg to a 1d vector and return it
                    std::vector<uint32_t> seg_vector;
                    for(int i=0;i<5;i++)
                        seg_vector.push_back(i);
                    std::cout << "seg_vector: ";// << seg_vector;
                    write_volume(out+"square/" + std::to_string(thold) + ".dat", seg);  // return threshold
                 }
                 auto x = compare_volumes_arb(*gt_ptr, *seg, dimX,dimY,dimZ);
                 r.push_back(x.first);
                 r.push_back(x.second);
             }
             write_to_file(out+"square.dat", r.data(), r.size());
         }

        returnMap["square"] = r;
     }

     return returnMap;

 }
std::map<std::string,std::vector<double>> oneThresh(int dimX, int dimY, int dimZ, int dcons, uint32_t* gt,
float* affs, int thresh,int eval)
{
/////////////////////////////////////////// LOAD DATA ///////////////////////////////////////////////////////////////
    bool write_dats =0; // save_seg!=0;
    bool recreate_rg = false;
    bool debug = 1;
    // these values based on 5% at iter = 10000
    double LOW=  .0001;// 0.003785; //.00001; //default = .3
    double HIGH= .9999;// 0.999971; //.99988; //default = .99

    std::cout << "evaluating..." << std::endl;
    std::string experiment_result_folder = ".";

    volume_ptr<uint32_t> gt_ptr = read_volumes<uint32_t>("", dimX, dimY, dimZ);

    int totalDim = dimX*dimY*dimZ;
    for(int i=0;i<totalDim;i++){
        gt_ptr->data()[i] = gt[i];
    }

    std::cout << std::endl;

    affinity_graph_ptr<float> aff = read_affinity_graphe<float>("", dimX, dimY, dimZ, dcons);

    totalDim*=dcons;
    for(int i=0;i<totalDim;i++){
        aff->data()[i] = affs[i];
    }

    std::cout << "0_dim of affinity " << aff->shape()[0] << "\n";
    std::cout << "1_dim of affinity " << aff->shape()[1] << "\n";
    std::cout << "2_dim of affinity " << aff->shape()[2] << "\n";
    std::cout << "3_dim of affinity " << aff->shape()[3] << "\n";

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
	 merge_segments_with_function(seg, rg, counts, square(thold), 10,recreate_rg);
	    //copy seg to a 1d vector and return it
	    std::vector<double> seg_vector;
    		//for(int i=0;i<totalDim;i++){
        	//	aff->data()[i] = affs[i];
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
/////////////////////////////////////////// LOAD DATA ///////////////////////////////////////////////////////////////
    bool write_dats =0; // save_seg!=0;
    bool recreate_rg = false;
    bool debug = 1;
    // these values based on 5% at iter = 10000
    double LOW=  .0001;// 0.003785; //.00001; //default = .3
    double HIGH= .9999;// 0.999971; //.99988; //default = .99

    std::cout << "evaluating..." << std::endl;
    std::string experiment_result_folder = ".";


    int totalDim = dimX*dimY*dimZ;

    std::cout << std::endl;

    affinity_graph_ptr<float> aff = read_affinity_graphe<float>("", dimX, dimY, dimZ, dcons);

    totalDim*=dcons;
    for(int i=0;i<totalDim;i++){
        aff->data()[i] = affs[i];
    }

    std::cout << "0_dim of affinity " << aff->shape()[0] << "\n";
    std::cout << "1_dim of affinity " << aff->shape()[1] << "\n";
    std::cout << "2_dim of affinity " << aff->shape()[2] << "\n";
    std::cout << "3_dim of affinity " << aff->shape()[3] << "\n";

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
	 merge_segments_with_function(seg, rg, counts, square(thold), 10,recreate_rg);
	    //copy seg to a 1d vector and return it
	    std::vector<double> seg_vector;
    		//for(int i=0;i<totalDim;i++){
        	//	aff->data()[i] = affs[i];
	    for(int i=0;i<dimX*dimY*dimZ;i++)
		seg_vector.push_back(((double)(seg->data()[i])));
	    std::cout << "seg_vector: ";// << seg_vector;
	returnMap["seg"] = seg_vector; 
	//}
     return returnMap;

 }

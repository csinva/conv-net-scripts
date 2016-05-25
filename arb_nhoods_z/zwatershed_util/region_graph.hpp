#pragma once

#include "types.hpp"

#include <cstddef>
#include <iostream>
#include <map>

template< typename ID, typename F >
inline region_graph_ptr<ID,F>
get_region_graph( //const affinity_graph_ptr<F>& aff_ptr,
                const ID*node1, const ID*node2, const F*edgeWeight, int n_edge,
                  const volume_ptr<ID> seg_ptr, std::size_t max_segid)
{

    region_graph_ptr<ID,F> rg_ptr( new region_graph<ID,F> );
    region_graph<ID,F>& rg = *rg_ptr;
    volume<ID>& seg = *seg_ptr;
    std::vector<std::map<ID,F>> edges(max_segid+1);


    for(int i=0;i<n_edge;i++){
        ID n1 = node1[i];
        ID n2 = node2[i];
        F w = edgeWeight[i];
        std::cout << "n1 n2 " << n1 << " " << n2 << "\n";
        auto mm = std::minmax(seg[n1],seg[n2]);
    }


    /*
    region_graph<ID,F>& rg = *rg_ptr;
        volume<ID>& seg = *seg_ptr;

    std::vector<std::map<ID,F>> edges(max_segid+1);

    std::ptrdiff_t xdim = aff_ptr->shape()[0];
    std::ptrdiff_t ydim = aff_ptr->shape()[1];
    std::ptrdiff_t zdim = aff_ptr->shape()[2];

    affinity_graph<F> aff = *aff_ptr;




    for ( std::ptrdiff_t z = 0; z < zdim; ++z )
        for ( std::ptrdiff_t y = 0; y < ydim; ++y )
            for ( std::ptrdiff_t x = 0; x < xdim; ++x )
            {
                if ( (x > 0) && seg[x][y][z] && seg[x-1][y][z] )
                {
                    auto mm = std::minmax(seg[x][y][z], seg[x-1][y][z]);
                    F& curr = edges[mm.first][mm.second];
                    curr = std::max(curr, aff[x][y][z][0]);
                }
                if ( (y > 0) && seg[x][y][z] && seg[x][y-1][z] )
                {
                    auto mm = std::minmax(seg[x][y][z], seg[x][y-1][z]);
                    F& curr = edges[mm.first][mm.second];
                    curr = std::max(curr, aff[x][y][z][1]);
                }
                if ( (z > 0) && seg[x][y][z] && seg[x][y][z-1] )
                {
                    auto mm = std::minmax(seg[x][y][z], seg[x][y][z-1]);
                    F& curr = edges[mm.first][mm.second];
                    curr = std::max(curr, aff[x][y][z][2]);
                }
            }
    */



    /*
    for ( ID id1 = 1; id1 <= max_segid; ++id1 )
    {
        for ( const auto& p: edges[id1] )
        {
            rg.emplace_back(p.second, id1, p.first);
            //std::cout << p.second << " " << id1 << " " << p.first << "\n";
        }
    }

    std::cout << "Region graph size: " << rg.size() << std::endl;

    std::stable_sort(std::begin(rg), std::end(rg),
                     std::greater<std::tuple<F,ID,ID>>());
    */

    return rg_ptr;
}
#pragma once

#include "types.hpp"
#include <queue>
#include <iostream>
using namespace std;
template< typename ID, typename F, typename L, typename H >
inline tuple< volume_ptr<ID>, vector<size_t> >
watershed(int x_dim, int y_dim, int z_dim, const ID* node1, const ID* node2, const F* edgeWeight, int n_edge//const affinity_graph_ptr<F>& aff_ptr
            , const L& lowv, const H& highv )
{
    using affinity_t = F;
    using id_t       = ID;
    using traits     = watershed_traits<id_t>;
    affinity_t low  = static_cast<affinity_t>(lowv);
    affinity_t high = static_cast<affinity_t>(highv);
    ptrdiff_t xdim = x_dim;//aff_ptr->shape()[0];
    ptrdiff_t ydim = y_dim;//aff_ptr->shape()[1];
    ptrdiff_t zdim = z_dim;//aff_ptr->shape()[2];
    ptrdiff_t size = xdim * ydim * zdim;

    tuple< volume_ptr<id_t>, vector<size_t> > result(
          volume_ptr<id_t>( new volume<id_t>(boost::extents[xdim][ydim][zdim])),//, boost::fortran_storage_order())),
          vector<size_t>(1)
    );
    auto& counts = get<1>(result);
    counts[0] = 0;

    volume<id_t>& seg = *get<0>(result);
    id_t* seg_raw = seg.data();

    // filter by Tmax, Tmin, get rid of repeat edges
    map<pair<int,int>, float> weights; //smaller always on left
    for(int i=0;i<n_edge;i++){
        F weight = edgeWeight[i];
        if(weight<high){ //1a Remove each {u, v} from E if w({vi, u}) > Tmax.
            if(weight<low){
                weight = 0; //1b For each {u, v} from E set w({vi, u}) = 0 if w({vi, u}) < Tmin.
            }
            auto pair = make_pair(node1[i],node2[i]); //if there are repeat edges, only keep the lowest
            if(node2[i]<node1[i])
                pair = make_pair(node2[i],node1[i]);
            if(weights.find(pair)==weights.end()){
                weights[pair] = weight;
            }
            else if(weight < weights[pair]){
                weights[pair] = weight;
            }
        }
    }

    //keep only one outgoing edge pointing to a vertex with the minimal index
    // THERE MIGHT BE AN ERROR HERE - sometimes more than 1 edge per vertex
    map<pair<int,int>, float> weights_filtered; //smaller always on left
    map<int,pair<float,int>> maxes;
    for ( const auto &pair : weights ) {
        ID v0 = get<0>(pair.first);
        ID v1 = get<1>(pair.first);
        F aff = pair.second;
        if(maxes.find(v0)==maxes.end()){
            maxes[v0] = make_pair(aff,v1);
        }
        else{
            auto old_pair = maxes[v0];
            if(aff < old_pair.first){ //2a, 2b Only keep minimal outgoing edge
                maxes[v0] = make_pair(aff,v1);
            }
            else if(aff==old_pair.first&&v1<old_pair.second){ //point to vertex with the minimal index
                maxes[v0] = make_pair(aff,v1);
            }
        }
    }

    // 4. Modify G′ to split the non-minimal plateaus:
    std::queue<int> vqueue;
    map<int,bool> visited;
    for(int i=0;i<n_edge;i++){
        ID v = static_cast<ID>(i);
        // check whether the vertex is a plateau corner
        // check whether it has at least one out-going edge
        // check whether it has at least one biderectional edge
        // CHANGE THIS CHANGE THIS !!!!!!!!!!!!!!!!!!!!!
        if(true){
            visited[v] = true;
            vqueue.push(v);
        }

    }

    while(!vqueue.empty()){
        ID u = vqueue.front();
        vqueue.pop();
        // For all v such that (u,v) ∈ E′ and (v,u) ∈ E′ remove (u,v) from E′. If v is visited remove (v, u) from E′ as well, otherwise mark v as visited and
add it to the end of Q.
    }

    //initialize seg
    for(int i=0;i<size;i++){
        //seg_raw[i] = 1;
    }


    // 5. Replace all unidirectional edges with bidirectional edges. For each (u, v) ∈ E′ add (v,u) to E′ if not already there.

    // 6. Return connected components of the modified G′ - prune and renum

    // 7. Return counts
    /*
    1(c) Remove singleton vertices (vertices with no incident edges in E). Mark them as background.
    */


    /*
    //affinity_graph<F>& aff = *aff_ptr;
    //PREPROCESSING - this stores the minimal outgoing edge for
    for ( std::ptrdiff_t x = 0; x < xdim; ++x )
        for ( std::ptrdiff_t y = 0; y < ydim; ++y )
            for ( std::ptrdiff_t z = 0; z < zdim; ++z )
            {
                id_t& id = seg[x][y][z] = 0;

                F negx = (x>0) ? aff[x][y][z][0] : low;
                F negy = (y>0) ? aff[x][y][z][1] : low;
                F negz = (z>0) ? aff[x][y][z][2] : low;
                F posx = (x<(xdim-1)) ? aff[x+1][y][z][0] : low;
                F posy = (y<(ydim-1)) ? aff[x][y+1][z][1] : low;
                F posz = (z<(zdim-1)) ? aff[x][y][z+1][2] : low;

                F m = std::max({negx,negy,negz,posx,posy,posz}); // max aff of all outgoing edges

                if ( m > low ) // if anything greater than high, id |= marker, if equal
                {
                    if ( negx == m || negx >= high ) { id |= 0x01; }
                    if ( negy == m || negy >= high ) { id |= 0x02; }
                    if ( negz == m || negz >= high ) { id |= 0x04; }
                    if ( posx == m || posx >= high ) { id |= 0x08; }
                    if ( posy == m || posy >= high ) { id |= 0x10; }
                    if ( posz == m || posz >= high ) { id |= 0x20; }
                }
            }


    const std::ptrdiff_t dir[6] = { -1, -xdim, -xdim*ydim, 1, xdim, xdim*ydim }; //offsets for 3x
    const id_t dirmask[6]  = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20 };
    const id_t idirmask[6] = { 0x08, 0x10, 0x20, 0x01, 0x02, 0x04 }; //opposite dir

    // get plato corners

    std::vector<std::ptrdiff_t> bfs;

    for ( std::ptrdiff_t idx = 0; idx < size; ++idx )
    {
        for ( std::ptrdiff_t d = 0; d < 6; ++d )
        {
            if ( seg_raw[idx] & dirmask[d] )
            {
                if ( !(seg_raw[idx+dir[d]] & idirmask[d]) )
                {
                    seg_raw[idx] |= 0x40;
                    bfs.push_back(idx);
                    d = 6; // break;
                }
            }
        }
    }

    // divide the plateaus

    std::size_t bfs_index = 0;

    while ( bfs_index < bfs.size() )
    {
        std::ptrdiff_t idx = bfs[bfs_index];

        id_t to_set = 0;

        for ( std::ptrdiff_t d = 0; d < 6; ++d )
        {
            if ( seg_raw[idx] & dirmask[d] )
            {
                if ( seg_raw[idx+dir[d]] & idirmask[d] )
                {
                    if ( !( seg_raw[idx+dir[d]] & 0x40 ) )
                    {
                        bfs.push_back(idx+dir[d]);
                        seg_raw[idx+dir[d]] |= 0x40;
                    }
                }
                else
                {
                    to_set = dirmask[d];
                }
            }
        }
        seg_raw[idx] = to_set;
        ++bfs_index;
    }

    bfs.clear();

    // main watershed logic

    id_t next_id = 1;

    for ( std::ptrdiff_t idx = 0; idx < size; ++idx )
    {
        if ( seg_raw[idx] == 0 )
        {
            seg_raw[idx] |= traits::high_bit;
            ++counts[0];
        }

        if ( !( seg_raw[idx] & traits::high_bit ) && seg_raw[idx] )
        {
            bfs.push_back(idx);
            bfs_index = 0;
            seg_raw[idx] |= 0x40;

            while ( bfs_index < bfs.size() )
            {
                std::ptrdiff_t me = bfs[bfs_index];

                for ( std::ptrdiff_t d = 0; d < 6; ++d )
                {
                    if ( seg_raw[me] & dirmask[d] )
                    {
                        std::ptrdiff_t him = me + dir[d];
                        if ( seg_raw[him] & traits::high_bit )
                        {
                            counts[ seg_raw[him] & ~traits::high_bit ]
                                += bfs.size();

                            for ( auto& it: bfs )
                            {
                                seg_raw[it] = seg_raw[him];
                            }

                            bfs.clear();
                            d = 6; // break
                        }
                        else if ( !( seg_raw[him] & 0x40 ) )
                        {
                            seg_raw[him] |= 0x40;
                            bfs.push_back( him );

                        }
                    }
                }
                ++bfs_index;
            }

            if ( bfs.size() )
            {
                counts.push_back( bfs.size() );
                for ( auto& it: bfs )
                {
                    seg_raw[it] = traits::high_bit | next_id;
                }
                ++next_id;
                bfs.clear();
            }
        }
    }

    std::cout << "found: " << (next_id-1) << " components\n";

    for ( std::ptrdiff_t idx = 0; idx < size; ++idx )
    {
        seg_raw[idx] &= traits::mask;
    }
    */

    return result;
}

#pragma once

#include "types.hpp"
#include <queue>
#include <iostream>
#include <boost/pending/disjoint_sets.hpp>
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

    // 1 - filter by Tmax, Tmin, get rid of repeat edges
    map<pair<int,int>, float> weights; //smaller always on left
    for(int i=0;i<n_edge;i++){
        F weight = edgeWeight[i];
        if(weight<high){ //1a Remove each {u, v} from E if w({vi, u}) > Tmax.
            if(weight<low){
                weight = 0; //1b For each {u, v} from E set w({vi, u}) = 0 if w({vi, u}) < Tmin.
            }
            auto pair = make_pair(node1[i],node2[i]);
            weights[pair]=weight;
        }
    }
    for ( const auto &pair : weights ) { // make all edges bidirectional
        auto pair2 = make_pair(pair.first.second,pair.first.first);
        weights[pair2] = weights[pair.first];
    }

    // 2 - keep only outgoing edges with min edge (can be multiple)
    map<ID,F> mins; //find mins for each vertex
    for ( const auto &pair : weights ) {
        ID v = pair.first.first;
        F aff = pair.second;
        if(mins.find(v)==mins.end()){
            mins[v] = aff;
        }
        else if(aff < mins[v]){
            mins[v] = aff;
        }
    }
    map<pair<int,int>, float> weights_filtered; //filter only if matching mins
    for ( const auto &pair : weights ) {
        ID v1 = pair.first.first;
        ID v2 = pair.first.second;
        F aff = pair.second;
        if(aff==mins[v1]){
            weights_filtered[make_pair(v1,v2)] = aff;
        }
    }

    // 3 keep only one strictly outgoing edge pointing to a vertex with the minimal index
    weights.clear();
    map<ID,ID> min_indexes;
    for ( const auto &pair : weights_filtered ) {
        ID v1 = pair.first.first;
        ID v2 = pair.first.second;
        if(weights_filtered.find(make_pair(v2,v1))==weights_filtered.end()){ // if not bidirectional
            if(min_indexes.find(v1)==min_indexes.end()){
                min_indexes[v1] = v2;
                weights[make_pair(v1,v2)] = pair.second;
            }
            else if(v2 < min_indexes[v1]){
                weights.erase(make_pair(v1,min_indexes[v1]));
                weights[make_pair(v1,v2)] = pair.second;
                min_indexes[v1]=v2;
            }
        }
        else{ // if bidirectional
            weights[make_pair(v1,v2)] = pair.second;
        }
    }

    // 4. Modify G′ to split the non-minimal plateaus:
    queue<ID> vqueue;
    map<ID,bool> visited;
    map<ID,bool> bidirectional;
    map<ID,bool> outgoing;
    for ( const auto &pair : weights ) { // check which vertices have outoing, bidirectional edges
        ID v1 = pair.first.first;
        ID v2 = pair.first.second;
        if(weights.find(make_pair(v2,v1))==weights.end()) // not bidirectional
            outgoing[v1] = true;
        else                                              // bidirectional
            bidirectional[v1] = true;
    }
    for(const auto &v_pair: bidirectional){ // check whether it has at least one bidirectional edge
        ID v = v_pair.first;
        if(outgoing[v]){ // check whether it has at least one out-going edge
            visited[v] = true; //found a plateau corner
            vqueue.push(v);
        }
    }
    while(!vqueue.empty()){
        ID u = vqueue.front();
        vqueue.pop();
        for(const auto&v_pair:bidirectional){
            ID v = v_pair.first;
            if(weights.find(make_pair(u,v))!=weights.end()){     //u,v in E
                if(weights.find(make_pair(v,u))!=weights.end()){ //v,u in E
                    weights.erase(make_pair(u,v));               //remove u,v from E
                    if(visited[v])                               //If v is visited
                        weights.erase(make_pair(v,u));            //remove v,u from E
                    else{                                         //otherwise
                        visited[v] = true;                        //mark v as visited
                        vqueue.push(v);                           //and add it to the end of Q
                    }
                }
            }
        }
    }


    // 5. Replace all unidirectional edges with bidirectional edges. For each (u, v) ∈ E′ add (v,u) to E′ if not already there.
    for ( const auto &pair : weights ) {
        ID v1 = pair.first.first;
        ID v2 = pair.first.second;
        weights[make_pair(v2,v1)] = pair.second;
    }
    // 6. Return connected components of the modified G′ - prune and renum
    // return counts
    for(int i=0;i<size;i++){//initialize seg
        //seg_raw[i] = 1;
    }


    // Make disjoint sets
    vector<int> rank(size);
    vector<int> parent(size);
    boost::disjoint_sets<int*, int*> dsets(&rank[0],&parent[0]);
    for (int i=0; i<size; ++i)
        dsets.make_set(i);

    /*
    // union
    for (int i = 0; i < nEdge; ++i )
         // check bounds to make sure the nodes are valid
        if ((edgeWeight[i]!=0) && (node1[i]>=0) && (node1[i]<nVert) && (node2[i]>=0) && (node2[i]<nVert))
            dsets.union_set(node1[i],node2[i]);

    // find
    for (int i = 0; i < nVert; ++i)
        seg[i] = dsets.find_set(i);
    */

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

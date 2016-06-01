#pragma once

#include "types.hpp"
#include <queue>
#include <iostream>
#include <boost/pending/disjoint_sets.hpp>
using namespace std;
template< typename ID, typename F, typename L, typename H >
inline tuple< volume_ptr<ID>, vector<size_t> >
watershed(int x_dim, int y_dim, int z_dim, ID* node1, ID* node2, F* edgeWeight, int n_edge//const affinity_graph_ptr<F>& aff_ptr
            , const L& lowv, const H& highv )
{
    // Set up fake example //////////////////////////////////////
    /*
    x_dim = 5;
    y_dim = 4;
    z_dim = 1;
    n_edge = 31;
    ID n1[n_edge];
    ID n2[n_edge];
    F ew[n_edge];
    node1 = n1;
    node2 = n2;
    edgeWeight = ew;
    for(int i=0;i<31;i++)
        edgeWeight[i] = .6;
    // horizontal edges - accross then down (reading order),
    node1[0]=1;node2[0]=2;edgeWeight[0]=.5;
    node1[1]=2;node2[1]=3;edgeWeight[1]=.3;
    node1[2]=3;node2[2]=4;edgeWeight[2]=.4;
    node1[3]=4;node2[3]=5;edgeWeight[3]=.8;
    node1[4]=6;node2[4]=7;edgeWeight[4]=.6;
    node1[5]=7;node2[5]=8;edgeWeight[5]=.6;
    node1[6]=8;node2[6]=9;edgeWeight[6]=.6;
    node1[7]=9;node2[7]=10;edgeWeight[7]=.5;
    node1[8]=11;node2[8]=12;edgeWeight[8]=.6;
    node1[9]=12;node2[9]=13;edgeWeight[9]=.6;
    node1[10]=13;node2[10]=14;edgeWeight[10]=.9;
    node1[11]=14;node2[11]=15;edgeWeight[11]=.4;
    node1[12]=16;node2[12]=17;edgeWeight[12]=.6;
    node1[13]=17;node2[13]=18;edgeWeight[13]=.6;
    node1[14]=18;node2[14]=19;edgeWeight[14]=.6;
    node1[15]=19;node2[15]=20;edgeWeight[15]=.4;
    // vertical edges (reading order)
    node1[16]=1;node2[16]=6;edgeWeight[16]=.6;
    node1[17]=2;node2[17]=7;edgeWeight[17]=.7;
    node1[18]=3;node2[18]=8;edgeWeight[18]=.8;
    node1[19]=4;node2[19]=9;edgeWeight[19]=.5;
    node1[20]=5;node2[20]=10;edgeWeight[20]=.1;
    node1[21]=6;node2[21]=11;edgeWeight[21]=.9;
    node1[22]=7;node2[22]=12;edgeWeight[22]=.6;
    node1[23]=8;node2[23]=13;edgeWeight[23]=.6;
    node1[24]=9;node2[24]=14;edgeWeight[24]=.9;
    node1[25]=10;node2[25]=15;edgeWeight[25]=.4;
    node1[26]=11;node2[26]=16;edgeWeight[26]=.6;
    node1[27]=12;node2[27]=17;edgeWeight[27]=.7;
    node1[28]=13;node2[28]=18;edgeWeight[28]=.7;
    node1[29]=14;node2[29]=19;edgeWeight[29]=.4;
    node1[30]=15;node2[30]=20;edgeWeight[30]=.4;
    for(int i=0;i<31;i++){
        node1[i]-=1;
        node2[i]-=1;
    }
    */
    // End fake example ////////////////////////////////////////
    using affinity_t = F;
    using id_t       = ID;
    using traits     = watershed_traits<id_t>;
    affinity_t low  = static_cast<affinity_t>(lowv);
    affinity_t high = static_cast<affinity_t>(highv);
    ptrdiff_t xdim = x_dim;
    ptrdiff_t ydim = y_dim;
    ptrdiff_t zdim = z_dim;
    ptrdiff_t size = xdim * ydim * zdim;
    ID MAX = 4294967295;
    cout << "nEdge start: " << n_edge << endl;
    tuple< volume_ptr<id_t>, vector<size_t> > result(
          volume_ptr<id_t>( new volume<id_t>(boost::extents[xdim][ydim][zdim])),//, boost::fortran_storage_order())),
          vector<size_t>(1)
    );

    volume<id_t>& seg = *get<0>(result);
    id_t* seg_raw = seg.data();
    for(ID i=0;i<size;i++)
        seg_raw[i]=0;

    ///////////////////////////////////////////////////////////// 1 & 2

    // find maxes
    int num_deleted = 0;
    map<ID,F> maxes; //find maxes for each vertex
    for(int i=0;i<n_edge;i++){
        F aff = edgeWeight[i];
        ID v1 = node1[i];
        ID v2 = node2[i];
        if(!maxes.count(v1))
            maxes[v1] = aff;
        else if(aff > maxes[v1])
            maxes[v1] = aff;
        if(!maxes.count(v2))
            maxes[v2] = aff;
        else if(aff > maxes[v2])
            maxes[v2] = aff;
    }

    // 1 - filter by Tmax, Tmin
    map<pair<ID,ID>, F> weights;
    map<pair<ID,ID>, F> weights_deleted;
    map<ID,bool> found;
    for(int i=0;i<n_edge;i++){
        ID v1 = node1[i];
        ID v2 = node2[i];
        F weight = edgeWeight[i];
        if(weight!=maxes[v1] && weight<high){
            weights[make_pair(node1[i],node2[i])] = weight;
            found[v1]=true;
            found[v2]=true;
        }
        else{
            num_deleted++;
            weights_deleted[make_pair(node1[i],node2[i])] = weight;
        }
        if(weight!=maxes[v2] && weight<high){ //1a Remove each {u, v} from E if w({vi, u}) > Tmax.
            weights[make_pair(node2[i],node1[i])] = weight; // make all edges bidirectional
            found[v1]=true;
            found[v2]=true;
        }
        else{
            num_deleted++;
            weights_deleted[make_pair(node1[i],node2[i])] = weight;
        }
    }
    int num_background=0;
    for(int i=0;i<size;i++)
        if(!found[i]){
            //seg_raw[i]=MAX;
            num_background++;
        }
    cout << "num_background " << num_background << endl;
    cout << "num_deleted " << num_deleted << endl;
    cout << "weights len after filtering " << weights.size() << endl;
    // this is equal to nEdge*2 - num_deleted from src_cython (off by 6)

    // 4. Modify G′ to split the non-minimal plateaus:
    queue<ID> vqueue;
    vector<ID> corners;
    map<ID,bool> visited;
    map<ID,bool> bidirectional;
    map<ID,bool> outgoing;
    int num_out=0;
    int num_bi=0;
    for ( const auto &pair : weights ) { // check which vertices have outgoing, bidirectional edges
        ID v1 = pair.first.first;
        ID v2 = pair.first.second;
        if(!weights.count(make_pair(v2,v1))){            // not bidirectional
            outgoing[v2] = true;
            vqueue.push(v2);
            visited[v2]=true;
            num_out++;
        }
        else{                                              // bidirectional
            bidirectional[v2] = true;
            num_bi++;
        }
    }

    cout << "num out: " << num_out << "=" << visited.size() << endl;
    cout << "num bi: " << num_bi << endl;
    cout << "num corners: " << vqueue.size() << endl;
    cout << "corners" << endl;


    int num_pops = 0;
    int num_pushes = 0;
    unsigned int num_visited = 0;
    int num_tested = 0;
    while(!vqueue.empty()){
        ID u = vqueue.front();
        vqueue.pop();
        num_pops++;
        for(const auto&v_pair:outgoing){                         // THIS IS A SLOW LOOP
            ID v = v_pair.second;
            if(weights.count(make_pair(v,u))){
                num_tested++;
            }
        }
    }
    std::cout << "num_tested: " <<num_tested << std::endl;
    std::cout << "num_visited: " <<num_visited << std::endl;
    std::cout << "num_pops: " << num_pops << std::endl;
    std::cout << "num_pushes: " << num_pushes << std::endl;


    // 5. Replace all unidirectional edges with bidirectional edges
    for ( const auto &pair : weights )
        weights[make_pair(pair.first.second,pair.first.first)] = pair.second;

    // 6. Return connected components of the modified G
    cout << "nEdge: " << weights.size() << endl;
    vector<ID> rank(size);
    vector<ID> parent(size);
    boost::disjoint_sets<ID*, ID*> dsets(&rank[0],&parent[0]);
    for (ID i=0; i<size; ++i){
        //if(!seg_raw[i]==MAX)
         dsets.make_set(i);
    }
    for ( const auto &pair : weights_deleted ) // union
        dsets.union_set(pair.first.first,pair.first.second);

    for(int i=0;i<size-1;i++){
        if(!found[i]){
            dsets.union_set(i,i+1);
        }
    }

    // find and renum
    map<ID,ID> renum;
    map<ID,ID> counts_map;
    for(ID i=0;i<size;i++){             // find
        //if(!seg_raw[i]==MAX)
        seg_raw[i]=dsets.find_set(i);
        counts_map[seg_raw[i]]++;
    }

    auto& counts = get<1>(result);
    int count = 0;
    for (const auto &pair:counts_map){
        renum[pair.first] = count++;
        counts.push_back(counts_map[pair.first]);
    }
    for(ID i=0;i<size;i++){
        seg_raw[i]=renum[seg_raw[i]];
    }




    return result;
}

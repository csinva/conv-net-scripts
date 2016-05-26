#pragma once

#include "types.hpp"
#include <queue>
#include <iostream>
#include <boost/pending/disjoint_sets.hpp>
using namespace std;
template< typename ID, typename F, typename L, typename H >
inline tuple< volume_ptr<ID>, vector<size_t> >
watershed(int x_dim, int y_dim, int z_dim, const ID* node11, const ID* node21, const F* edgeWeight1, int n_edge//const affinity_graph_ptr<F>& aff_ptr
            , const L& lowv, const H& highv )
{
    // Set up fake example //////////////////////////////////////
    x_dim = 5;
    y_dim = 4;
    z_dim = 1;
    n_edge = 31;
    ID n1[n_edge];
    ID n2[n_edge];
    F ew[n_edge];
    ID* node1 = n1;
    ID* node2 = n2;
    F* edgeWeight = ew;
    for(int i=0;i<31;i++)
        edgeWeight[i] = .6;
    // horizontal edges - accross then down (reading order),
    node1[0]=1;node2[0]=2;edgeWeight[0]=.5;
    node1[1]=2;node2[0]=3;edgeWeight[0]=.3;
    node1[2]=3;node2[0]=4;edgeWeight[0]=.4;
    node1[3]=4;node2[0]=5;edgeWeight[0]=.8;
    node1[4]=6;node2[0]=7;edgeWeight[0]=.6;
    node1[5]=7;node2[0]=8;edgeWeight[0]=.6;
    node1[6]=8;node2[0]=9;edgeWeight[0]=.6;
    node1[7]=9;node2[0]=10;edgeWeight[0]=.5;
    node1[8]=11;node2[0]=12;edgeWeight[0]=.6;
    node1[9]=12;node2[0]=13;edgeWeight[0]=.6;
    node1[10]=13;node2[0]=14;edgeWeight[0]=.9;
    node1[11]=14;node2[0]=15;edgeWeight[0]=.4;
    node1[12]=16;node2[0]=17;edgeWeight[0]=.6;
    node1[13]=17;node2[0]=18;edgeWeight[0]=.6;
    node1[14]=18;node2[0]=19;edgeWeight[0]=.6;
    node1[15]=19;node2[0]=20;edgeWeight[0]=.4;
    // vertical edges (reading order)
    node1[16]=1;node2[0]=6;edgeWeight[0]=.6;
    node1[17]=2;node2[0]=7;edgeWeight[0]=.7;
    node1[18]=3;node2[0]=8;edgeWeight[0]=.8;
    node1[19]=4;node2[0]=9;edgeWeight[0]=.5;
    node1[20]=5;node2[0]=10;edgeWeight[0]=.1;
    node1[21]=6;node2[0]=11;edgeWeight[0]=.9;
    node1[22]=7;node2[0]=12;edgeWeight[0]=.6;
    node1[23]=8;node2[0]=13;edgeWeight[0]=.6;
    node1[24]=9;node2[0]=14;edgeWeight[0]=.9;
    node1[25]=10;node2[0]=15;edgeWeight[0]=.4;
    node1[26]=11;node2[0]=16;edgeWeight[0]=.6;
    node1[27]=12;node2[0]=17;edgeWeight[0]=.7;
    node1[28]=13;node2[0]=18;edgeWeight[0]=.7;
    node1[29]=14;node2[0]=19;edgeWeight[0]=.4;
    node1[30]=15;node2[0]=20;edgeWeight[0]=.4;

    // End fake example ////////////////////////////////////////
    // REMEMBER TO SET THE NAMES BACK

    using affinity_t = F;
    using id_t       = ID;
    using traits     = watershed_traits<id_t>;
    affinity_t low  = static_cast<affinity_t>(lowv);
    affinity_t high = static_cast<affinity_t>(highv);
    ptrdiff_t xdim = x_dim;
    ptrdiff_t ydim = y_dim;
    ptrdiff_t zdim = z_dim;
    ptrdiff_t size = xdim * ydim * zdim;
    cout << "nEdge start: " << n_edge << endl;
    tuple< volume_ptr<id_t>, vector<size_t> > result(
          volume_ptr<id_t>( new volume<id_t>(boost::extents[xdim][ydim][zdim])),//, boost::fortran_storage_order())),
          vector<size_t>(1)
    );
    auto& counts = get<1>(result);
    counts[0] = 0;

    volume<id_t>& seg = *get<0>(result);
    id_t* seg_raw = seg.data();

    // 1 - filter by Tmax, Tmin, get rid of repeat edges
    map<pair<int,int>, float> weights;
    for(int i=0;i<n_edge;i++){
        F weight = edgeWeight[i];
        if(weight<high){ //1a Remove each {u, v} from E if w({vi, u}) > Tmax.
            if(weight<low)
                weight = 0; //1b For each {u, v} from E set w({vi, u}) = 0 if w({vi, u}) < Tmin.
            weights[make_pair(node1[i],node2[i])] = weight;
            weights[make_pair(node2[i],node1[i])] = weight; // make all edges bidirectional
        }
    }
    for (const auto &pair:weights){
        cout << "weight " << pair.first.first << "," << pair.first.second << " = " << pair.second << endl;
    }

    // 2 - keep only outgoing edges with min edge (can be multiple)
    map<ID,F> mins; //find mins for each vertex
    for ( const auto &pair : weights ) {
        ID v = pair.first.first;
        F aff = pair.second;
        if(!mins.count(v))
            mins[v] = aff;
        else if(aff < mins[v])
            mins[v] = aff;

    }
    for (const auto &pair:mins){
        cout << "min " << pair.first << " = " << pair.second << endl;
    }
    map<pair<int,int>, float> weights_filtered; //filter only if matching mins
    for ( const auto &pair : weights ) {
        ID v1 = pair.first.first;
        ID v2 = pair.first.second;
        F aff = pair.second;
        F epsilon = .000001;
        if(aff<=mins[v1]+epsilon) //float comparison
            weights_filtered[make_pair(v1,v2)] = aff;
    }

    // 3 keep only one strictly outgoing edge pointing to a vertex with the minimal index
    weights.clear();
    map<ID,ID> min_indexes;
    for ( const auto &pair : weights_filtered ) {
        ID v1 = pair.first.first;
        ID v2 = pair.first.second;
        if(!weights.count(make_pair(v2,v1))){ // if strictly outgoing
            if(!min_indexes.count(v1)){
                min_indexes[v1] = v2;
                weights[make_pair(v1,v2)] = pair.second;
            }
            else if(v2 < min_indexes[v1]){
                weights.erase(make_pair(v1,min_indexes[v1]));
                weights[make_pair(v1,v2)] = pair.second;
                min_indexes[v1]=v2;
            }
        }
        else // if bidirectional
            weights[make_pair(v1,v2)] = pair.second;
    }

    // 4. Modify G′ to split the non-minimal plateaus:
    queue<ID> vqueue;
    map<ID,bool> visited;
    map<ID,bool> bidirectional;
    map<ID,bool> outgoing;
    for ( const auto &pair : weights ) { // check which vertices have outgoing, bidirectional edges
        ID v1 = pair.first.first;
        ID v2 = pair.first.second;
        if(!weights.count(make_pair(v2,v1)))            // not bidirectional
            outgoing[v1] = true;
        else                                              // bidirectional
            bidirectional[v1] = true;
    }
    for(const auto &v_pair: bidirectional){ // check whether it has at least one bidirectional edge
        ID v = v_pair.first;
        if(outgoing[v]){                    // check whether it has at least one out-going edge
            visited[v] = true;              //found a plateau corner
            vqueue.push(v);
        }
    }
    while(!vqueue.empty()){
        ID u = vqueue.front();
        vqueue.pop();
        for(const auto&v_pair:bidirectional){
            ID v = v_pair.first;
            if(weights.count(make_pair(u,v))){                    //u,v in E
                if(weights.count(make_pair(v,u))){                //v,u in E
                    weights.erase(make_pair(u,v));                //remove u,v from E
                    if(visited[v])                                //If v is visited
                        weights.erase(make_pair(v,u));            //remove v,u from E
                    else{                                         //otherwise
                        visited[v] = true;                        //mark v as visited
                        vqueue.push(v);                           //and add it to the end of Q
                    }
                }
            }
        }
    }

    // 5. Replace all unidirectional edges with bidirectional edges
    for ( const auto &pair : weights )
        weights[make_pair(pair.first.first,pair.first.second)] = pair.second;

    // 6. Return connected components of the modified G
    cout << "nEdge: " << weights.size() << endl;
    cout << "size: " << size << endl;
    vector<ID> rank(size);
    vector<ID> parent(size);
    boost::disjoint_sets<ID*, ID*> dsets(&rank[0],&parent[0]);
    for (ID i=0; i<size; ++i)
        dsets.make_set(i);

    for ( const auto &pair : weights ) // union
        dsets.union_set(pair.first.first,pair.first.second);

    for(ID i=0;i<size;i++)             // find
        seg_raw[i]=dsets.find_set(i);

    /*
    1(c) Remove singleton vertices (vertices with no incident edges in E). Mark them as background. - doesn't matter here because everything has edges
    */
    return result;
}

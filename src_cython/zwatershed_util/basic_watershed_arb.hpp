#pragma once

#include "types.hpp"
#include <queue>
#include <iostream>
#include <boost/pending/disjoint_sets.hpp>
using namespace std;
template< typename ID, typename F, typename L, typename H >
inline tuple< volume_ptr<ID>, vector<size_t> >
watershed_arb(int x_dim, int y_dim, int z_dim, ID* node1, ID* node2, F* edgeWeight, int n_edge//const affinity_graph_ptr<F>& aff_ptr
            , const L& lowv, const H& highv )
{

    using affinity_t = F;
    using id_t       = ID;
    using traits     = watershed_traits<id_t>;
    ID MAX = 4294967295;
    affinity_t low  = static_cast<affinity_t>(lowv);
    affinity_t high = static_cast<affinity_t>(highv);
    ptrdiff_t xdim = x_dim;
    ptrdiff_t ydim = y_dim;
    ptrdiff_t zdim = z_dim;
    ptrdiff_t size = xdim * ydim * zdim;
    cout << "nEdge start: " << n_edge << endl;
    tuple< volume_ptr<id_t>, vector<size_t> > result(
          volume_ptr<id_t>( new volume<id_t>(boost::extents[xdim][ydim][zdim])),
          vector<size_t>(1)
    );

    volume<id_t>& seg = *get<0>(result);
    id_t* seg_raw = seg.data();
    for(ID i=0;i<size;i++)
        seg_raw[i]=MAX;

    // 1 - filter by Tmax, Tmin
    map<pair<int,int>, float> weights;
    for(int i=0;i<n_edge;i++){
        F weight = edgeWeight[i];
            if(weight<low)
                weight = 0; //1b For each {u, v} from E set w({vi, u}) = 0 if w({vi, u}) < Tmin.
            weights[make_pair(node1[i],node2[i])] = weight;
            weights[make_pair(node2[i],node1[i])] = weight; // make all edges bidirectional
            seg_raw[node1[i]] = 0;
            seg_raw[node2[i]] = 0; //1c set all vertices with no edges as background
    }

    // 2 - keep only outgoing edges with min edge (can be multiple)
    map<ID,F> maxes; //find mins for each vertex
    for ( const auto &pair : weights ) {
        ID v = pair.first.first;
        F aff = pair.second;
        if(!maxes.count(v))
            maxes[v] = aff;
        else if(aff > maxes[v])
            maxes[v] = aff;
    }

    map<pair<int,int>, float> weights_filtered; //filter only if matching mins
    for ( const auto &pair : weights ) {
        ID v1 = pair.first.first;
        ID v2 = pair.first.second;
        F aff = pair.second;
        if(aff==maxes[v1] || aff >=high) //float comparison
            weights_filtered[make_pair(v1,v2)] = aff;
    }

    cout << "weights len: "<<weights.size()<<endl;
    cout << "weights filtered len: "<<weights_filtered.size()<<endl;

    // 3 keep only one strictly outgoing edge pointing to a vertex with the minimal index
    weights.clear();
    map<ID,ID> min_indexes;
    for ( const auto &pair : weights_filtered ) {
        ID v1 = pair.first.first;
        ID v2 = pair.first.second;
        if(!weights_filtered.count(make_pair(v2,v1))){ // if strictly outgoing
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
    cout << "weights len: "<<weights.size()<<endl;

    // 4. Modify G′ to split the non-minimal plateaus:
    queue<ID> vqueue;
    map<ID,bool> visited;
    map<ID,bool> bidirectional;
    map<ID,bool> outgoing;
    int num_outgoing = 0;
    for ( const auto &pair : weights ) { // check which vertices have outgoing, bidirectional edges
        ID v1 = pair.first.first;
        ID v2 = pair.first.second;
        if(!weights.count(make_pair(v2,v1))){            // not bidirectional
            vqueue.push(v1); // might be v2
            visited[v1] = true;
        }
    }
    cout << "num corners " << vqueue.size() << endl;
    cout << "num distinct corners " << visited.size() << endl;
    while(!vqueue.empty()){
        ID u = vqueue.front();
        vqueue.pop();
        for(const auto&v_pair:bidirectional){
            ID v = v_pair.first;
            if(weights.count(make_pair(u,v))){                    //u,v in E
                if(weights.count(make_pair(v,u))){                //v,u in E
                    weights.erase(make_pair(u,v));                //remove u,v from E
                    if(visited[v]){                                //If v is visited
                        weights.erase(make_pair(v,u));            //remove v,u from E
                    }
                    else{                                         //otherwise
                        visited[v] = true;                        //mark v as visited
                        vqueue.push(v);                           //and add it to the end of Q
                    }
                }
            }
        }
    }

    // 5. Replace all unidirectional edges with bidirectional edges - probably not necessary
    for ( const auto &pair : weights )
        weights[make_pair(pair.first.second,pair.first.first)] = pair.second;

    // 6. Return connected components of the modified G
    cout << "nEdge: " << weights.size() << endl;
    cout << "size: " << size << endl;
    vector<ID> rank(size);
    vector<ID> parent(size);
    boost::disjoint_sets<ID*, ID*> dsets(&rank[0],&parent[0]);
    for (ID i=0; i<size; ++i){
        if(seg_raw[i]!=MAX)
            dsets.make_set(i);
    }
    for ( const auto &pair : weights ) // union
        dsets.union_set(pair.first.first,pair.first.second);

    for(ID i=0;i<size;i++){             // find
        if(seg_raw[i]!=MAX)
            seg_raw[i]=dsets.find_set(i);
    }

    // renumber and get counts
    for(ID i=0;i<size;i++){
        if(seg_raw[i]==MAX)
            seg_raw[i]=0;
        else
            seg_raw[i]++;
    }

    map<ID,ID> counts_map;  //label->count(label)
    map<ID,ID> renum;       //old_label->new_label
    auto& counts = get<1>(result);
    for(ID i=0;i<size;i++)
        counts_map[seg_raw[i]]++;

    // deal with 0
    /*
    if(counts_map.count(0))
        counts.push_back(counts_map[0]);
        counts_map.erase(0);
    else
        counts.push_back(0);
    renum[0]=0
    */

    int new_label = 1;
    for (const auto &pair:counts_map){
        //cout << pair.first << " ";
        renum[pair.first] = new_label;
        counts.push_back(counts_map[pair.first]);
        new_label++;
    }
    for(ID i=0;i<size;i++){
        seg_raw[i]=renum[seg_raw[i]];
    }
    return result;
}

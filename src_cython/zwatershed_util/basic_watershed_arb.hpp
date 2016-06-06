#pragma once

#include "types.hpp"
#include <queue>
#include <iostream>
#include <unordered_set>
#include <boost/pending/disjoint_sets.hpp>

using namespace std;
template< typename ID, typename F, typename L, typename H >
inline tuple< volume_ptr<ID>, vector<size_t> >
watershed_arb(int xdim, int ydim, int zdim, ID* node1, ID* node2, F* edgeWeight, int n_edge//const affinity_graph_ptr<F>& aff_ptr
            , const L& lowv, const H& highv )
{

    using affinity_t = F;
    using id_t       = ID;
    using traits     = watershed_traits<id_t>;
    ID MAX = 4294967295;
    affinity_t low  = static_cast<affinity_t>(lowv);
    affinity_t high = static_cast<affinity_t>(highv);
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
    map<ID,F> maxes; //find maxes for each vertex
    map<pair<int,int>, float> weights;
    for(int i=0;i<n_edge;i++){
        F aff = edgeWeight[i];
        ID v1 = node1[i];
        ID v2 = node2[i];
        if(aff>=low){ //1b For each {u, v} from E set w({vi, u}) = 0 if w({vi, u}) < Tmin.
            weights[make_pair(v1,v2)] = aff;
            seg_raw[node1[i]] = 0; //1c set all vertices with no edges as background
            seg_raw[node2[i]] = 0;
            if(!maxes.count(v1))
                maxes[v1] = aff;
            else if(aff > maxes[v1])
                maxes[v1] = aff;
            if(!maxes.count(v2)) // make all edges bidirectional
                maxes[v2] = aff;
            else if(aff > maxes[v2])
                maxes[v2] = aff;
        }
    }

    // 2 - keep only outgoing edges with max edge (can be multiple)
    map<ID, unordered_set<ID> > edges;
    for ( const auto &pair : weights ) {
        ID v1 = pair.first.first;
        ID v2 = pair.first.second;
        F aff = pair.second;
        if(aff==maxes[v1] || aff >=high){
            if(aff==maxes[v2] || aff >=high){
                ID v2_bi = v2|traits::high_bit;
                edges[v1].insert(v2_bi);  // insert bidirectional edge
            }
            else{
                edges[v1].insert(v2);
            }
        }
        else if(aff==maxes[v2] || aff >=high){ // not bidirectional
            edges[v2].insert(v1);
        }

    }

    cout << "edges len: "<<edges.size()<<endl;

    // 3 keep only one strictly outgoing edge pointing to a vertex with the minimal index
    int num_out = 0;
    int num_bi = 0;
    map<ID,ID> min_indexes;
    queue<ID> vqueue;
    map<ID,bool> visited;
    for ( const auto &pair : edges ) {
        ID v1 = pair.first;
        for( const auto &v2:pair.second){
            //if(edges[v2].find(v1)==edges[v2].end()){ // if strictly outgoing
            if(! (v2 & traits::high_bit) ){ // if strictly outgoing
                num_out++;
                if(!min_indexes.count(v1))
                    min_indexes[v1] = v2;
                else if(v2 < min_indexes[v1]){
                    edges[v1].erase(min_indexes[v1]);
                    min_indexes[v1]=v2;
                }
                vqueue.push(v1);
                visited[v1] = true;
            }
            else{
                num_bi++;
            }
        }
    }

    cout << "num out " << num_out << endl;
    cout << "num bi " << num_bi << endl;
    cout << "num corners " << vqueue.size() << endl;
    cout << "num distinct corners " << visited.size() << endl;

    // 4. Modify G′ to split the non-minimal plateaus:
    while(!vqueue.empty()){
        ID u = vqueue.front();
        vqueue.pop();
        //for( const auto &v:edges[u]){//u,v in E
        for (auto it = edges[u].begin(); it != edges[u].end();) {
            ID v = *it;
            if(edges[v].find(u)!=edges[v].end()){                //v,u in E
                it = edges[u].erase(it);
                if(visited[v])                                //If v is visited
                    edges[v].erase(u);
                else{                                         //otherwise
                    visited[v] = true;                        //mark v as visited
                    vqueue.push(v);                           //and add it to the end of Q
                }
            }
            else
                ++it;
        }
    }

    // 6. Return connected components of the modified G
    vector<ID> rank(size);
    vector<ID> parent(size);
    boost::disjoint_sets<ID*, ID*> dsets(&rank[0],&parent[0]);
    for (ID i=0; i<size; ++i){
        if(seg_raw[i]!=MAX)
            dsets.make_set(i);
    }
    for ( const auto &pair : edges ) {                  // check which vertices have outgoing, bidirectional edges
        ID v1 = pair.first;
        for( const auto &v2:pair.second){
            dsets.union_set(v1,v2);                     // 5. Replace all unidirectional edges with bidirectional edges
        }
    }

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


    int new_label = 1;
    for (const auto &pair:counts_map){
        renum[pair.first] = new_label;
        counts.push_back(counts_map[pair.first]);
        new_label++;
    }
    for(ID i=0;i<size;i++){
        seg_raw[i]=renum[seg_raw[i]];
    }
    return result;
}

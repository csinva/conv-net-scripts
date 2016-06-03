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
    map<pair<int,int>, float> weights;
    for(int i=0;i<n_edge;i++){
        F weight = edgeWeight[i];
            if(weight<low)
                weight = 0; //1b For each {u, v} from E set w({vi, u}) = 0 if w({vi, u}) < Tmin.
            weights[make_pair(node1[i],node2[i])] = weight;
            weights[make_pair(node2[i],node1[i])] = weight; // make all edges bidirectional
            seg_raw[node1[i]] = 0; //1c set all vertices with no edges as background
            seg_raw[node2[i]] = 0;
    }

    // 2 - keep only outgoing edges with min edge (can be multiple)
    map<ID,F> maxes; //find maxes for each vertex
    for ( const auto &pair : weights ) {
        ID v = pair.first.first;
        F aff = pair.second;
        if(!maxes.count(v))
            maxes[v] = aff;
        else if(aff > maxes[v])
            maxes[v] = aff;
    }

    map<ID, unordered_set<ID> > edges;
    for ( const auto &pair : weights ) {
        ID v1 = pair.first.first;
        ID v2 = pair.first.second;
        F aff = pair.second;
        if(aff==maxes[v1] || aff >=high){ //float comparison
            edges[v1].insert(v2);
        }
    }

    cout << "weights len: "<<weights.size()<<endl;

    // 3 keep only one strictly outgoing edge pointing to a vertex with the minimal index
    map<ID,ID> min_indexes;

    for ( const auto &pair : edges ) {
        for( const auto &v2:pair.second){
            ID v1 = pair.first;
            if(edges[v2].find(v1)==edges[v2].end()){ // if strictly outgoing
                if(!min_indexes.count(v1)){
                    min_indexes[v1] = v2;
                }
                else if(v2 < min_indexes[v1]){
                    edges[v1].erase(min_indexes[v1]);
                    min_indexes[v1]=v2;
                }
            }
        }
    }

    // 4. Modify G′ to split the non-minimal plateaus:
    queue<ID> vqueue;
    map<ID,bool> visited;
    int num_outgoing = 0;
    for ( const auto &pair : edges ) {                  // check which vertices have outgoing, bidirectional edges
        ID v1 = pair.first;
        for( const auto &v2:pair.second){
            if(edges[v2].find(v1)==edges[v2].end()){            // not bidirectional
                vqueue.push(v1);
                visited[v1] = true;
            }
        }
    }

    cout << "num corners " << vqueue.size() << endl;
    cout << "num distinct corners " << visited.size() << endl;
    while(!vqueue.empty()){
        ID u = vqueue.front();
        vqueue.pop();
        for( const auto &v:edges[u]){//u,v in E
            if(edges[v].find(u)!=edges[v].end()){                //v,u in E
                edges[u].erase(v);
                if(visited[v])                                //If v is visited
                    edges[v].erase(u);
                else{                                         //otherwise
                    visited[v] = true;                        //mark v as visited
                    vqueue.push(v);                           //and add it to the end of Q
                }
            }
        }
    }

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

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
    ID MAX = 4294967295;
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
    for(ID i=0;i<size;i++)
        seg_raw[i]=0;

    // don't get rid of weights on the high end
    //low  = 0;
    //high = 1;


    // 1 - filter by Tmax, Tmin, get rid of repeat edges
    map<pair<int,int>, float> weights;
    for(int i=0;i<n_edge;i++){
        F weight = edgeWeight[i];
        if(weight<=high){ //1a Remove each {u, v} from E if w({vi, u}) > Tmax.
            if(weight<low)
                weight = 0; //1b For each {u, v} from E set w({vi, u}) = 0 if w({vi, u}) < Tmin.
            weights[make_pair(node1[i],node2[i])] = weight;
            weights[make_pair(node2[i],node1[i])] = weight; // make all edges bidirectional
        }
        else{
            seg_raw[node1[i]]=MAX;
            seg_raw[node2[i]]=MAX;
        }
    }


    for (const auto &pair:weights){
        //cout << "weight " << pair.first.first << "," << pair.first.second << " = " << pair.second << endl;
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
    /*
    for (const auto &pair:mins){
        cout << "min " << pair.first << " = " << pair.second << endl;
    }
    */
    map<pair<int,int>, float> weights_filtered; //filter only if matching mins
    for ( const auto &pair : weights ) {
        ID v1 = pair.first.first;
        ID v2 = pair.first.second;
        F aff = pair.second;
        F epsilon = 1e-20; // this is subject to change
        if(aff<=mins[v1]+epsilon) //float comparison
            weights_filtered[make_pair(v1,v2)] = aff;
    }

    /*
    cout << "\nweights filtered len: "<<weights.size()<<endl;
    for (const auto &pair:weights_filtered){
        cout << "weight filtered " << pair.first.first << "," << pair.first.second << " = " << pair.second << endl;
    }
    */


    // 3 keep only one strictly outgoing edge pointing to a vertex with the minimal index
    weights.clear();
    map<ID,ID> min_indexes;
    for ( const auto &pair : weights_filtered ) {
        ID v1 = pair.first.first;
        ID v2 = pair.first.second;
        if(!weights_filtered.count(make_pair(v2,v1))){ // if strictly outgoing
            if(!min_indexes.count(v1)){
                //cout <<  v1 << "," << min_indexes[v1] << "," << v2 << endl;
                min_indexes[v1] = v2;
                weights[make_pair(v1,v2)] = pair.second;
            }
            else if(v2 < min_indexes[v1]){
                //cout << "erasing " << v1 << "," << min_indexes[v1] << endl;
                weights.erase(make_pair(v1,min_indexes[v1]));
                weights[make_pair(v1,v2)] = pair.second;
                min_indexes[v1]=v2;
            }
        }
        else // if bidirectional
            weights[make_pair(v1,v2)] = pair.second;
    }
    /*
    for (const auto &pair:min_indexes){
        cout << "min_index " << pair.first << " = " << pair.second << endl;
    }

    cout << "\nweights len: "<<weights.size()<<endl;
    for (const auto &pair:weights){
        cout << "weight " << pair.first.first << "," << pair.first.second << " = " << pair.second << endl;
    }

    cout << "\ndiff weights" <<endl;
    for (const auto &pair:weights_filtered){
        if(!weights.count(pair.first))
            cout << "diff weight " << pair.first.first << "," << pair.first.second << " = " << pair.second << endl;
    }
    */

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
            //cout << "plateau corner " << v << endl;
        }
    }
    while(!vqueue.empty()){
        ID u = vqueue.front();
        vqueue.pop();
        //cout << "u: " << u << endl;
        for(const auto&v_pair:bidirectional){
            ID v = v_pair.first;
            //cout << "\tv: " << v << endl;
            if(weights.count(make_pair(u,v))){                    //u,v in E
                //cout << "\t\tfound u,v!" << endl;
                if(weights.count(make_pair(v,u))){                //v,u in E
                    //cout << "\t\t\tfound v,u!" << endl;
                    weights.erase(make_pair(u,v));                //remove u,v from E
                    //cout << "\t\t\t\terase end " << u << "," << v << endl;
                    if(visited[v]){                                //If v is visited
                        //cout << "\t\t\t\terase end " << v << "," << u << endl;
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
    for (const auto &pair:weights){
        //cout << "weights end " << pair.first.first << "," << pair.first.second << " = " << pair.second << endl;
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
    for (ID i=0; i<size; ++i){
        if(!seg_raw[i])
            dsets.make_set(i);
    }
    for ( const auto &pair : weights ) // union
        dsets.union_set(pair.first.first,pair.first.second);

    for(ID i=0;i<size;i++){             // find
        if(!seg_raw[i])
            seg_raw[i]=dsets.find_set(i);
    }

    // renumber
    for(ID i=0;i<size;i++){
        if(seg_raw[i]==MAX){
            seg_raw[i]=0;
        }
        else{
            seg_raw[i]++;
        }
    }
    return result;
}

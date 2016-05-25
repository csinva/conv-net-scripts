#include <iostream>
#include <cstdlib>
#include <cmath>
#include <boost/pending/disjoint_sets.hpp>
#include <vector>
#include <queue>
#include <map>

using namespace std;

template <class T>
class AffinityGraphCompare{
	private:
	    const T * mEdgeWeightArray;
	public:
		AffinityGraphCompare(const T * EdgeWeightArray){
			mEdgeWeightArray = EdgeWeightArray;
		}
		bool operator() (const int& ind1, const int& ind2) const {
			return (mEdgeWeightArray[ind1] > mEdgeWeightArray[ind2]);
		}
};

void connected_components_cpp(const int nVert,
               const int nEdge, const int* node1, const int* node2, const int* edgeWeight,
               int* seg){

    /* Make disjoint sets */
    vector<int> rank(nVert);
    vector<int> parent(nVert);
    boost::disjoint_sets<int*, int*> dsets(&rank[0],&parent[0]);
    for (int i=0; i<nVert; ++i)
        dsets.make_set(i);

    /* union */
    for (int i = 0; i < nEdge; ++i )
         // check bounds to make sure the nodes are valid
        if ((edgeWeight[i]!=0) && (node1[i]>=0) && (node1[i]<nVert) && (node2[i]>=0) && (node2[i]<nVert))
            dsets.union_set(node1[i],node2[i]);

    /* find */
    for (int i = 0; i < nVert; ++i)
        seg[i] = dsets.find_set(i);
}

map<pair<int,int>, float> marker_watershed_cpp(const int nVert, const int* marker,
               const int nEdge, const int* node1, const int* node2, const float* edgeWeight,
               int* seg){

    /* Make disjoint sets */
    vector<int> rank(nVert);
    vector<int> parent(nVert);
    boost::disjoint_sets<int*, int*> dsets(&rank[0],&parent[0]);
    for (int i=0; i<nVert; ++i)
        dsets.make_set(i);

    /* initialize output array and find representatives of each class */
    map<int,int> components;
    for (int i=0; i<nVert; ++i){
        seg[i] = marker[i];
        if (seg[i] > 0)
            components[seg[i]] = i;
    }

    // merge vertices labeled with the same marker
    for (int i=0; i<nVert; ++i)
        if (seg[i] > 0)
            dsets.union_set(components[seg[i]],i);

    /* Sort all the edges in decreasing order of weight */
    vector<int> pqueue( nEdge );
    int j = 0;
    for (int i = 0; i < nEdge; ++i)
        if ((edgeWeight[i]!=0) &&
            (node1[i]>=0) && (node1[i]<nVert) &&
            (node2[i]>=0) && (node2[i]<nVert) &&
            (marker[node1[i]]>=0) && (marker[node2[i]]>=0))
                pqueue[ j++ ] = i;
    unsigned long nValidEdge = j;
    pqueue.resize(nValidEdge);
    sort( pqueue.begin(), pqueue.end(), AffinityGraphCompare<float>( edgeWeight ) );

    /* Start MST */
    int set1, set2, label_of_set1, label_of_set2;
    for (unsigned int i = 0; i < pqueue.size(); ++i ) {
        int e = pqueue[i];
        set1=dsets.find_set(node1[e]);
        set2=dsets.find_set(node2[e]);
        label_of_set1 = seg[set1];
        label_of_set2 = seg[set2];

        if ((set1!=set2) &&
            ( ((label_of_set1==0) && (marker[set1]==0)) ||
             ((label_of_set2==0) && (marker[set1]==0))) ){

            dsets.link(set1, set2);
            // either label_of_set1 is 0 or label_of_set2 is 0.
            seg[dsets.find_set(set1)] = max(label_of_set1,label_of_set2);
        }
    }

    // write out the final coloring
    for (int i=0; i<nVert; i++){
        int before = seg[i];
        seg[i] = seg[dsets.find_set(i)];
    }

    // calculate the region graph
    map<pair<int,int>, float> rg;
    cout << "calculating rgn graph init\n";
    for (unsigned int i = 0; i < pqueue.size(); ++i ) {
        int e = pqueue[i];
        set1=seg[dsets.find_set(node1[e])];
        set2=seg[dsets.find_set(node2[e])];
        if(set1!=set2){
            auto pair = make_pair(set1,set2);
            if(set2<set1)
                pair = make_pair(set2,set1);
            float w_new = edgeWeight[i];
            float w_old = 0;
            auto iter = rg.find(pair);
            if(iter == rg.end())
                rg[pair] = w_new;
            else{
                w_old = iter->second;
                if(w_new < w_old)
                    rg[pair] = w_new;
            }
        }
    }
    return rg;
}


map<pair<int,int>, float> marker_watershed_with_thresh(const int nVert, const int* marker,
               const int nEdge, const int* node1, const int* node2, const float* edgeWeight,
               int* seg, int * seg_sizes, float thresh, map<pair<int,int>, float> rg){

    // cout << "merge segments\n";

    /* Make disjoint sets */
    vector<int> rank(nVert);
    vector<int> parent(nVert);
    boost::disjoint_sets<int*, int*> dsets(&rank[0],&parent[0]);
    for (int i=0; i<nVert; ++i)
        dsets.make_set(i);

    /* initialize output array and find representatives of each class */
    map<int,int> components;
    for (int i=0; i<nVert; ++i){
        seg[i] = marker[i];
        if (seg[i] > 0)
            components[seg[i]] = i;
    }

    // merge vertices labeled with the same marker
    for (int i=0; i<nVert; ++i)
        if (seg[i] > 0)
            dsets.union_set(components[seg[i]],i);


    // merge segments based on rg
    map<pair<int,int>, float> rg_return;

    bool merged = false;
    for(auto it = rg.begin(); it != rg.end(); it++){
        auto key = it->first;
        auto aff = it->second;
        //set1=seg[dsets.find_set(node1[e])];
        int seg1 = get<0>(key);
        int seg2 = get<1>(key);
        int set1 = dsets.find_set(components[seg1]);//dsets.find_set(seg1);
        int set2 = dsets.find_set(components[seg2]);//dsets.find_set(seg2);
        //rg_return[key]=aff;
        if(set1==set2 && !merged){
            cout << "\n\nERROR DIFFERENT SEGS WITHOUT MERGING\n\n" << endl;
        }
        //if(seg1==seg2)
         //cout << "\tsegs " << seg1 << "," << seg2 << " sets " << set1 << "=" <<set2 << "!" << endl;

        if(set1!=set2){
            int size1 = seg_sizes[components[seg1]];
            int size2 = seg_sizes[components[seg2]];
            //cout << "\nseg1 " << seg1 << " seg2 " << seg2 << endl;
            //cout << "set1 " << set1 << " set2 " << set2 << endl;
            //cout << "sizes " << size1 << " " << size2 << endl;
            int size_min = min(size1,size2);
            //double THRESH=  .9999;
            //float max_size_to_merge = 1e10;
            //if(aff < THRESH)
            float max_size_to_merge = thresh*aff*aff;


            rg_return[key]=aff;

            if(size_min < max_size_to_merge){
                cout << "sizes " << size_min << " < " << max_size_to_merge << endl;
                // merge
                merged = true;
                cout << "merging " << set1 << " " << set2 << "\n";
                dsets.union_set(set1,set2);
                //cout << "merged into set " << dsets.find_set(components[seg1]) << endl;
                //seg_sizes[dsets.find_set(components[seg1])] = size1+size2;
                // update sizes
                int new_set = dsets.find_set(seg1);
                for(int i=0;i<components.size();i++){
                    if(dsets.find_set(i)==new_set){
                        seg_sizes[i]=size1+size2;
                    }
                   //sizes[dsets.find_set(i)] =
                }
                /*
                seg_sizes[seg1]+=size2;
                seg_sizes[seg2]+=size1;
                */
            }
            else{
                rg_return[key]=aff;
            }
        }



    }

    // write out the final coloring
    for (int i=0; i<nVert; i++)
        seg[i] = seg[dsets.find_set(i)];

    return rg_return;

}


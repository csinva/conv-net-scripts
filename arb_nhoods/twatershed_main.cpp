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


void marker_watershed_cpp(const int nVert, const int* marker,
               const int nEdge, const int* node1, const int* node2, const float* edgeWeight,
               int* seg){

    /* Make disjoint sets */
    vector<int> rank(nVert);
    vector<int> parent(nVert);
    boost::disjoint_sets<int*, int*> dsets(&rank[0],&parent[0]);
    for (int i=0; i<nVert; ++i)
        dsets.make_set(i);

    /* initialize output array and find representatives of each class */
    std::map<int,int> components;
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
    std::vector<int> pqueue( nEdge );
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

        set1=dsets.find_set(node1[i]);
        set2=dsets.find_set(node2[i]);
        label_of_set1 = seg[set1];
        label_of_set2 = seg[set2];

        if ((set1!=set2) &&
            ( ((label_of_set1==0) && (marker[set1]==0)) ||
             ((label_of_set2==0) && (marker[set1]==0))) ){

            dsets.link(set1, set2);
            // either label_of_set1 is 0 or label_of_set2 is 0.
            seg[dsets.find_set(set1)] = std::max(label_of_set1,label_of_set2);
            
        }

    }

    // write out the final coloring
    for (int i=0; i<nVert; i++)
        seg[i] = seg[dsets.find_set(i)];

}

#ifndef TWATERSHED_H
#define TWATERSHED_H

void connected_components_cpp(const int nVert,
               const int nEdge, const int* node1, const int* node2, const int* edgeWeight,
               int* seg);

std::map<std::pair<int,int>, float> marker_watershed_cpp(const int nVert, const int* marker,
               const int nEdge, const int* node1, const int* node2, const float* edgeWeight,
               int* seg);

std::map<std::pair<int,int>, float> marker_watershed_with_thresh(const int nVert, const int* marker,
               const int nEdge, const int* node1, const int* node2, const float* edgeWeight,
               int* seg, int * seg_sizes, float thresh, std::map<std::pair<int,int>, float> rg);

#endif
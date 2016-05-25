from libcpp.list cimport list
from libcpp.map cimport map
from libcpp.vector cimport vector
from libcpp.string cimport string
from libcpp.pair cimport pair
from libc.stdint cimport uint32_t
import numpy as np
import os
cimport numpy as np
import h5py

# interface methods
def zwatershed_and_metrics_edge(gt, node1, node2, edgeWeight, threshes, save_threshes):
    threshes.sort()
    return eval_all(gt, node1, node2, edgeWeight, threshes, save_threshes, eval=1, h5=0)

def calc_rgn_graph(np.ndarray[uint32_t, ndim=3] seg, np.ndarray[int, ndim=1] node1,
             np.ndarray[int, ndim=1] node2, np.ndarray[float, ndim=1] edgeWeight):
    cdef np.ndarray[uint32_t, ndim=1] counts = np.empty(1, dtype='uint32')
    dims = seg.shape
    dims[3] = 0
    map = calc_region_graph(dims[0], dims[1], dims[2], dims[3], &node1[0], &node2[0], &edgeWeight[0])
    graph = np.array(map['rg'], dtype='float32')
    return {'rg': graph.reshape(len(graph) / 3, 3), 'seg': np.array(map['seg'], dtype='uint32'),
            'counts': np.array(map['counts'], dtype='uint32')}

def eval_all(np.ndarray[uint32_t, ndim=3] gt, np.ndarray[int, ndim=1] node1,
             np.ndarray[int, ndim=1] node2, np.ndarray[float, ndim=1] edgeWeight, threshes, save_threshes, int eval,
             int h5, seg_save_path="NULL/"):
    '''
    if h5:
        makedirs(seg_save_path)

    # get initial seg,rg
    affs = np.asfortranarray(np.transpose(affs, (1, 2, 3, 0)))
    gt = np.array(gt, order='F')
    map = calc_rgn_graph(gt, affs)
    cdef np.ndarray[uint32_t, ndim=1] seg_in = map['seg']
    cdef np.ndarray[uint32_t, ndim=1] counts_out = map['counts']
    cdef np.ndarray[np.float32_t, ndim=2] rgn_graph = map['rg']
    counts_len = len(map['counts'])
    dims = affs.shape

    # get segs, stats
    segs, splits, merges = [], [], []
    for i in range(len(threshes)):
        map = oneThresh_with_stats(dims[0], dims[1], dims[2], dims[3], &gt[0, 0, 0], &affs[0, 0, 0, 0],
                                   &rgn_graph[0, 0],
                                   rgn_graph.shape[0], &seg_in[0], &counts_out[0], counts_len, threshes[i], eval)
        seg = np.array(map['seg'], dtype='uint32').reshape((dims[2], dims[1], dims[0])).transpose(2, 1, 0)
        graph = np.array(map['rg'], dtype='float32')
        counts_out = np.array(map['counts'], dtype='uint32')
        counts_len = len(counts_out)
        seg_in = np.array(map['seg'], dtype='uint32')
        rgn_graph = graph.reshape(len(graph) / 3, 3)
        if threshes[i] in save_threshes:
            if h5:
                f = h5py.File(seg_save_path + 'seg_' + str(threshes[i]) + '.h5', 'w')
                f["main"] = seg
                f.close()
            else:
                segs.append(seg)
        splits = splits + [map['stats'][0]]
        merges = merges + [map['stats'][1]]
    max_f_score = 2 / (1 / splits[0] + 1 / merges[0])
    for j in range(len(splits)):
        f_score = 2 / (1 / splits[j] + 1 / merges[j])
        if f_score > max_f_score:
            max_f_score = f_score
    returnMap = {'V_Rand': max_f_score, 'V_Rand_split': splits, 'V_Rand_merge': merges}
    if h5:
        return returnMap
    else:
        return segs, returnMap
    '''

'''
def zwatershed_and_metrics_h5(gt, affs, threshes, save_threshes, seg_save_path):
    threshes.sort()
    return eval_all(gt, affs, threshes, save_threshes, eval=1, h5=1, seg_save_path=seg_save_path)

def zwatershed(affs, threshes):
    threshes.sort()
    return watershed_all_no_eval(affs, threshes, threshes, eval=0, h5=0)

def zwatershed_h5(affs, threshes, seg_save_path):
    threshes.sort()
    watershed_all_no_eval(affs, threshes, threshes, eval=0, h5=1, seg_save_path=seg_save_path)
'''

# helper methods


def watershed_all_no_eval(np.ndarray[np.float32_t, ndim=4] affs, threshes, save_threshes, int eval, int h5,
                          seg_save_path="NULL/"):
    if h5:
        makedirs(seg_save_path)

    # get initial seg,rg
    affs = np.asfortranarray(np.transpose(affs, (1, 2, 3, 0)))
    dims = affs.shape
    seg_empty = np.empty((dims[0], dims[1], dims[2]), dtype='uint32')
    map = calc_rgn_graph(seg_empty, affs)
    cdef np.ndarray[uint32_t, ndim=1] seg_in = map['seg']
    cdef np.ndarray[uint32_t, ndim=1] counts_out = map['counts']
    cdef np.ndarray[np.float32_t, ndim=2] rgn_graph = map['rg']
    segs = []

    # get segs, stats
    for i in range(len(threshes)):
        map = oneThresh(dims[0], dims[1], dims[2], dims[3], &affs[0, 0, 0, 0], &rgn_graph[0, 0],
                        rgn_graph.shape[0], &seg_in[0], &counts_out[0], len(map['counts']), threshes[i], eval)
        seg = np.array(map['seg'], dtype='uint32').reshape((dims[2], dims[1], dims[0])).transpose(2, 1, 0)
        graph = np.array(map['rg'], dtype='float32')
        counts_out = np.array(map['counts'], dtype='uint32')
        counts_len = len(counts_out)
        seg_in = np.array(map['seg'], dtype='uint32')
        rgn_graph = graph.reshape(len(graph) / 3, 3)
        if threshes[i] in save_threshes:
            if h5:
                f = h5py.File(seg_save_path + 'seg_' + str(threshes[i]) + '.h5', 'w')
                f["main"] = seg
                f.close()
            else:
                segs.append(seg)
    if not h5:
        return segs

def makedirs(seg_save_path):
    if not seg_save_path.endswith("/"):
        seg_save_path += "/"
    if not os.path.exists(seg_save_path):
        os.makedirs(seg_save_path)

# c++ methods
cdef extern from "zwatershed.h":
    map[string, list[float]] calc_region_graph(int dimX, int dimY, int dimZ, int dcons, const int*node1,
                                               const int*node2, const float*edgeWeight)
    map[string, vector[double]] oneThresh_with_stats(int dx, int dy, int dz, int dcons, np.uint32_t*gt,
                                                     np.float32_t*affs,
                                                     np.float32_t*rgn_graph, int rgn_graph_len, uint32_t*seg,
                                                     uint32_t*counts,
                                                     int counts_len, int thresh, int evaluate)
    map[string, vector[double]] oneThresh(int dx, int dy, int dz, int dcons, np.float32_t*affs,
                                          np.float32_t*rgn_graph, int rgn_graph_len, uint32_t*seg,
                                          uint32_t*counts,
                                          int counts_len, int thresh,
                                          int evaluate)

# edgelist methods
def nodelist_like(shape, nhood):
    # constructs the node lists corresponding to the edge list representation of an affinity graph
    # assume  node shape is represented as:
    # shape = (z, y, x)
    # nhood.shape = (edges, 3)
    nEdge = nhood.shape[0]
    nodes = np.arange(np.prod(shape), dtype=np.int32).reshape(shape)
    node1 = np.tile(nodes, (nEdge, 1, 1, 1))
    node2 = np.full(node1.shape, -1, dtype=np.int32)

    for e in range(nEdge):
        node2[e, \
        max(0, -nhood[e, 0]):min(shape[0], shape[0] - nhood[e, 0]), \
        max(0, -nhood[e, 1]):min(shape[1], shape[1] - nhood[e, 1]), \
        max(0, -nhood[e, 2]):min(shape[2], shape[2] - nhood[e, 2])] = \
            nodes[max(0, nhood[e, 0]):min(shape[0], shape[0] + nhood[e, 0]), \
            max(0, nhood[e, 1]):min(shape[1], shape[1] + nhood[e, 1]), \
            max(0, nhood[e, 2]):min(shape[2], shape[2] + nhood[e, 2])]

    return (node1, node2)

def affgraph_to_edgelist(aff, nhood):
    node1, node2 = nodelist_like(aff.shape[1:], nhood)
    return (node1.ravel(), node2.ravel(), aff.ravel())

def mknhood3d(radius=1):
    # Makes nhood structures for some most used dense graphs.
    # The neighborhood reference for the dense graph representation we use
    # nhood(1,:) is a 3 vector that describe the node that conn(:,:,:,1) connects to
    # so to use it: conn(23,12,42,3) is the edge between node [23 12 42] and [23 12 42]+nhood(3,:)
    # See? It's simple! nhood is just the offset vector that the edge corresponds to.

    ceilrad = np.ceil(radius)
    x = np.arange(-ceilrad, ceilrad + 1, 1)
    y = np.arange(-ceilrad, ceilrad + 1, 1)
    z = np.arange(-ceilrad, ceilrad + 1, 1)
    [i, j, k] = np.meshgrid(z, y, z)

    idxkeep = (i ** 2 + j ** 2 + k ** 2) <= radius ** 2
    i = i[idxkeep].ravel();
    j = j[idxkeep].ravel();
    k = k[idxkeep].ravel();
    zeroIdx = np.ceil(len(i) / 2).astype(np.int32);

    nhood = np.vstack((k[:zeroIdx], i[:zeroIdx], j[:zeroIdx])).T.astype(np.int32)
    return np.ascontiguousarray(np.flipud(nhood))

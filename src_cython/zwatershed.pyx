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

cdef extern from "zwatershed.h":
    map[string,list[float]] calc_region_graph(int dimX, int dimY, int dimZ, int dcons, np.uint32_t*seg, np.float32_t*affs)
    map[string, vector[double]] oneThresh(int dx, int dy, int dz, int dcons, np.uint32_t*gt, np.float32_t*affs,
                                          np.float32_t*rgn_graph, int rgn_graph_len, int thresh, int evaluate)
    map[string, vector[double]] oneThresh_no_gt(int dx, int dy, int dz, int dcons, np.float32_t*affs, int thresh,
                                                int evaluate)

def calc_rgn_graph(np.ndarray[uint32_t, ndim=3] seg, np.ndarray[np.float32_t, ndim=4] affs):
    dims = affs.shape
    map = calc_region_graph(dims[0], dims[1], dims[2], dims[3], &seg[0, 0, 0], &affs[0, 0, 0, 0])
    graph = np.array(map['rg'], dtype='float32')
    rgn_graph = graph.reshape(len(graph) / 3, 3)  # num, num, float
    return rgn_graph

def evalAll(np.ndarray[uint32_t, ndim=3] gt, np.ndarray[np.float32_t, ndim=4] affs, threshes, save_threshes, int eval,
            int h5, seg_save_path="NULL/"):
    affs = np.transpose(affs, (1, 2, 3, 0))
    gt = np.array(gt, order='F')
    affs = np.array(affs, order='F')
    cdef np.ndarray[np.float32_t, ndim=2] rgn_graph = calc_rgn_graph(gt, affs)
    if not seg_save_path.endswith("/"):
        seg_save_path = seg_save_path + "/"
        if not os.path.exists(seg_save_path):
            os.makedirs(seg_save_path)
    dims = affs.shape
    segs, splits, merges = [], [], []
    for i in range(len(threshes)):
        map = oneThresh(dims[0], dims[1], dims[2], dims[3], &gt[0, 0, 0], &affs[0, 0, 0, 0], &rgn_graph[0, 0],
                        rgn_graph.shape[0], threshes[i], eval)
        seg_np = np.array(map['seg'], dtype='uint32').reshape((dims[0], dims[1], dims[2]))
        seg_np = np.transpose(seg_np, (2, 1, 0))
        if threshes[i] in save_threshes:
            segs = segs + [seg_np]
            if h5 == 1:
                f = h5py.File(seg_save_path + 'seg_' + str(threshes[i]) + '.h5', 'w')
                f["main"] = seg_np
                f.close()
        splits = splits + [map['stats'][0]]
        merges = merges + [map['stats'][1]]
    max_f_score = 2 / (1 / splits[0] + 1 / merges[0])
    for j in range(len(splits)):
        f_score = 2 / (1 / splits[j] + 1 / merges[j])
        if f_score > max_f_score:
            max_f_score = f_score
    returnMap = {}
    returnMap['V_Rand'] = max_f_score
    returnMap['V_Rand_split'] = splits
    returnMap['V_Rand_merge'] = merges
    if h5 == 1:
        return returnMap
    else:
        return segs, returnMap

def watershedAll_no_eval(np.ndarray[np.float32_t, ndim=4] affs, threshes, save_threshes, int eval, int h5,
                         seg_save_path="NULL/"):
    if not seg_save_path.endswith("/"):
        seg_save_path = seg_save_path + "/"
        if not os.path.exists(seg_save_path):
            os.makedirs(seg_save_path)
    # change both to fortran order
    affs = np.transpose(affs, (1, 2, 3, 0))
    affs = np.array(affs, order='F')
    dims = affs.shape
    segs = []
    for i in range(len(threshes)):
        map = oneThresh_no_gt(dims[0], dims[1], dims[2], dims[3], &affs[0, 0, 0, 0], threshes[i], eval)
        seg_np = np.array(map['seg'], dtype='uint32').reshape((dims[0], dims[1], dims[2]))
        seg_np = np.transpose(seg_np, (2, 1, 0))
        if threshes[i] in save_threshes:
            segs = segs + [seg_np]
            if h5 == 1:
                f = h5py.File(seg_save_path + 'seg_' + str(threshes[i]) + '.h5', 'w')
                f["main"] = seg_np
                f.close()
    if not h5 == 1:
        return segs

def zwatershed_and_metrics(gt, affs, threshes, save_threshes):
    threshes.sort()
    return evalAll(gt, affs, threshes, save_threshes, eval=1, h5=0)

def zwatershed_and_metrics_h5(gt, affs, threshes, save_threshes, seg_save_path):
    threshes.sort()
    return evalAll(gt, affs, threshes, save_threshes, eval=1, h5=1, seg_save_path=seg_save_path)

def zwatershed(affs, threshes):
    threshes.sort()
    return watershedAll_no_eval(affs, threshes, threshes, eval=0, h5=0)

def zwatershed_h5(affs, threshes, seg_save_path):
    threshes.sort()
    watershedAll_no_eval(affs, threshes, threshes, eval=0, h5=1, seg_save_path=seg_save_path)

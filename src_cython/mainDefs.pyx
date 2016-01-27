from libcpp.list cimport list
from libcpp.map cimport map
from libcpp.vector cimport vector
from libcpp.string cimport string
from libcpp.pair cimport pair
import numpy as np
import os
cimport numpy as np
import h5py

cdef extern from "main2.h":
    map[string,vector[double]] eval_c(int dx, int dy, int dz, int dcons, np.uint32_t* gt, np.float32_t* affs, list[int] *threshes, list[string] *funcs, list[int] *save_threshes, string* out)
    map[string,vector[double]] oneThresh(int dx, int dy, int dz, int dcons, np.uint32_t* gt, np.float32_t* affs, int thresh, int evaluate)
    map[string,vector[double]] oneThresh_no_gt(int dx, int dy, int dz, int dcons, np.float32_t* affs, int thresh, int evaluate)

def eval(np.ndarray[np.uint32_t,ndim=3] gt,np.ndarray[np.float32_t,ndim=4] affs, list[int] threshes, list[string] funcs, list[int] save_threshes, string out='out/'):
    dims = affs.shape
    dirs = [out,out+'square']
    for i in range(len(dirs)):
        if not os.path.exists(dirs[i]):
            os.makedirs(dirs[i])
    map = eval_c(dims[0],dims[1],dims[2],dims[3],&gt[0,0,0],&affs[0,0,0,0],&threshes,&funcs,&save_threshes,&out)
    return map

def evalAll(np.ndarray[np.uint32_t,ndim=3] gt,np.ndarray[np.float32_t,ndim=4] affs, threshes, save_threshes,int eval, int h5, seg_save_path="NULL/"):
    if not seg_save_path.endswith("/"):
        seg_save_path = seg_save_path + "/"
        if not os.path.exists(seg_save_path):
            os.makedirs(seg_save_path)
    dims = affs.shape
    segs = []
    for i in range(len(threshes)):
        map = oneThresh(dims[0],dims[1],dims[2],dims[3],&gt[0,0,0],&affs[0,0,0,0],threshes[i],eval)
        seg_np = np.array(map['seg']).reshape((dims[0],dims[1],dims[2]))
        seg_np = np.transpose(seg_np,(2,1,0))
        if threshes[i] in save_threshes:
            segs = segs + [seg_np]
            if h5==1:
                f = h5py.File(seg_save_path+'seg_'+str(threshes[i])+'.h5','w')
                f["main"] = seg_np
                f.close()
    if not h5==1:
        if eval:
            return segs,map['stats']
        else:
            return segs

def watershedAll_no_eval(np.ndarray[np.float32_t,ndim=4] affs, threshes, save_threshes,int eval, int h5, seg_save_path="NULL/"):
    if not seg_save_path.endswith("/"):
        seg_save_path = seg_save_path + "/"
        if not os.path.exists(seg_save_path):
            os.makedirs(seg_save_path)
    dims = affs.shape
    segs = []
    for i in range(len(threshes)):
        map = oneThresh_no_gt(dims[0],dims[1],dims[2],dims[3],&affs[0,0,0,0],threshes[i],eval)
        seg_np = np.array(map['seg']).reshape((dims[0],dims[1],dims[2]))
        seg_np = np.transpose(seg_np,(2,1,0))
        if threshes[i] in save_threshes:
            segs = segs + [seg_np]
            if h5==1:
                f = h5py.File(seg_save_path+'seg_'+str(threshes[i])+'.h5','w')
                f["main"] = seg_np
                f.close()
    if not h5==1:
        if eval:
            return segs
        else:
            return segs
    

def zwatershed_and_metrics(np.ndarray[np.uint32_t,ndim=3] gt,np.ndarray[np.float32_t,ndim=4] affs, list[int] threshes, list[int] save_threshes):
    return evalAll(gt,affs,threshes,save_threshes,eval=1,h5=0)

def zwatershed_and_metrics_h5(np.ndarray[np.uint32_t,ndim=3] gt, np.ndarray[np.float32_t,ndim=4] affs, list[int] threshes, list[int] save_threshes,seg_save_path):
    return evalAll(gt,affs,threshes,save_threshes,eval=1,h5=1,seg_save_path=seg_save_path)

def zwatershed(np.ndarray[np.uint32_t,ndim=3] gt,np.ndarray[np.float32_t,ndim=4] affs, list[int] threshes):
    return watershedAll_no_eval(affs,threshes,threshes,eval=1,h5=0)

def zwatershed_h5(np.ndarray[np.uint32_t,ndim=3] gt,np.ndarray[np.float32_t,ndim=4] affs, list[int] threshes):
    watershedAll_no_eval(affs,threshes,threshes,eval=1,h5=1)


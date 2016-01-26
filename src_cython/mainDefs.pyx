from libcpp.list cimport list
from libcpp.map cimport map
from libcpp.vector cimport vector
from libcpp.string cimport string
from libcpp.pair cimport pair
import numpy as np
import os
cimport numpy as np

cdef extern from "main2.h":
    map[string,vector[double]] eval_c(int dx, int dy, int dz, int dcons, np.uint32_t* gt, np.float32_t* affs, list[int] *threshes, list[string] *funcs, list[int] *save_threshes, string* out)

cdef extern from "main2.h":
    map[string,vector[double]] oneThresh(int dx, int dy, int dz, int dcons, np.uint32_t* gt, np.float32_t* affs, int thresh, int evaluate)

def eval(np.ndarray[np.uint32_t,ndim=3] gt,np.ndarray[np.float32_t,ndim=4] affs, list[int] threshes, list[string] funcs, list[int] save_threshes, string out='out/'):
    dims = affs.shape
    dirs = [out,out+'linear',out+'square',out+'threshold',out+'watershed',out+'lowhigh']
    for i in range(len(dirs)):
        if not os.path.exists(dirs[i]):
            os.makedirs(dirs[i])
    # list contains six args ('linear','square','fel','thresh','watershed','lowhigh')
    map = eval_c(dims[0],dims[1],dims[2],dims[3],&gt[0,0,0],&affs[0,0,0,0],&threshes,&funcs,&save_threshes,&out)
    return map


def evalAll(np.ndarray[np.uint32_t,ndim=3] gt,np.ndarray[np.float32_t,ndim=4] affs, list[int] threshes, list[int] save_threshes, string out='out/'):
    print("new evalAll")
    dims = affs.shape
    map = oneThresh(dims[0],dims[1],dims[2],dims[3],&gt[0,0,0],&affs[0,0,0,0],100,1)
    print map

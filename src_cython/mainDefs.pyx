from libcpp.list cimport list
from libcpp.string cimport string
import numpy as np
import os
cimport numpy as np

cdef extern from "main2.h":
    int eval_c(int dx, int dy, int dz, int dcons, np.uint32_t* gt, np.float32_t* affs, list[int] *threshes, string* out)

def eval(np.ndarray[np.uint32_t,ndim=3] gt,np.ndarray[np.float32_t,ndim=4] affs, list[int] threshes, string out='out/'):
    dims = affs.shape
    dirs = [out,out+'felzenszwalb',out+'linear',out+'square',out+'threshold',out+'watershed']
    for i in range(len(dirs)):
        if not os.path.exists(dirs[i]):
            os.makedirs(dirs[i])

    eval_c(dims[0],dims[1],dims[2],dims[3],&gt[0,0,0],&affs[0,0,0,0],&threshes,&out)
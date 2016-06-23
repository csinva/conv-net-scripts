import numpy as np
import sys
import time
import os
import h5py
import os.path as op
import matplotlib
import matplotlib.cm as cm
import matplotlib.pyplot as plt
from multiprocessing import Pool
from itertools import product
from par_funcs import *
sys.path.append('..')
V = 20

# -------------------------------- parameters ---------------------------------------
# pred_file = '/groups/turaga/home/turagas/research/caffe_v2/processed/bock2/120000/sample_A_x1_y1_z1_xy1.h5'
pred_file = '/groups/turaga/home/turagas/research/caffe_v2/processed/bock2/120000/cutout_3k.h5'
# pred_file = '/groups/turaga/home/turagas/turagalab/FROM_TIER2/singhc/train/output_200000/tstvol-1_2.h5'
out_folder = '/nobackup/turaga/singhc/3k_400/' # _3k _vol has full, max_len = 300
outname = out_folder+'out.h5'
NUM_WORKERS = 32
MAX_LEN = 400





def merge_by_thresh(seg,seg_sizes,rg,thresh): #agnostic to seg shape
    re = {}
    seg_max = np.max(seg)
    seg_min = np.min(seg)
    print "calculating renums..."
    t1 =time.time()
    for i in range(rg.shape[0]):
        n1,n2,w = rg[i,:]
        size = w*w*thresh
        if seg_sizes[n1] < size or seg_sizes[n2] < size:
            re[n2]=n1
            seg_sizes[n1]+=seg_sizes[n2]
            seg_sizes[n2]+=seg_sizes[n1]
    re_filtered = {}
    print "calc renums time",time.time()-t1,"secs"
    t1 =time.time()
    print "filtering renums..."
    for key in re:
        val = re[key]
        while val in re:
            val = re[val]
        if key < seg_max and val < seg_max:
            re_filtered[key] = val
    print "filtering renums time",time.time()-t1,"secs"
    t1 =time.time()
    print "renumbering..."
    mp = np.arange(0,seg_max+1,dtype='uint64')
    mp[re_filtered.keys()] = re_filtered.values()
    seg = mp[seg]
    print "renumbering time",time.time()-t1,"secs"
    t1 =time.time()
    return seg



print "loading data..."
num,thresh = 0,2000
f = h5py.File(outname, 'a')
s,e = f['starts'][num],f['ends'][num]
seg = f['seg'][s[0]:e[0]-3,s[1]:e[1]-3,s[2]:e[2]-3]
seg_sizes = np.array(f['seg_sizes'])
rg = np.array(f['rg_'+str(num)])
f.close()
print "num_segs",len(np.unique(seg))
print "rg lens",len(rg)


seg_merged = merge_by_thresh(seg,seg_sizes,rg,thresh)

print "num_segs",len(np.unique(seg))
print "rg lens",len(rg)


from __future__ import print_function
import sys, os, math
import h5py
import numpy as np
import time
from numpy import float32, int32, uint8, dtype
from os.path import join
sys.path.append('/groups/turaga/home/singhc/caffe_v1/PyGreentea') # Relative path to where PyGreentea resides
sys.path.append('src_cython')
import PyGreentea as pygt
from evaluateFile import evaluateFile
from processFile import processFile

hdf5_gt_file = '/groups/turaga/home/turagas/data/FlyEM/fibsem_medulla_7col/tstvol-520-1-h5/groundtruth_seg_thick.h5'
hdf5_aff_file = '/groups/turaga/home/turagas/data/FlyEM/fibsem_medulla_7col/tstvol-520-1-h5/groundtruth_aff.h5'
#hdf5_pred_file = '/tier2/turaga/turagas/research/pygt_models/fibsem5/test_out_0.h5' #'',0,2,3,4,6
#hdf5_pred_file = '/groups/turaga/home/singhc/caffe_v1/pygt_models/fibsem4/tstvol-520-1-h5.h5'
hdf5_pred_file = "tstvol-520-1-h5.h5"
#dirs = [i*10000 for i in range(1,40)] #should go to 40
dirs = [10000,20000]

iters = ['net_iter_'+str(dirs[i])+'.caffemodel' for i in range(len(dirs))]
#pred_files = ['net_iter_'+dirs[i]+'_out.h5' for i in range(len(dirs))]
print(iters)

# threshes
#threshes = [100+i*100 for i in range(0,10)]+[i*1000 for i in range(2,11)]+[i*10000 for i in range(2,11)] # 100...1,000...100,000
threshes = [50000 + i*10000 for i in range(10)]
print(threshes)

# funcs
#funcs = ['linear','square','threshold','watershed','lowhigh']
funcs = ['linear','square','threshold']
#funcs = ['lowhigh']

save_segs = False

# folder
basefolders = ['','2','3','4','6']
basename = '/groups/turaga/home/turagas/research/caffe_v1/pygt_models/fibsem'
filenames = ["tstvol-1_"+basefolders[i] for i in range(len(basefolders))]

# output folder
outs = ['out/fibsem' +basefolders[i]+ '_iter_'+str(dirs[j])+'/' for j in range(len(dirs)) for i in range(len(basefolders))]
print(outs)



for iter_idx in range(len(dirs)):
	for base_idx in range(len(basefolders)):
		start = time.clock()
		processFile(basename+basefolders[base_idx]+'/',dirs[iter_idx],filenames[base_idx])
		args = [hdf5_gt_file,filenames[base_idx],threshes,funcs,save_segs,outs[iter_idx*len(basefolders)+base_idx]]
		#evaluateFile(args)
		print("time elapsed ",time.clock()-start)



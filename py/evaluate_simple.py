import numpy as np
import sys
import array
import time
import os
import h5py
sys.path.append('src_cython')
start = time.clock()
from evaluateFile import evaluateFile

########################### FIBSEM ######################################
hdf5_gt_file = '/groups/turaga/home/turagas/data/FlyEM/fibsem_medulla_7col/tstvol-520-1-h5/groundtruth_seg_thick.h5'
hdf5_aff_file = '/groups/turaga/home/turagas/data/FlyEM/fibsem_medulla_7col/tstvol-520-1-h5/groundtruth_aff.h5'
hdf5_pred_file = '/tier2/turaga/turagas/research/pygt_models/fibsem5/test_out_0.h5' #'',0,2,3,4,6

# threshes
#threshes = [100+i*100 for i in range(0,10)]+[i*1000 for i in range(2,11)]+[i*10000 for i in range(2,11)] # 100...1,000...100,000
threshes = [i*10 for i in range(0,16)]
print threshes

# funcs
#funcs = ['linear','square','fel','threshold','watershed','lowhigh']
# funcs = ['linear','square','fel','threshold']
funcs = ['lowhigh']

save_segs = False

# output folder
out = 'out/fibsem5_0-10-150/'

evaluateFile([hdf5_gt_file,hdf5_pred_file,threshes,funcs,save_segs,out])

end = time.clock()
print "time elapsed ",end-start
#groundtruth seg: /groups/turaga/home/turagas/data/FlyEM/fibsem_medulla_7col/tstvol-520-1-h5/
#aff seg:
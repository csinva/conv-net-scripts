import numpy as np
import sys
import array
import time
import os
import h5py
sys.path.append('src_cython')
from mainDefs import eval
from multiprocessing import Pool
start = time.clock()
from evaluateFile import evaluateFile

########################### FIBSEM ARGS ######################################
# files
hdf5_gt_file = '/groups/turaga/home/turagas/data/FlyEM/fibsem_medulla_7col/tstvol-520-1-h5/groundtruth_seg_thick.h5'
hdf5_aff_file = '/groups/turaga/home/turagas/data/FlyEM/fibsem_medulla_7col/tstvol-520-1-h5/groundtruth_aff.h5'
hdf5_pred_file = '/tier2/turaga/turagas/research/pygt_models/fibsem5/test_out_0.h5'
hdf5_pred_file_pre = '/tier2/turaga/turagas/research/pygt_models/fibsem'
hdf5_pred_file_post = '/test_out_0.h5'
dirs = ['','2','3','4','6']

# threshes
threshes = [100+i*100 for i in range(0,10)]+[i*1000 for i in range(2,11)]+[i*10000 for i in range(2,11)] # 100...1,000...100,000
# threshes = [100+i*300 for i in range(0,4)]+[i*3000 for i in range(1,4)]+[i*30000 for i in range(2,4)] # 100...1,000...100,000
# threshes = [i*10 for i in range(0,21)]+ [i*25000 for i in range(1,11)]
print threshes

# funcs
#funcs = ['linear','square','fel','threshold','watershed','lowhigh']
funcs = ['linear','square','fel','threshold']

# output folder
out = 'out/fibsem'
###############################################################################

# make array of args
numWorkers = len(dirs)
pred_arr = [hdf5_pred_file_pre+dirs[i]+hdf5_pred_file_post for i in range(numWorkers)]
out_arr = [out+dirs[i]+'/' for i in range(numWorkers)]
argsArr=[]
save_segs=False
for i in range(numWorkers):
    argsArr.append([hdf5_gt_file, pred_arr[i], threshes, funcs,save_segs,out_arr[i]])
print argsArr
# parallel call
p = Pool(numWorkers)
print "Parallel Pool:",numWorkers
p.map(evaluateFile,argsArr)

end = time.clock()
print "time elapsed ",end-start



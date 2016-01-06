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
def evaluateFile(args):
    hdf5_gt_file,hdf5_pred_file,threshes,out = args
    hdf5_gt = h5py.File(hdf5_gt_file, 'r')
    hdf5_aff = h5py.File(hdf5_aff_file, 'r')
    gt = np.asarray(hdf5_gt[hdf5_gt.keys()[0]],dtype='uint32')
    aff = np.asarray(hdf5_aff[hdf5_aff.keys()[0]],dtype='float32')
    aff = aff.transpose(3,2,1,0)
    dims = np.array(aff.shape,dtype='uint32')
    print 'dims:',aff.shape

    # trim gt data - only works for perfect cubes
    gt_data_dimension = gt.shape[0]
    data_dimension = aff.shape[1]
    if gt_data_dimension != data_dimension:
        print("Data dimension do not match. Clip the GT borders.")
        padding = (gt_data_dimension - data_dimension) / 2
        gt = gt[padding:(-1*padding),padding:(-1*padding),padding:(-1*padding)]
        print"New GT data shape :",gt.shape

    # evaluate call
    print "gt shape:",gt.shape
    print "aff shape:",aff.shape
    eval(gt,aff,threshes,out)

    end = time.clock()
    print "time elapsed ",end-start

########################### FIBSEM ARGS ######################################
# files
hdf5_gt_file = '/groups/turaga/home/turagas/data/FlyEM/fibsem_medulla_7col/tstvol-520-1-h5/groundtruth_seg_thick.h5'
hdf5_aff_file = '/groups/turaga/home/turagas/data/FlyEM/fibsem_medulla_7col/tstvol-520-1-h5/groundtruth_aff.h5'
hdf5_pred_file = '/tier2/turaga/turagas/research/pygt_models/fibsem5/test_out_0.h5'
hdf5_pred_file_pre = '/tier2/turaga/turagas/research/pygt_models/fibsem'
hdf5_pred_file_post = '/test_out_0.h5'

# threshes
threshes = [100+i*100 for i in range(0,10)]+[i*1000 for i in range(2,11)]+[i*10000 for i in range(2,11)] # 100...1,000...100,000
# threshes = [100+i*300 for i in range(0,4)]+[i*3000 for i in range(1,4)]+[i*30000 for i in range(2,4)] # 100...1,000...100,000
# threshes = [i*10 for i in range(0,21)]+ [i*25000 for i in range(1,11)]
print threshes

# output folder
out = 'out/fibsem'

###############################################################################

# evaluateFile(hdf5_gt_file,hdf5_pred_file,threshes,out)

# make array of args
dirs = ['','2','3','4','6']
numWorkers = len(dirs)
pred_arr = [hdf5_pred_file_pre+dirs[i]+hdf5_pred_file_post for i in range(numWorkers)]
out_arr = [out+dirs[i]+'/' for i in range(numWorkers)]
argsArr=[]
for i in range(numWorkers):
    argsArr.append([hdf5_gt_file, pred_arr[i], threshes, out_arr[i]])
print argsArr
# parallel call
p = Pool(numWorkers)
print "Parallel Pool:",numWorkers
p.map(evaluateFile,argsArr)



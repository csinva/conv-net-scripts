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

def evaluateFile(hdf5_gt_file,hdf5_pred_file,threshes,funcs,save_segs,out):
    hdf5_gt = h5py.File(hdf5_gt_file, 'r')
    hdf5_aff = h5py.File(hdf5_pred_file, 'r')
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
    if not os.path.exists(out):
        os.makedirs(out)
    f = open(out+'info.txt', 'w')
    f.write('gt: '+hdf5_gt_file+'\n')
    f.write('pred: '+hdf5_pred_file+'\n')
    f.write('pred_dims: '+np.array_str(dims))
    f.close()
    eval(gt,aff,threshes,funcs,save_segs,out)

def test():
    print("testing...\n\n")


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

evaluateFile(hdf5_gt_file,hdf5_pred_file,threshes,funcs,save_segs,out)

end = time.clock()
print "time elapsed ",end-start
#groundtruth seg: /groups/turaga/home/turagas/data/FlyEM/fibsem_medulla_7col/tstvol-520-1-h5/
#aff seg:
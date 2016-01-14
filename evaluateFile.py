import numpy as np
import sys
import time
import os
import h5py
sys.path.append('src_cython')
from mainDefs import eval
start = time.clock()

def evaluateFile(args):
    hdf5_gt_file,hdf5_pred_file,threshes,funcs,save_segs,out = args
    hdf5_gt = h5py.File(hdf5_gt_file, 'r')
    hdf5_aff = h5py.File(hdf5_pred_file, 'r')
    gt = np.asarray(hdf5_gt[hdf5_gt.keys()[0]],dtype='uint32')
    aff = np.asarray(hdf5_aff[hdf5_aff.keys()[0]],dtype='float32')
    aff = np.transpose(aff,(1,2,3,0))

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

    # change both to fortran order
    gt = np.array(gt,order='F')
    aff = np.array(aff,order='F')

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

def averageFiles(hdf5_pred_files,outputFileName): #array of pred files, average them
    hdf5_aff = h5py.File(hdf5_pred_files[0]+'.h5', 'r')
    aff = np.asarray(hdf5_aff[hdf5_aff.keys()[0]],dtype='float32')

    # loop over and average preds
    for i in range(1,len(hdf5_pred_files)):
        print('fibsem'+hdf5_pred_files[i])
        hdf5_aff = h5py.File(hdf5_pred_files[i]+'.h5', 'r')
        aff += np.asarray(hdf5_aff[hdf5_aff.keys()[0]],dtype='float32')
    aff = aff/len(hdf5_pred_files)
    f = h5py.File(outputFileName,'w')
    f.create_dataset("main",data=aff)


if __name__ == "__main__":
    print "running evaluateFile main..."
    hdf5_gt_file = '/groups/turaga/home/turagas/data/FlyEM/fibsem_medulla_7col/tstvol-520-2-h5/groundtruth_seg_thick.h5'
    #hdf5_aff_file = '/groups/turaga/home/turagas/data/FlyEM/fibsem_medulla_7col/tstvol-520-1-h5/groundtruth_aff.h5'
    #hdf5_pred_file = '/tier2/turaga/singhc/output_10000/tstvol-1_5.h5'

    threshes = [i*2000 for i in range(1,6)]+[i*20000 for i in range(2,16)] # 100...1,000...100,000
    funcs = ['square'] # ['linear','square','threshold','watershed','lowhigh']
    out = 'data_tier2/test/out/fibsem'
    iters = [10000,20000,80000,200000] #[10000,30000,50000,70000,100000,200000]
    strs = ["2","3","4","5","6"]
    files = ['/tier2/turaga/singhc/train/output_70000/tstvol-2_' +strs[i] for i in range(len(strs))]
    '''
    for i in range(len(strs)):
        evaluateFile([hdf5_gt_file,files[i]+'.h5',threshes,funcs,True,out+strs[i]+'_10000/'])
    '''
    for iter in range(len(iters)):
        files = ['/tier2/turaga/singhc/train/output_'+str(iters[iter])+'/tstvol-1_' +strs[i] for i in range(len(strs))]
        averageFiles(files,'/tier2/turaga/singhc/train/output_'+str(iters[iter])+'/tstvol-1_ave.h5')



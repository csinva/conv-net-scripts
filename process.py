from __future__ import print_function
import sys, os, math
import h5py
import numpy as np
import time
from numpy import float32, int32, uint8, dtype
from os.path import join
# Load PyGreentea - this block only works inside of process
pygt_path = '/groups/turaga/home/singhc/caffe_v1/PyGreentea' # Relative path to where PyGreentea resides
sys.path.append(pygt_path)
import PyGreentea as pygt
sys.path.append('src_cython')
from evaluateFile import evaluateFile

def process(iter):
	print('processing...',iter)
	# Load the datasets
	path = '/groups/turaga/home/turagas/data/FlyEM/fibsem_medulla_7col/'
	# Test set
	test_dataset = []

	test_dataset.append({})
	dname = 'tstvol-520-1-h5'
	test_dataset[-1]['name'] = dname
	h5im = h5py.File(join(path,dname,'img_normalized.h5'),'r')
	h5im_n = pygt.normalize(np.asarray(h5im[h5im.keys()[0]]).astype(float32), -1, 1)
	test_dataset[-1]['data'] = h5im_n

	'''
	test_dataset.append({})
	dname = 'tstvol-520-2-h5'
	test_dataset[-1]['name'] = dname
	h5im = h5py.File(join(path,dname,'img_normalized.h5'),'r')
	h5im_n = pygt.normalize(np.asarray(h5im[h5im.keys()[0]]).astype(float32), -1, 1)
	test_dataset[-1]['data'] = h5im_n
	'''

	# Set devices
	test_device = 0
	print('Setting devices...')
	pygt.caffe.set_mode_gpu()
	pygt.caffe.set_device(test_device)
	# pygt.caffe.select_device(test_device, False)

	# Load model
	basename = '/groups/turaga/home/turagas/research/caffe_v1/pygt_models/fibsem4/'
	proto = basename + 'net_test.prototxt'
	model = basename + 'net_iter_'+str(20000)+'.caffemodel'
	print('Loading model...')
	net = pygt.caffe.Net(proto, model, pygt.caffe.TEST)

	# Process
	print('Processing...')
	pygt.process(net,test_dataset)
	print('Processing Complete')

start = time.clock()
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
#funcs = ['linear','square','fel','threshold','watershed','lowhigh']
funcs = ['linear','square','fel','threshold']
#funcs = ['lowhigh']

save_segs = False

# output folder
outs = ['out/fibsem_iter_'+str(dirs[i])+'/' for i in range(len(dirs))]
print(outs)

for iter_idx in range(len(dirs)):
	process(dirs[iter_idx])
	args = [hdf5_gt_file,hdf5_pred_file,threshes,funcs,save_segs,outs[iter_idx]]
	evaluateFile(args)

end = time.clock()
print("time elapsed ",end-start)



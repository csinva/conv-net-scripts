from __future__ import print_function
import sys, os, math
import h5py
import numpy as np
from numpy import float32, int32, uint8, dtype
from os.path import join
sys.path.append('/groups/turaga/home/singhc/caffe_v1/PyGreentea') # Relative path to where PyGreentea resides
import PyGreentea as pygt

def processFile(basename,iter,filename1 = "tstvol-520-1-h5"):
	print('processing...',iter)
	# Load the datasets
	path = '/groups/turaga/home/turagas/data/FlyEM/fibsem_medulla_7col/'
	# Test set
	test_dataset = []

	test_dataset.append({})
	dname = filename1
	test_dataset[-1]['name'] = dname
	h5im = h5py.File(join(path,"tstvol-520-1-h5",'img_normalized.h5'),'r')
	h5im_n = pygt.normalize(np.asarray(h5im[h5im.keys()[0]]).astype(float32), -1, 1)
	test_dataset[-1]['data'] = h5im_n

	''' Don't use second test set yet
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
	proto = basename + 'net_test.prototxt'
	model = basename + 'net_iter_'+str(iter)+'.caffemodel'
	print('Loading model...')
	net = pygt.caffe.Net(proto, model, pygt.caffe.TEST)

	# Process
	print('Processing...')
	pygt.process(net,test_dataset)
	print('Processing Complete')
import numpy as np
import sys
import time
import os
import h5py
import os.path as op
import matplotlib.cm as cm
from matplotlib.widgets import Slider, Button, RadioButtons
import matplotlib.pyplot as plt
import array

sys.path.append('..')
from zwatershed import *

from visualization.visualize_funcs import *

path_to_folder = '/Users/chandansingh/drive/janelia/conv_net_scripts/'
path_to_data = path_to_folder + 'data/'

threshes = [2000]  # for i in range(1,6)]+[i*20000 for i in range(2,16)] # 100...1,000...100,000
iters = [10000]
strs = ["2"]
hdf5_gt_file = path_to_data + 'groundtruth_seg_thick.h5'  # /groups/turaga/home/turagas/data/FlyEM/fibsem_medulla_7col/tstvol-520-1-h5/groundtruth_seg_thick.h5'
hdf5_pred_file = path_to_data + 'tstvol-1_2.h5'  # /tier2/turaga/singhc/train/output_200000/tstvol-1_2.h5'
out = path_to_data + 'out/'  # '/groups/turaga/home/singhc/evaluation/out/'
save_threshes = threshes
rand = 0
p_small = 200

hdf5_gt = h5py.File(hdf5_gt_file, 'r')
hdf5_aff = h5py.File(hdf5_pred_file, 'r')
gt = np.asarray(hdf5_gt[hdf5_gt.keys()[0]], dtype='uint32')
aff = np.asarray(hdf5_aff[hdf5_aff.keys()[0]], dtype='float32')
aff = aff[:, p_small:(-1 * p_small), p_small:(-1 * p_small), p_small:(-1 * p_small)]

dims = np.array(aff.shape, dtype='uint32')
# trim gt data - only works for perfect cubes
gt_data_dimension = gt.shape[0]
data_dimension = aff.shape[1]
if gt_data_dimension != data_dimension:
    padding = (gt_data_dimension - data_dimension) / 2
    gt = gt[padding:(-1 * padding), padding:(-1 * padding), padding:(-1 * padding)]

nhood = mknhood3d(1)
print "calculating edgelist..."
node1, node2, edge_affs = affgraph_to_edgelist(aff, nhood)
print "len node1",len(node1)
print "n1,n2,edge",min(node1), max(node1), min(node2), max(node2), min(edge_affs), max(edge_affs)
print "calling watershed..."
zwatershed_and_metrics_edge(gt, np.array(node1,dtype='uint32'), np.array(node2,dtype='uint32'), np.array(edge_affs), threshes, save_threshes)
'''
segs, rand = zwatershed_and_metrics_edge(gt, node1, node2, edge_affs, threshes, save_threshes)
# segs, rand = zwatershed_and_metrics(gt, aff, threshes, save_threshes)
# segs = zwatershed(aff, threshes)
# rand = zwatershed_and_metrics_h5(gt, aff, threshes, save_threshes, out)
# zwatershed_h5(aff, threshes, out)

print rand
seg0 = segs[0]
print seg0.shape
NUM = 22
NUM2 = 70
print sum(seg0.flatten()) - 26763581
(x, y, z) = (sum(seg0[0:NUM2, NUM, NUM]) - 458, sum(seg0[NUM, 0:NUM2, NUM]) - 239, sum(seg0[NUM, NUM, 0:NUM2]) - 474)
print x, y, z
'''

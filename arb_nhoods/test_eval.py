import numpy as np
import h5py
import datetime

np.set_printoptions(precision=4)
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
import matplotlib

sys.path.append('arb_nhoods')
import twatershed as tw

sys.path.append('../')
from visualization.visualize_funcs import *

path_to_folder = '/Users/chandansingh/drive/janelia/conv_net_scripts/'
path_to_data = path_to_folder + 'data/'

threshes = [500, 1000, 2000]  # for i in range(1,6)]+[i*20000 for i in range(2,16)] # 100...1,000...100,000
iters = [10000]
strs = ["2"]
hdf5_gt_file = path_to_data + 'groundtruth_seg_thick.h5'  # /groups/turaga/home/turagas/data/FlyEM/fibsem_medulla_7col/tstvol-520-1-h5/groundtruth_seg_thick.h5'
hdf5_pred_file = path_to_data + 'tstvol-1_2.h5'  # /tier2/turaga/singhc/train/output_200000/tstvol-1_2.h5'
out = path_to_data + 'out/'  # '/groups/turaga/home/singhc/evaluation/out/'
save_threshes = threshes
rand = 0
p_small = 150

hdf5_gt = h5py.File(hdf5_gt_file, 'r')
hdf5_aff = h5py.File(hdf5_pred_file, 'r')
gt = np.asarray(hdf5_gt[hdf5_gt.keys()[0]], dtype='int32')
aff = np.asarray(hdf5_aff[hdf5_aff.keys()[0]], dtype='float32')
aff = aff[:, p_small:(-1 * p_small), p_small:(-1 * p_small), p_small:(-1 * p_small)]

dims = np.array(aff.shape, dtype='uint32')
print('dims:', aff.shape)

# trim gt data - only works for perfect cubes
gt_data_dimension = gt.shape[0]
data_dimension = aff.shape[1]
if gt_data_dimension != data_dimension:
    print("data dimension do not match - clipping GT borders.")
    padding = (gt_data_dimension - data_dimension) / 2
    gt = gt[padding:(-1 * padding), padding:(-1 * padding), padding:(-1 * padding)]
    print("New GT data shape :", gt.shape)

nhood = tw.mknhood3d(1)
node1, node2, edge_affs = tw.affgraph_to_edgelist(aff, nhood)
node1 = np.array(node1, dtype='int32')
node2 = np.array(node2, dtype='int32')
THRESH = .9999  # higher is more connected
# print node1[0:40]
# print node2[0:40]
edge_affs_thresh = np.array(edge_affs <= THRESH, dtype='int32')
print "edge_affs: ", edge_affs_thresh
print "percent below thresh", sum(edge_affs <= THRESH) / float(len(edge_affs))
seg_cc, _ = tw.connected_components(int(np.size(gt)), node1, node2, edge_affs_thresh)
print "num segs seg_cc", max(seg_cc.flatten())
print "num segs gt", max(gt.flatten())
# seg_cc = gt
seg, seg_sizes = tw.marker_watershed(seg_cc.flatten(), node1, node2, edge_affs, threshes)
# print "output_seg_len", len(seg_sizes)
print "num segs seg_cc", max(seg.flatten())
print "num segs gt", max(gt.flatten())
seg = seg.reshape(gt.shape)

aff = aff.transpose(1, 2, 3, 0)
display_arbitrary_seg(gt, aff, seg_cc.reshape(gt.shape))
display_arbitrary_seg(gt, aff, seg)

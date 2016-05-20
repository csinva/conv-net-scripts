import numpy as np
import h5py
import datetime

np.set_printoptions(precision=4)
import twatershed as tw

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
p_small = 200

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

print("shapes: ", gt.shape, aff.shape)
nhood = tw.mknhood3d(1)
print("nhood: ", nhood)
node1, node2, edge_affs = tw.affgraph_to_edgelist(aff, nhood)
print("edgelist: ")
for i in range(min(len(node1), 10)):
    print(node1[i], node2[i], edge_affs[i])

seg, seg_sizes = tw.marker_watershed(gt.flatten(), node1, node2, edge_affs)
seg = seg.reshape(gt.shape)
print seg.shape, seg[0:10]
print seg_sizes[0:10]
aff = aff.transpose(1, 2, 3, 0)
display_arbitrary_seg(gt, aff, seg)
raw = gt
label = aff

plt.plot(range(10))
plt.show()
'''
cmap = matplotlib.colors.ListedColormap(np.vstack(((0, 0, 0), np.random.rand(255, 3))))
fig = plt.figure(figsize=(20, 10))
fig.set_facecolor('white')
ax1, ax2, ax3 = fig.add_subplot(1, 3, 1), fig.add_subplot(1, 3, 2), fig.add_subplot(1, 3, 3)

fig.subplots_adjust(left=0.2, bottom=0.25)

# Image is grayscale
ax1.imshow(raw[1, :, :], cmap=cm.Greys_r)
ax1.set_title('Raw Image')

ax2.imshow(label[1, :, :, :])
ax2.set_title('Groundtruth')

ax3.imshow(seg[1, :, :], cmap=cmap)
ax3.set_title('Seg')

plt.show()
'''

'''
marker_watershed(np.ndarray[int, ndim=1] marker,
                     np.ndarray[int, ndim=1] node1,
                     np.ndarray[int, ndim=1] node2,
                     np.ndarray[float, ndim=1] edgeWeight,
                     int sizeThreshold=1):
                     '''

'''
def affgraph_to_edgelist(aff, nhood):
    node1, node2 = nodelist_like(aff.shape[1:], nhood)
    return (node1.ravel(), node2.ravel(), aff.ravel())
'''
# segs, rand = zwatershed_and_metrics(gt, aff, threshes, save_threshes)
# segs = zwatershed(aff, threshes)
# rand = zwatershed_and_metrics_h5(gt, aff, threshes, save_threshes, out)
# zwatershed_h5(aff, threshes, out)

# print rand
# print sum(segs[0][0][0:100][0])

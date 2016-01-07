import os
import matplotlib.cm as cm
import numpy as np
from matplotlib.widgets import Slider, Button, RadioButtons
import matplotlib.pyplot as plt
import h5py
import array
import matplotlib

### display an im
cmap = matplotlib.colors.ListedColormap(np.vstack(((0,0,0),np.random.rand(255000,3))))
f = open("/groups/turaga/home/singhc/evaluation/out/fibsem5_0-10-150/lowhigh/vout.0.000010.0.999999.out")
pred = array.array("I")
pred.fromfile(f, 432*432*432)
pred = np.array(pred).reshape((432,432,432))
print 'pred.shape:',pred.shape
print 'max:',np.max(pred)
print 'min:',np.min(pred)
f.close()

fig = plt.figure(figsize=(20,10))
fig.set_facecolor('white')
ax1 = fig.add_subplot(1,3,1)
seg = np.zeros((432,432)).astype(np.int)
seg[:,:]=pred[:,:,1]

im3 = ax1.imshow(seg==10,cmap=cmap)
ax1.set_title('Predictions')
plt.show()



### display gt seg
hdf5_gt_file = '/groups/turaga/home/turagas/data/FlyEM/fibsem_medulla_7col/tstvol-520-1-h5/groundtruth_seg_thick.h5'
hdf5_gt = h5py.File(hdf5_gt_file, 'r')
gt = np.asarray(hdf5_gt[hdf5_gt.keys()[0]],dtype='uint32')

fig = plt.figure(figsize=(20,10))
fig.set_facecolor('white')
ax1 = fig.add_subplot(1,3,1)
im_ = np.zeros((520,520)).astype(np.int)
im_[:,:]=gt[:,:,1]
im3 = ax1.imshow(im_)
ax1.set_title('Predictions')
plt.show()
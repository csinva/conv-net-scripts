import numpy as np
import h5py
import sys
import time
import os
import h5py
import os.path as op

'''
pred_file_rand = '/nobackup/turaga/singhc/rand_affs/333k_rand.h5'
# np.random.seed(seed=703858704)
# affs = np.random.rand(3,3000,3000,3000).astype('float32')

if op.isfile(pred_file_rand):
    os.remove(pred_file_rand)
f = h5py.File(pred_file_rand, 'a')
dset = f.create_dataset("main",(3,3000,3000,3000), chunks=True)
for i in range(3):
    for j in range(3000):
        print i,j
        dset[i,j,:,:] = np.random.random_sample((3000,3000))
# f['main'] = affs
f.close()
pred_file = pred_file_rand
'''
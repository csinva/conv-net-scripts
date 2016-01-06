import numpy as np
import sys
import array
import time
import os
import h5py
sys.path.append('src_cython')
from mainDefs import eval

start = time.clock()
dims = np.array([256,256,256,3],dtype='uint32')

########################### RAND ERROR ######################################
# gt
f = open("/groups/turaga/home/singhc/evaluation/data/gt.in")
a = array.array("I")
a.fromfile(f, dims[0]*dims[1]*dims[2])
print(a[0:10])
f.close()

# affs
f = open("/groups/turaga/home/singhc/evaluation/data/ws_test_256.raw")
aff = array.array("f")
aff.fromfile(f, dims[0]*dims[1]*dims[2]*dims[3])
print(aff[0:10])
f.close()

# threshes
threshes = [100+i*100 for i in range(0,10)]+[i*1000 for i in range(2,11)]+[i*10000 for i in range(2,11)] # 100...1,000...100,000
# threshes = [100+i*300 for i in range(0,4)]+[i*3000 for i in range(1,4)]+[i*30000 for i in range(2,4)] # 100...1,000...100,000
# threshes = [i*10 for i in range(0,21)]+ [i*25000 for i in range(1,11)]
print threshes

# output folder
out = 'out/test_full/'

gt = np.frombuffer(a,dtype='uint32').reshape(dims[0:3])
affs = np.frombuffer(aff,dtype='float32').reshape(dims)
eval(gt,affs,threshes,out)
import numpy as np
import sys
import array
import time
import h5py
sys.path.append('src_cython')
from mainDefs import test_py, eval

start = time.clock()
dims = np.array([72,936,936,11],dtype='uint32')

f = open("/groups/turaga/turagalab/greentea_experiments/project_data/labels_id_cropped.raw")
a = array.array("I") 
a.fromfile(f, dims[0]*dims[1]*dims[2])
print(a[0:10])
f.close()

f = open("/groups/turaga/home/turagas/research/caffe_neural_models/dataset_07/processed/train_euclid.raw")
aff = array.array("f")
aff.fromfile(f, dims[0]*dims[1]*dims[2]*dims[3])
print(aff[0:10])
f.close()


test_py()

print('reshaping gt...')
gt = np.frombuffer(a,dtype='uint32').reshape(dims[0:3])
print('reshaping affs...')
affs = np.frombuffer(aff,dtype='float32').reshape(dims)
print('calling c...')
eval(gt,affs)

end = time.clock()
print (end-start)*60,"seconds elapsed"

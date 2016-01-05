import numpy as np
import sys
import h5py
sys.path.append('src_cython')
from mainDefs import test_py, eval

f = open("/groups/turaga/turagalab/greentea_experiments/project_data/labels_id_cropped.raw")
dims = np.array([72,936,936],dtype='uint32')
import array
a = array.array("I")  # L is the typecode for uint32
a.fromfile(f, dims[0]*dims[1]*dims[2])
print(a[0:10])
f.close()



test_py()

gt = np.array(a,dtype='uint32').reshape(dims)

eval(gt)

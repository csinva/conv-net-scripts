import numpy as np
import sys
import h5py
sys.path.append('src_cython')
from mainDefs import test_py, eval

f = open("/groups/turaga/turagalab/greentea_experiments/project_data/labels_id_cropped.raw")
dims = [72,936,936]
import array
a = array.array("I")  # L is the typecode for uint32
a.fromfile(f, dims[0]*dims[1]*dims[2])
print(a[0:10])

# volume_ptr<uint32_t> gt_ptr = read_volumes<uint32_t>("/groups/turaga/turagalab/greentea_experiments/project_data/labels_id_cropped.raw", 72, 936, 936);

test_py()

eval()

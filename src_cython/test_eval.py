import numpy as np
import sys
import time
import os
import h5py
import os.path as op
import matplotlib
import matplotlib.cm as cm
import matplotlib.pyplot as plt
from multiprocessing import Pool
from itertools import product
from par_funcs import *
sys.path.append('..')
V = 20

# -------------------------------- parameters ---------------------------------------
# pred_file = '/groups/turaga/home/turagas/research/caffe_v2/processed/bock2/120000/sample_A_x1_y1_z1_xy1.h5'
pred_file = '/groups/turaga/home/turagas/research/caffe_v2/processed/bock2/120000/cutout_3k.h5'
# pred_file = '/groups/turaga/home/turagas/turagalab/FROM_TIER2/singhc/train/output_200000/tstvol-1_2.h5'
out_folder = '/nobackup/turaga/singhc/3k_300/' # _3k _vol has full, max_len = 300
outname = out_folder+'out.h5'
NUM_WORKERS = 32
MAX_LEN = 300

t1 = time.time()
partition_data = partition_subvols(pred_file,out_folder,max_len=MAX_LEN)
print "time",time.time()-t1,"secs"

t1 = time.time()
x = eval_with_par_map(partition_data[0],NUM_WORKERS)
print "time",time.time()-t1,"secs"

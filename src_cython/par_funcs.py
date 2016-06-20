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

def partition_subvols(pred_file,out_folder,max_len):
    f = h5py.File(pred_file, 'r')
    preds = f['main']
    
    def dim_to_name(start):
        return str(start[0])+'_'+str(start[1])+'_'+str(start[2])+'_vol0/'
    dims = np.array(preds.shape[1:])
    num_vols = np.array([int(x/max_len)+1 for x in dims])
    deltas = dims/num_vols
    print "dims",dims
    print "num_vols",num_vols
    print "deltas",deltas
    starts,ends = [],[]
    for x in range(num_vols[0]):
        for y in range(num_vols[1]):
            for z in range(num_vols[2]):
                starts.append((x,y,z)*deltas - [3,3,3] +3*np.array([x==0,y==0,z==0]))
                extra = 3*np.array([x==num_vols[0]-1,y==num_vols[1]-1,z==num_vols[2]-1],dtype='int')
                ends.append((x,y,z)*deltas + deltas + [1,1,1]+extra +[3,3,3])
    args = []
    for i in range(len(starts)):
        s,e = starts[i],ends[i]
        args.append((pred_file,s,e,out_folder+dim_to_name(s)))    
    return args,starts,ends
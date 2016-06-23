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
# pred_file = '/groups/turaga/home/turagas/research/caffe_v2/processed/bock2/120000/cutout_3k.h5'
# pred_file = '/groups/turaga/home/turagas/turagalab/FROM_TIER2/singhc/train/output_200000/tstvol-1_2.h5'
# out_folder = '/nobackup/turaga/singhc/3k_200/' # _3k _vol has full, max_len = 300
pred_file = '/nobackup/turaga/singhc/rand_affs/3k_rand.h5'
out_folder = '/nobackup/turaga/singhc/rand_3k_200/' # _3k _vol has full, max_len = 300
outname = out_folder+'out.h5'
NUM_WORKERS = 32
MAX_LEN = 200

t1 = time.time()
partition_data = partition_subvols(pred_file,out_folder,max_len=MAX_LEN)
print "time",time.time()-t1,"secs"

def stitch_and_save(partition_data,outname):
    args,starts,ends,dims,num_vols = partition_data
    (X,Y,Z) = num_vols #(1,1,2) # num_vols
    # X,Y,Z = 1,1,2
    if not outname.endswith('.h5'):
        outname += '.h5'
    if op.isfile(outname):
        os.remove(outname)
    f = h5py.File(outname, 'a')
    t1 = time.time()

    dset_seg = f.create_dataset('seg', dims, dtype='uint64', chunks=True)
    # dset_seg = f.create_dataset('seg', (110,220,220), dtype='uint64', chunks=True)
    inc,re,merges,rgs,i_arr=0,{},{},{},[]

    print "create dataset time",time.time()-t1,"secs"
    t1 = time.time()
    t2 = time.time()
    # calc all merges, set dset_seg, rg with incrementing
    for x,y,z in product(range(X),range(Y),range(Z)):
        i = x*num_vols[1]*num_vols[2]+y*num_vols[2]+z
        print "\ti =",str(i-1),"time",time.time()-t1,"secs"
        t1 = time.time()
        i_arr.append(i)
        s,e = starts[i],ends[i]
        basic_file = h5py.File(args[i][-1]+'basic.h5','r')
        seg,rg = np.array(basic_file['seg']),np.array(basic_file['rg'])
        seg[seg!=0]+=inc
        rg[:,:2] += inc
        rgs[i] = rg
        inc = np.max(seg)
        print "\ti,x,y,z",i,x,y,z
        if not z==0: 
            re,merges = calc_merges(edge_mins=dset_seg[s[0]:e[0],s[1]:e[1],s[2]+3],edge_maxes=seg[:,:,3], re=re, merges=merges)
        if not y==0:
            re,merges = calc_merges(edge_mins=dset_seg[s[0]:e[0],s[1]+3,s[2]:e[2]],edge_maxes=seg[:,3,:],re=re,merges=merges)
        if not x==0:
            re,merges = calc_merges(edge_mins=dset_seg[s[0]+3,s[1]:e[1],s[2]:e[2]],edge_maxes=seg[3,:,:],re=re, merges=merges)
        dset_seg[s[0]:e[0],s[1]:e[1],s[2]:e[2]] = seg[:,:,:]
#         plt.imshow(dset_seg[0, :, :], cmap=cmap)
#         plt.show()
    print "calculate all merges",time.time()-t2,"secs"

    t1 = time.time()
    merges_filtered = filter_merges(merges)
#     plt.imshow(dset_seg[V, :, :], cmap=cmap)
#     plt.show()
    print "filter merges time",time.time()-t1,"secs"
    t1 = time.time()
    
    rgs = merge(merges_filtered,rgs,i_arr,args,f,max_val=inc)
    print "merge time",time.time()-t1,"secs"
    t1 = time.time()
    
#     plt.imshow(dset_seg[V, :, :], cmap=cmap)
#     plt.show()
    
    seg_sizes = calc_seg_sizes(f)
    print "calc_seg_sizes time",time.time()-t1,"secs"
    t1 = time.time()

    # save
    f = h5py.File(outname, 'a')
    dset_seg_sizes = f.create_dataset('seg_sizes', data=np.array(seg_sizes))
    for key in rgs:
        rg_dset = f.create_dataset('rg_'+str(key),data=np.array(rgs[key]))
    dset_starts = f.create_dataset('starts',data=np.array(starts))
    dset_ends = f.create_dataset('ends',data=np.array(ends))                               
    f.close()
    print "save time",time.time()-t1,"secs"
    t1 = time.time()
stitch_and_save(partition_data,outname)
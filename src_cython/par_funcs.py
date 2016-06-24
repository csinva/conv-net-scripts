import numpy as np
import h5py
from zwatershed import *
import os.path as op
from itertools import product
from multiprocessing import Pool
from edgelist_methods import *

######################      partition subvols     ######################

def partition_subvols(pred_file,out_folder,max_len,nhood=None):
    f = h5py.File(pred_file, 'r')
    preds = f['main']
    def dim_to_name(start):
        return str(start[0])+'_'+str(start[1])+'_'+str(start[2])+'_vol/'
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
                ends.append((x,y,z)*deltas + deltas + [1,1,1] + extra + [3,3,3])
    args = []
    for i in range(len(starts)):
        s,e = starts[i],ends[i]      
        args.append((pred_file,s,e,out_folder+dim_to_name(s),nhood))    
    return args,starts,ends,dims,num_vols
    
def unoverlap(s_old,e_old,dims):
    s_old,e_old = np.array(s_old), np.array(e_old)
    s,e = [],[]
    for i in range(len(s_old)):
        s.append(np.array(s_old[i])+[4,4,4] - 4*(s_old[i]==0))
        e.append(np.array(e_old[i])-[3,3,3] + 3*(e_old[i]>dims))
    return np.array(s),np.array(e)
        
######################      call watershed     ######################

def zwshed_h5_par(arg):
    (pred_file,s,e,seg_save_path,nhood) = arg
    f = h5py.File(pred_file, 'r')
    preds_small = f['main']
    pred_vol = preds_small[:,s[0]:e[0],s[1]:e[1],s[2]:e[2]]
    def zwatershed_basic_h5_arb(seg_shape, node1, node2, edgeWeight, seg_save_path)
    if nhood==None:
        zwatershed_basic_h5(pred_vol,seg_save_path)
    else:
        seg_shape = preds_small.shape[1:]
        node1, node2, edgeWeight = affgraph_to_edgelist(pre_vol, nhood)
        zwatershed_basic_h5_arb(seg_shape, node1, node2, edgeWeight, seg_save_path)
    print "finished",seg_save_path,"watershed"

def eval_with_par_map(args,num_workers):
    p = Pool(num_workers)
    p.map(zwshed_h5_par, args)
    
# run using: spark-janelia -n 3 lsd -s lsd-example.py
def eval_with_spark(args):
    from pyspark import SparkConf, SparkContext
    conf = SparkConf().setAppName('zwshed')
    sc = SparkContext(conf=conf)
    finish_status = sc.parallelize(args,len(args)).map(zwshed_h5_par).collect()
    print(zip(args,finish_status))

######################      stitch main    #########################

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
        # print "\ti =",str(i-1),"time",time.time()-t1,"secs"
        t1 = time.time()
        i_arr.append(i)
        s,e = starts[i],ends[i]
        basic_file = h5py.File(args[i][-3]+'basic.h5','r')
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

######################      stitch helpers    #########################
def add_or_inc(key_max,key_min,d):
    key = (key_max,key_min)
    if not key in d:
        d[key] = 1
    else:
        d[key] +=1
        
def calc_merges(edge_mins,edge_maxes, re, merges={}):
    edge_mins = edge_mins.ravel()
    edge_maxes = edge_maxes.ravel()
    for j in range(len(edge_mins)):
        edge_min = edge_mins[j]
        edge_max = edge_maxes[j]
        if not edge_min==0 and not edge_max==0 and not edge_max==edge_min: # last condition is unnecessary
            if edge_max in re: # already in map
                old_min = re[edge_max]
                merge_max = max(old_min,edge_min)
                merge_min = min(old_min,edge_min)
                if not merge_max==merge_min:
                    re[merge_max] = merge_min
                    add_or_inc(merge_max,merge_min,merges)
            re[edge_max] = edge_min
            add_or_inc(edge_max,edge_min,merges)
    return re, merges  

def filter_merges(merges):
    COUNT_THRESH = 0
    print "filter_merges..."
    # only keep strongest edges
    renums,count_maxes = {},{}
    for pair in merges:
        count = merges[pair]
        e1,e2 = pair
        if e1 in count_maxes:
            if count > count_maxes[e1]:
                renums[e1] = e2
                count_maxes[e1] = count
        else:
            renums[e1] = e2
            count_maxes[e1] = count
    
    # compress merges
    renums_filtered = {}
    for key in renums:
        val = renums[key]
        if merges[(key,val)] > COUNT_THRESH:
            while val in renums:
                val = renums[val]
            renums_filtered[key] = val
    return renums_filtered
    
def merge(merges_filtered,rgs,i_arr,args,f,max_val=1e5):  
    print "merge"
    # merge segs        
    mp = np.arange(0,max_val+100,dtype='uint64')
    mp[merges_filtered.keys()] = merges_filtered.values()
    for i in i_arr:
        s,e = args[i][1],args[i][2]
        seg = np.array(f['seg'][s[0]:e[0],s[1]:e[1],s[2]:e[2]])
        f['seg'][s[0]:e[0],s[1]:e[1],s[2]:e[2]] = mp[seg]
    # merge rgs
    for key in rgs:
        rg = rgs[key]
        rg_to_renum = rg[:,:2].astype('int')
        rg[:,:2] = mp[rg_to_renum]
        keeps = rg[:,0]<rg[:,1]
        rgs[key] = rg[keeps,:]    
    return rgs

def calc_seg_sizes(f): # there must be at least one background pixel   
    print "calculating seg_sizes..."
    segId,seg_sizes = np.unique(f['seg'],return_counts=True) # this might have to be done in parts
    seg_sizes_proper = np.zeros(segId.max()+1,dtype='uint64')
    seg_sizes_proper[segId] = seg_sizes
    return seg_sizes_proper
    
######################      agglomeration     #########################

def merge_seg_by_thresh(seg,seg_sizes,rg,thresh):
    re = {}
    seg_max = np.max(seg)
    seg_min = np.min(seg)
    print "calculating renums..."
    for i in range(rg.shape[0]):
        n1,n2,w = rg[i,:]
        size = w*w*thresh
        if seg_sizes[n1] < size or seg_sizes[n2] < size:
            re[n2]=n1
            seg_sizes[n1]+=seg_sizes[n2]
            seg_sizes[n2]+=seg_sizes[n1]
    re_filtered = {}
    print "filtering renums..."
    for key in re:
        val = re[key]
        while val in re:
            val = re[val]
        if key < seg_max and val < seg_max:
            re_filtered[key] = val
    
    print "renumbering..."
    mp = np.arange(0,seg_max+1,dtype='uint64')
    mp[re_filtered.keys()] = re_filtered.values()
    seg = mp[seg]
    return seg

def merge_vol_by_thresh(fname_in,fname_out,thresh):
    f = h5py.File(fname_in)
    dims = f['seg'].shape
    print "dims",dims
    starts,ends = f['starts'],f['ends']
    seg_sizes = f['seg_sizes']
    if op.isfile(fname_out):
        os.remove(fname_out)
    g = h5py.File(fname_out)
    seg_out = g.create_dataset("seg",dims, chunks=True)

    for i in range(len(starts)):
        s,e = starts[i],ends[i]
        seg = f['seg'][s[0]:e[0],s[1]:e[1],s[2]:e[2]]
        rg = f['rg_'+str(i)]
        print i,s,e,len(rg)
        seg_merged = merge_seg_by_thresh(seg,seg_sizes,rg,thresh)
        g[s[0]:e[0],s[1]:e[1],s[2]:e[2]] = seg_merged
    f.close()
    g.close()
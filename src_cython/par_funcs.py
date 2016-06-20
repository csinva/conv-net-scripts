import numpy as np
import h5py
from zwatershed import *

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
                ends.append((x,y,z)*deltas + deltas + [1,1,1] + extra + [3,3,3])
    args = []
    for i in range(len(starts)):
        s,e = starts[i],ends[i]
        args.append((pred_file,s,e,out_folder+dim_to_name(s)))    
    return args,starts,ends,dims,num_vols
    
def zwshed_h5_par(arg):
    (pred_file,s,e,seg_save_path) = arg
    f = h5py.File(pred_file, 'r')
    preds_small = f['main']
    pred_vol = preds_small[:,s[0]:e[0],s[1]:e[1],s[2]:e[2]]
    zwatershed_basic_h5(pred_vol,seg_save_path)
    print "finished",seg_save_path,"watershed"
    
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
        if not edge_min==0 and not edge_max==0:
            if not edge_max==edge_min:
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

# merge methods
def filter_merges(merges):
    COUNT_THRESH = 0
    print "filter merges..."
    # only keep strongest edges
    renums = {}
    count_maxes = {}
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
    sum_counts = 0
    for key in merges:
        sum_counts += merges[key]
    renums_filtered = {}
    print "merging numbers,",len(renums.keys()),"keys ... "            
    for key in renums:
        val = renums[key]
        if merges[(key,val)] > COUNT_THRESH:
            while val in renums:
                val = renums[val]
            renums_filtered[key] = val
    return renums_filtered
    
# stitch methods
def merge(merges_filtered,rgs,i_arr,(args,starts,ends),f,max_val=1e5):     
    # merge segs        
    mp = np.arange(0,max_val+1,dtype='uint64')
    mp[merges_filtered.keys()] = merges_filtered.values()
    for i in i_arr:
        arg,s,e = args[i],starts[i],ends[i]
        seg = np.array(f['seg'][s[0]:e[0],s[1]:e[1],s[2]:e[2]])
        f['seg'][s[0]:e[0],s[1]:e[1],s[2]:e[2]] = mp[seg]

    # merge rgs
    for key in rgs:
        rg = rgs[key]
        rg_to_renum = rg[:,:2].astype('int')
        rg[:,:2] = mp[rg_to_renum]
        keeps = rg[:,0]!=rg[:,1]
        rg_filtered = rg[keeps,:]
        rgs[key] = rg_filtered
    
    return rgs

def renum_all(seg,rgs,i_arr,(args,starts,ends),f): # there must be at least one background pixel   
    segId,seg_sizes = np.unique(seg,return_counts=True) # this might have to be done in parts
    renum = np.zeros(segId.max()+1,dtype=np.uint64)
    renum[segId] = np.arange(0,len(segId)+1) 

    for x in range(dims[0]):
        seg = np.array(f['seg'][x,:,:])
        f['seg'][x,:,:] = renum[seg]

    # renumber rgs
    for key in rgs:
        rg = rgs[key]
        rg_to_renum = rg[:,:2].astype('int64')
#         rg[:,:2] = renum[rg_to_renum]
        rgs[key] = rg

    return renum, seg_sizes, rgs
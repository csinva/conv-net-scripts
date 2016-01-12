from __future__ import print_function
import sys, os, math
import time
sys.path.append('/groups/turaga/home/singhc/caffe_v1/PyGreentea') # Relative path to where PyGreentea resides
sys.path.append('src_cython')
from evaluateFile import evaluateFile, averageAndEvaluateFiles
from processFile import processFile

hdf5_gt_file = '/groups/turaga/home/turagas/data/FlyEM/fibsem_medulla_7col/tstvol-520-1-h5/groundtruth_seg_thick.h5' #groundtruth_aff.h5
hdf5_aff_file = '/groups/turaga/home/turagas/data/FlyEM/fibsem_medulla_7col/tstvol-520-1-h5/groundtruth_aff.h5'

# input models
model_base_folder = '/groups/turaga/home/turagas/research/caffe_v1/pygt_models/fibsem'
fibsemFolders = ['5'] #['','2','3','4',,'5','6']
iters = [80000] #10000 - 39000

# output folders
h5OutputFilenames = ["data_tier2/output_10000/"+"tstvol-1_"+fibsemFolders[i] for i in range(len(fibsemFolders))]
randOutputFolder = ['data_tier2/out/fibsem' +fibsemFolders[i]+ '_iter_'+str(iters[j])+'/' for j in range(len(iters)) for i in range(len(fibsemFolders))]

# settings
threshes = [50000 + i*10000 for i in range(10)] #[100+i*100 for i in range(0,10)]+[i*1000 for i in range(2,11)]+[i*10000 for i in range(2,11)] # 100...1,000...100,000
funcs = ['linear','square','threshold'] #'watershed','lowhigh'
save_segs = False


for iter_idx in range(len(iters)):
	for fibsem_idx in range(len(fibsemFolders)):
		start = time.clock()
		processFile(model_base_folder+fibsemFolders[fibsem_idx]+'/',iters[iter_idx],h5OutputFilenames[fibsem_idx])
		evaluateFile([hdf5_gt_file,h5OutputFilenames[fibsem_idx]+'.h5',threshes,funcs,save_segs,randOutputFolder[iter_idx*len(fibsemFolders)+fibsem_idx]])
		#averageAndEvaluateFiles([hdf5_gt_file,h5OutputFilenames, threshes,funcs,save_segs,randOutputFolder[iter_idx*len(fibsemFolders)+fibsem_idx]]) # for averaging
		print("time elapsed ",time.clock()-start)



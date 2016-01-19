from __future__ import print_function
import sys, os, math
import time
sys.path.append('/groups/turaga/home/singhc/caffe_v1/PyGreentea') # Relative path to where PyGreentea resides
sys.path.append('src_cython')
from evaluateFile import evaluateFile, averageFiles
from processFile import processFile

# gt file
train = True # which dataset to evaluate
t = "train" if train else "test"
vol = "1" if train else "2"

# input models
model_base_folder = '/groups/turaga/home/turagas/research/caffe_v1/pygt_models/fibsem'
fibsemFolders = ['2','3','4','5','6']
iters = [10000*i for i in range(1,21)]
hdf5_gt_file = '/groups/turaga/home/turagas/data/FlyEM/fibsem_medulla_7col/tstvol-520-'+vol+'-h5/groundtruth_seg_thick.h5' #groundtruth_aff.h5

# settings
threshes = [i*2000 for i in range(1,6)]+[i*20000 for i in range(2,16)] # default: 100...1,000...100,000
funcs = ['square'] #'linear','threshold','watershed','lowhigh'
save_threshes = [] #threshes
process = True
eval = True

# output folders
h5OutputFilenames = ["data_tier2/"+t+"/output_"+str(iters[j])+"/"+"tstvol-"+vol+"_"+fibsemFolders[i] for j in range(len(iters)) for i in range(len(fibsemFolders))]
randOutputFolder = ['data_tier2/'+t+'/out/fibsem' +fibsemFolders[i]+ '_'+str(iters[j])+'/' for j in range(len(iters)) for i in range(len(fibsemFolders))]


for iter_idx in range(len(iters)):
	for fibsem_idx in range(len(fibsemFolders)):
		'''
		if process:
			processFile(model_base_folder+fibsemFolders[fibsem_idx]+'/',iters[iter_idx],h5OutputFilenames[iter_idx*len(fibsemFolders)+fibsem_idx],train)
		if eval:
			evaluateFile([hdf5_gt_file,h5OutputFilenames[iter_idx*len(fibsemFolders)+fibsem_idx]+'.h5',threshes,funcs,save_threshes,randOutputFolder[iter_idx*len(fibsemFolders)+fibsem_idx]])
		'''
	# this part might not work
	h5_filenames_to_average = ["/tier2/turaga/singhc/"+t+"/output_"+str(iters[iter_idx])+"/"+"tstvol-"+vol+"_"+fibsemFolders[i] for i in range(len(fibsemFolders))]
	out_folder = '/tier2/turaga/singhc/'+t+'/out/fibsemave_'+str(iters[iter_idx])
	if process:
		averageFiles(h5_filenames_to_average,'/tier2/turaga/singhc/'+t+'/output_'+str(iters[iter_idx])+'/tstvol-'+vol+'_ave.h5')
	if eval:
		evaluateFile([hdf5_gt_file,'/tier2/turaga/singhc/'+t+'/output_'+str(iters[iter_idx])+'/tstvol-'+vol+'_ave.h5',threshes,funcs,save_threshes,out_folder+'/'])


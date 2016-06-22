# convolutional network metric scripts
- Code is based around code from https://bitbucket.org/poozh/watershed described in http://arxiv.org/abs/1505.00249.  For use in https://github.com/naibaf7/PyGreentea. 

# building cython
- run ./make.sh

# function api
- `(segs, rand) = pygt.zwatershed_and_metrics(segTrue, aff_graph, eval_thresh_list, seg_save_thresh_list)`
	- *returns segmentations and metrics*
	- `segs`: list of segmentations
		- `len(segs) == len(seg_save_thresh_list)`
	- `rand`: dict
		- `rand['V_Rand']`:  V_Rand score (scalar)
		- `rand['V_Rand_split']`: list of score values
			- `len(rand['V_Rand_split']) == len(eval_thresh_list)`
		- `rand['V_Rand_merge']`: list of score values, 
			- `len(rand['V_Rand_merge']) == len(eval_thresh_list)`
- `segs = pygt.zwatershed(aff_graph, seg_save_thresh_list)` 
		- *returns segmentations*
	- `segs`: list of segmentations
		- `len(segs) == len(seg_save_thresh_list)`

##### These methods have versions which save the segmentations to hdf5 files instead of returning them

- `rand = pygt.zwatershed_and_metrics_h5(segTrue, aff_graph, eval_thresh_list, seg_save_thresh_list, seg_save_path)`
- `pygt.zwatershed_h5(aff_graph, eval_thresh_list, seg_save_path)`

##### All 4 methods have versions which take an edgelist representation of the affinity graph

- `(segs, rand) = pygt.zwatershed_and_metrics_arb(segTrue, node1, node2, edgeWeight, eval_thresh_list, seg_save_thresh_list)`
- `segs = pygt.zwatershed_arb(seg_shape, node1, node2, edgeWeight, seg_save_thresh_list)`
- `rand = pygt.zwatershed_and_metrics_h5_arb(segTrue, node1, node2, edgeWeight, eval_thresh_list, seg_save_thresh_list, seg_save_path)`
- `pygt.zwatershed_h5_arb(seg_shape, node1, node2, edgeWeight, eval_thresh_list, seg_save_path)`

##### parallel watershed - 4 steps
1. Partition the subvolumes
	- `partition_data = partition_subvols(pred_file,out_folder,max_len=MAX_LEN)`
2. Zwatershed each of the subvolumes
	- *with spark*
	- `eval_with_spark(partition_data[0])`
	- *with python multiprocessing map*
	- `eval_with_par_map(partition_data[0],NUM_WORKERS)`
3. Stitch the subvolumes together, save to new segmentation with several rgs
	- `stitch_and_save(partition_data,outname,(X,Y,Z)=num_vols)`
4. Threshold individual subvolumes by merging
	- `seg_merged = merge_by_thresh(seg,seg_sizes,rg,thresh)`

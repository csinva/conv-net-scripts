# metric functions
- *returns segmentations and metrics.  The first method returns the segmentations and metrics, the second method only computes segmentations and doesn't compute the metrics.*
	1. `(segs, rand) = pygt.zwatershed_and_metrics(aff_graph, segTrue, eval_thresh_list, seg_save_thresh_list)`
		- `segs`: list of segmentations
			- `len(segs) == len(seg_save_thresh_list)`
		- `rand`: dict
		- `rand['V_Rand']`:  V_Rand score (scalar)
		- `rand['V_Rand_split']`: list of score values
			- `len(rand['V_Rand_split']) == len(eval_thresh_list)`
		- `rand['V_Rand_merge']`: list of score values, 
			- `len(rand['V_Rand_merge']) == len(eval_thresh_list)`
	2. `segs = pygt.zwatershed(aff_graph, eval_thresh_list)` 
		- `segs`: list of segmentations
			- `len(segs) == len(seg_save_thresh_list)`
		- `rand`: dict
		- `rand['V_Rand']`:  V_Rand score (scalar)
		- `rand['V_Rand_split']`: list of score values
			- `len(rand['V_Rand_split']) == len(eval_thresh_list)`
		- `rand['V_Rand_merge']`: list of score values, 
			- `len(rand['V_Rand_merge']) == len(eval_thresh_list)`
- These next versions of the above methods save the segmentations to hdf5 files instead of returning them
	3. `rand = pygt.zwatershed_and_metrics_h5(aff_graph, segTrue , eval_thresh_list, seg_save_thresh_list, seg_save_path)`
	4. `pygt.zwatershed_h5(aff_graph, eval_thresh_list, seg_save_path)`


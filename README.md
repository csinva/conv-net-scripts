# convolutional network metric scripts
- Code is based around code from https://bitbucket.org/poozh/watershed described in http://arxiv.org/abs/1505.00249.  For use in https://github.com/naibaf7/PyGreentea. 

# building cython
- run ./make.sh

# function api
1. test button <button data-toggle="collapse" data-target="#111" >+</button><div class="collapse" id="111">
proof by contradiction </div>


- *returns segmentations and metrics.  The first method returns the segmentations and metrics, the second method only computes segmentations and doesn't compute the metrics.*
	1. `(segs, rand) = pygt.zwatershed_and_metrics(segTrue, aff_graph, eval_thresh_list, seg_save_thresh_list)`
		- ** want to reuse computation **
		- `segs`: list of segmentations
			- `len(segs) == len(seg_save_thresh_list)`
		- `rand`: dict
			- `rand['V_Rand']`:  V_Rand score (scalar)
			- `rand['V_Rand_split']`: list of score values
				- `len(rand['V_Rand_split']) == len(eval_thresh_list)`
			- `rand['V_Rand_merge']`: list of score values, 
				- `len(rand['V_Rand_merge']) == len(eval_thresh_list)`
	2. `segs = pygt.zwatershed(aff_graph, seg_save_thresh_list)` 
		- `segs`: list of segmentations
			- `len(segs) == len(seg_save_thresh_list)`
- These next versions of the above methods save the segmentations to hdf5 files instead of returning them
		- ** want to save h5 between every threshold but reuse watershed ** 
	3. `rand = pygt.zwatershed_and_metrics_h5(segTrue, aff_graph, eval_thresh_list, seg_save_thresh_list, seg_save_path)`
	4. `pygt.zwatershed_h5(aff_graph, eval_thresh_list, seg_save_path)`


<style>
.collapse{display:none}.collapse.in{display:block}.collapsing{position:relative;height:0;overflow:hidden;transition-timing-function:ease;transition-duration:0s;transition-property:height,visibility}button{background-color:#fff;color:#008CBA;text-align:center;display:inline-block;margin:1px;transition-duration:.4s;cursor:pointer;border:1px solid #008CBA}button:hover{background-color:#008CBA;color:#fff}div{background-color:#F0F0F0}
</style>
<script src="https://ajax.googleapis.com/ajax/libs/jquery/1.12.0/jquery.min.js"></script>
<script src="http://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/js/bootstrap.min.js"></script>
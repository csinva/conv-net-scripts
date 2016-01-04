import numpy as np
import sys
import subprocess
import os, os.path as op
import h5py


# watershed binary
ws_binary = "/groups/turaga/turagalab/greentea_experiments/emscripts/evaluation/bin/ws2"

# get args
prediction_filename = sys.argv[1]
prediction_filename_raw = prediction_filename.split('.')[0] + '.raw'

groundtruth_file = sys.argv[2]
groundtruth_file_raw = groundtruth_file.split('.')[0] + '.raw'

experiment_result_folder = sys.argv[3]
data_dimension = 520

# Prediction File (hdf5)

'''
# If raw does not exists, write it
print "filename:",prediction_filename
prediction_raw_written = False
f=h5py.File(prediction_filename, 'r')
data = f['main'].value
data_dimension = data.shape[1]
print("Prediction dimension is {0}".format(data_dimension))

if not os.path.exists( prediction_filename_raw ):
	print("Write .raw file for prediction ...")	
	print prediction_filename_raw
	data.tofile( prediction_filename_raw )
	prediction_raw_written = True
f.close()

# Ground truth File (raw)
f=h5py.File(groundtruth_file, 'r')
data = f['main'].value
gt_data_dimension = data.shape[0]
print("Ground truth dimension is {0}".format(gt_data_dimension))

if not os.path.exists( groundtruth_file_raw ):
	print("Ground truth file in .raw format not found")
	if gt_data_dimension != data_dimension:
		print("Data dimension do not match. Clip the GT borders.")
		padding = (gt_data_dimension - data_dimension) / 2
		print("Write clipped .raw file for gt ...")
		newdata = data[padding:(-1*padding),padding:(-1*padding),padding:(-1*padding)]
		print("New GT data shape :")
		print newdata.shape, newdata.max()
		newdata.tofile( groundtruth_file_raw )
		print groundtruth_file_raw
	else:
		print("Write .raw file for gt ...")
		data.tofile( groundtruth_file_raw )
		print groundtruth_file_raw
f.close()
'''

# Create target folder to store results
if not os.path.exists( experiment_result_folder ):
	print("Creating experiment results folder")
	os.mkdir( experiment_result_folder )

# Call ws with parameters
print("Running watershed evaluation")
print ws_binary, prediction_filename_raw, groundtruth_file_raw, experiment_result_folder, str(data_dimension)
subprocess.call( [ws_binary, groundtruth_file_raw, prediction_filename_raw, experiment_result_folder, str(data_dimension)])

# Remove the written prediction raw file
'''
if prediction_raw_written:
	os.remove( prediction_filename_raw )
'''
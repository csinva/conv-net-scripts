import numpy as np
import h5py
import datetime
import sys
np.set_printoptions(precision=4)
#sys.path.append('./malis')
import malis.malis as m


print "Constructing anisotropic neighborhood with 11 edges"
nhood = m.mknhood3d_aniso(1,1.8)
print nhood


datadir = '/groups/turaga/turagalab/greentea/project_data/dataset_06/fibsem_medulla_7col/tstvol-520-1-h5/'
affdir = '/groups/turaga/home/turagas/research/caffe_neural_models/dataset_07/processed/'
affgraphfile = 'train_euclid.h5'
# node1, node2 = m.nodelist_like((2,3,4),-np.eye(3))
# print node1
# print node2

hdf5_aff_file = affdir + affgraphfile
print  "[" +str(datetime.datetime.now())+"]" + "Reading affinity graph volume from " + hdf5_aff_file
h5aff = h5py.File(hdf5_aff_file, 'r')
aff = np.asarray(h5aff['main']).astype(np.float32)
print "aff.shape: " + str(aff.shape)

print "Converting to edelist"
node1, node2, edgeweight = m.affgraph_to_edgelist(aff,nhood)
idxkeep = (node1>=0) & (node2>=0)
node1 = node1[idxkeep]
node2 = node2[idxkeep]
edgeweight = edgeweight[idxkeep]
print "node1.shape: " + str(node1.shape)
print "node2.shape: " + str(node2.shape)
print "edgeweight.shape: " + str(edgeweight.shape)

print "And once we get a segmentation back,"
print "the segmentation should have the same size as aff.shape[1:]"
print "And we can just reshape the segmentation vector back to aff.shape[1:]"

# Serialize to region graph edge list (startID, endID, value) to parse in C++
dt = np.dtype("u4, u4, f4")
f=file('region_graph.raw','w')
np.array( [len(node1)], dtype=np.uint32 ).tofile( f )
np.array( zip(node1,node2,edgeweight), dtype = dt ).tofile( f )
f.close()

import numpy as np
import pylab as plt
import sys
import os.path as op

target_folder = "test_full"

'''
print len(sys.argv)
for arg in sys.argv:
    print arg
'''

outfolder = 'out/'+target_folder
a=np.fromfile( op.join(outfolder , 'linear.dat') )
print a
linear=a.reshape(len(a)/2,2)
plt.plot( linear[:,0], linear[:,1], 'ro-', label='linear')
plt.hold(True)

a=np.fromfile( op.join(outfolder, 'square.dat') )
linear=a.reshape(len(a)/2,2)
plt.plot( linear[:,0], linear[:,1], 'bo-', label='square')

a=np.fromfile( op.join(outfolder, 'felzenszwalb.dat') )
linear=a.reshape(len(a)/2,2)
plt.plot( linear[:,0], linear[:,1], 'ko-', label='felzenszwalb')


a=np.fromfile( op.join(outfolder, 'threshold.dat') )
linear=a.reshape(len(a)/2,2)
plt.plot( linear[:,0], linear[:,1], 'go-', label='threshold')


plt.xlabel('Rand Split')
plt.ylabel('Rand Merge')
plt.legend(bbox_to_anchor=(.5,.5),bbox_transform=plt.gcf().transFigure)
plt.savefig('figs/'+target_folder+'_results.png')
plt.show()


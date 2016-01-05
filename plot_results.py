import numpy as np
import pylab
import sys
import os.path as op

target_folder = "out"

a=np.fromfile( op.join(target_folder, 'linear.dat') )
print a
linear=a.reshape(len(a)/2,2)
pylab.plot( linear[:,0], linear[:,1], 'ro-', label='linear')
pylab.hold(True)

a=np.fromfile( op.join(target_folder, 'square.dat') )
linear=a.reshape(len(a)/2,2)
pylab.plot( linear[:,0], linear[:,1], 'bo-', label='square')

a=np.fromfile( op.join(target_folder, 'felzenszwalb.dat') )
linear=a.reshape(len(a)/2,2)
pylab.plot( linear[:,0], linear[:,1], 'ko-', label='felzenszwalb')

'''
a=np.fromfile( op.join(target_folder, 'threshold.dat') )
linear=a.reshape(len(a)/2,2)
pylab.plot( linear[:,0], linear[:,1], 'go-', label='threshold')


'''

pylab.xlabel('Rand Split')
pylab.ylabel('Rand Merge')

pylab.legend()	
pylab.show()
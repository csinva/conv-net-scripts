import numpy as np
import pylab
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
pylab.plot( linear[:,0], linear[:,1], 'ro-', label='linear')
pylab.hold(True)

a=np.fromfile( op.join(outfolder, 'square.dat') )
linear=a.reshape(len(a)/2,2)
pylab.plot( linear[:,0], linear[:,1], 'bo-', label='square')

a=np.fromfile( op.join(outfolder, 'felzenszwalb.dat') )
linear=a.reshape(len(a)/2,2)
pylab.plot( linear[:,0], linear[:,1], 'ko-', label='felzenszwalb')


a=np.fromfile( op.join(outfolder, 'threshold.dat') )
linear=a.reshape(len(a)/2,2)
pylab.plot( linear[:,0], linear[:,1], 'go-', label='threshold')


pylab.xlabel('Rand Split')
pylab.ylabel('Rand Merge')
pylab.legend()
pylab.savefig('figs/'+target_folder+'_results.png')
pylab.show()


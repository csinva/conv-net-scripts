import numpy as np
import pylab as plt
import sys
import os.path as op

def formatAndSave(ax):
    plt.xlabel('Rand Split')
    plt.ylabel('Rand Merge')
    #plt.xlim([0,1])
    #plt.ylim([0,1])
    plt.legend(bbox_to_anchor=(.4,.4),bbox_transform=plt.gcf().transFigure)
    # Shrink current axis by 20%
    box = ax.get_position()
    ax.set_position([box.x0, box.y0, box.width * 0.8, box.height])

    # Put a legend to the right of the current axis
    ax.legend(loc='center left', bbox_to_anchor=(1, 0.5))
    plt.savefig('figs/'+target_folder+'_results.png')
    plt.show()

target_folder = "test_full"
if len(sys.argv)>1:
    target_folder = sys.argv[1]
outfolder = 'data_tier2/out/'+target_folder

try:
    fig = plt.figure()
    ax = plt.subplot(111)
    a=np.fromfile( op.join(outfolder, 'square.dat') )
    linear=a.reshape(len(a)/2,2)
    print linear
    plt.plot( linear[:,0], linear[:,1], 'bo-', label='square')
    plt.hold(True)

    a=np.fromfile( op.join(outfolder , 'linear.dat') )
    linear=a.reshape(len(a)/2,2)
    plt.plot( linear[:,0], linear[:,1], 'ro-', label='linear') #ms=10

    a=np.fromfile( op.join(outfolder, 'threshold.dat') )
    linear=a.reshape(len(a)/2,2)
    plt.plot( linear[:,0], linear[:,1], 'go-', label='threshold')
    formatAndSave(ax)
except:
    formatAndSave(ax)



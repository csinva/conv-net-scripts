import numpy as np
import pylab as plt
import sys
import os.path as op

def formatAndSave(ax,outputFile):
    plt.xlabel('Iters')
    plt.ylabel('F-Score')
    #plt.xlim([.5,1])
    #plt.ylim([.5,1])
    plt.legend(bbox_to_anchor=(.4,.4),bbox_transform=plt.gcf().transFigure)
    box = ax.get_position()
    ax.set_position([box.x0, box.y0, box.width * 0.7, box.height])
    ax.legend(loc='center left', bbox_to_anchor=(1, 0.5))
    plt.savefig(outputFile)
    ax.grid()
    plt.show()

names = ['train_average','test_average']+['train_fibsem'+str(i) for i in range(2,7)]+['test_fibsem'+str(i) for i in range(2,7)]
folders = ["/tier2/turaga/singhc/train/out/fibsem_ave_","/tier2/turaga/singhc/test/out/fibsem_ave_"]+["/tier2/turaga/singhc/train/out/fibsem"+str(i)+"_" for i in range(2,7)]+["/tier2/turaga/singhc/test/out/fibsem"+str(i)+"_" for i in range(2,7)]
output_file = '/tier2/turaga/singhc/figs/f_scores_full'
train = [20000,80000,200000]
test = [10000,30000,50000,70000,100000,200000]
iters = [train]+[test]+[train]*5+[test]*5
dashed = [iters[i]==train for i in range(len(iters))]
print dashed
all_target_folders = [folders[i]+str(iters[i][j]) for i in range(len(folders)) for j in range(len(iters[i]))]#["data_tier2/train/out/fibsem_ave_"+str(train[i]) for i in range(len(train))]

print folders
print "target folders: ",all_target_folders
print "output file: ",output_file+'.png'

fig = plt.figure(figsize=(15,8))
ax = plt.subplot(111)
count = 0
for x in range(len(iters)):
    target_folders =  all_target_folders[count:count+len(iters[x])]
    maxes = range(len(iters[x]))
    for i in range(len(target_folders)):
        target_folder = target_folders[i]
        outfolder = target_folder
        a=np.fromfile( op.join(outfolder, 'square.dat') )
        data=a.reshape(len(a)/2,2)

        # compute f-scores
        f_scores = range(data.shape[0])
        for j in range(len(f_scores)):
            f_scores[j]=2/(1/data[j][0]+1/data[j][1])
        maxes[i]=max(f_scores)
        count+=1
    if dashed[x]:
        plt.plot(iters[x],maxes, 'o--', label=names[x])
    else:
        plt.plot(iters[x],maxes, 'o-', label=names[x])
    plt.hold(True)
formatAndSave(ax,output_file)



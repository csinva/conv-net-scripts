from __future__ import print_function
import sys, os, math, random, h5py
from os.path import join
import time
os.chdir('/groups/turaga/home/turagas/research/caffe_v2/conv_net_scripts')
sys.path.append('../zwatershed_jc')
import zwatershed
from pyspark import SparkConf, SparkContext
import malis

sets = ['trvol-250-1-h5','trvol-250-2-h5','tstvol-520-1-h5','tstvol-520-2-h5']
# paths models
#model_folders = ['fibsem' + f for f in ['3_adam','4','5','6','7','8','9','10']]
#model_folders = model_folders[-4:]
model_folders = ['fibsem' + f for f in ['8','9','10','11',]+['14_'+ff for ff in ['b','c','d','e','f','g','h','i']]+map(str,range(15,21)+range(22,37))+['32e']]
#iters = range(10000, 200000, 10000)
iters = range(10000, 410000, 10000)

#models_iters = [(m,it) for m in model_folders for it in iters]
models_iters_isets = [(m,it,iset) for m in model_folders for it in iters for iset in sets]
#random.shuffle(models_iters_isets)

def eval_model_iter_set((model_folder,it,iset)):
#       (model_folder,it,iset) = args

        import sys, os, math, h5py
        from os.path import join
        import time
        import numpy as np
        os.chdir('/groups/turaga/home/turagas/research/caffe_v2/conv_net_scripts')
        sys.path.append('../zwatershed_jc')
        import zwatershed

        f = open('/groups/turaga/home/turagas/research/caffe_v2/conv_net_scripts/log_eval_fibsem','a')

        # paths models
        model_base_folder = '/groups/turaga/home/turagas/research/caffe_v2/pygt_models/'
        output_base_folder = '/groups/turaga/home/turagas/research/caffe_v2/processed/'
        datapath = '/groups/turaga/home/turagas/data/FlyEM/fibsem_medulla_7col/'

        modelpath = model_base_folder + model_folder + '/'
        outputpath = output_base_folder + model_folder + '/' + str(it) + '/'

        # settings
#       threshes = [j*2000 for j in range(1,6)]+[j*20000 for j in range(2,16)] # default: 100...1,000...100,000
        threshes = [j*500 for j in range(1,4)]+[j*2000 for j in range(1,6)]+[j*30000 for j in range(2,30)]
        threshes.sort()
        funcs = ['square'] #'linear','threshold','watershed','lowhigh'
        process = True
        eval = True

        # load dataset
        nhood = malis.mknhood3d()

        d={}
        dname = iset
        d['name'] = dname
#               d['data'] = h5py.File(join(datapath,dname,'img_normalized.h5'),'r')['main']
        d['components'] = h5py.File(join(datapath,dname,'groundtruth_seg_thick.h5'),'r')['main']
#       d['affgraph'] = malis.seg_to_affgraph(d['components'],nhood)

        # Evaluate
#       for d in dset:
        predfname = outputpath + d['name'] + '.h5'
        print('Evaluating ' + predfname)
        f.write('Evaluating ' + predfname + '\n')
        segfname = outputpath + d['name'] + '_seg_best.h5'
        if os.path.exists(predfname) and not os.path.exists(segfname):
#                       if os.path.exists(segfname):
#                               return
                affh5 = h5py.File(predfname,'r')['main']
                aff = np.array(affh5,dtype=np.float32)
                off = [(s[0]-s[1])/2 for s in zip(d['components'].shape,affh5.shape[1:])]
                if reduce(max,off) == 0:
                        noborder = True
                        off = [44]*3
                slc = [slice(s[1],s[0]-s[1]) for s in zip(d['components'].shape,off)]
                if noborder:
                        aff = aff[:,slc[0],slc[1],slc[2]]

                d['components'],_ = malis.connected_components_affgraph(
                        malis.seg_to_affgraph(
                                d['components'][slc[0],slc[1],slc[2]],nhood),nhood)
                d['components'] = d['components'].astype(np.uint32)
                V_Rand = V_Rand_split = V_Rand_merge = []
                V_Rand_best = 0
                for t in threshes:
                        (s,V) = zwatershed.zwatershed_and_metrics(d['components'],aff,[t],[t])
                        V_Rand = V_Rand + V['V_Rand']
                        V_Rand_split = V_Rand_split + V['V_Rand_split']
                        V_Rand_merge = V_Rand_merge + V['V_Rand_merge']
                        if V_Rand[-1] > V_Rand_best:
                                V_Rand_best = V_Rand[-1]
                                thresh_best = t
                                seg_best = s[0]
                V_Rand = np.array(V_Rand)
                V_Rand_split = np.array(V_Rand_split)
                V_Rand_merge = np.array(V_Rand_merge)
                segh5 = h5py.File(segfname,'w')
                segh5.create_dataset('main',seg_best.shape,np.int32,data=seg_best)
                segh5.create_dataset('thresh_best',(1,),np.int32,data=thresh_best)
                segh5.create_dataset('V_Rand',V_Rand.shape,np.float32,data=V_Rand)
                segh5.create_dataset('V_Rand_split',V_Rand_split.shape,np.float32,data=V_Rand_split)
                segh5.create_dataset('V_Rand_merge',V_Rand_merge.shape,np.float32,data=V_Rand_merge)
                segh5.create_dataset('finish_status',(1,),np.int32,data=1)
                segh5.close()

        return True

#       sc = SparkContext("local", "eval_fibsem")
conf = SparkConf().setAppName('eval_fibsem')
CORESPERTASK = 4 # default is 1
conf.setAll([("spark.task.cpus", str(CORESPERTASK))])
#conf.setSystemProperty('cores','10')
#conf.setSystemProperty('spark.executor.instances','10')
sc = SparkContext(conf=conf)
finish_status = sc.parallelize(models_iters_isets,len(models_iters_isets)).map(eval_model_iter_set).collect()
print(zip(models_iters_isets,finish_status))
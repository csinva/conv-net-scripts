from __future__ import print_function

import os

import h5py
import malis

from dvision import DVIDDataInstance

from config import mask_threshold, mask_dilation_steps, minimum_component_size

## Training datasets
train_dataset = []

names = []
angle= 0
dname='SNEMI_aug'
for reflectz in range(2):
        for reflecty in range(2):
            for reflectx in range(2):
                for swapxy in range(2):
                    new_dname = '{dname}_z{z}_y{y}_x{x}_xy{swapxy}_angle{angle:05.1f}'.format(
                        dname=dname, z=reflectz, y=reflecty, x=reflectx, swapxy=swapxy,
                        angle=angle
                    )
                    names.append(new_dname)
base_dir = '/data/SNEMI_aug' #'/nobackup/turaga/singhc/SNEMI_aug/rotations'
for name in names:
    dataset = dict()
    dataset['name'] = name
    dataset['data'] = h5py.File(os.path.join(base_dir, 'raw.h5'), 'r')[name]
    dataset['components'] = h5py.File(os.path.join(base_dir, 'seg.h5'), 'r')[name]
    dataset['mask'] = h5py.File(os.path.join(base_dir, 'mask.h5'), 'r')[name]
    train_dataset.append(dataset)
                    
for dataset in train_dataset:
    dataset['nhood'] = malis.mknhood3d()
    dataset['mask_threshold'] = mask_threshold
    dataset['mask_dilation_steps'] = mask_dilation_steps
    dataset['transform'] = {}
    dataset['transform']['scale'] = (0.8, 1.2)
    dataset['transform']['shift'] = (-0.2, 0.2)

print('Training set contains',
      str(len(train_dataset)),
      'volumes:',
      [dataset['name'] for dataset in train_dataset],
      train_dataset)

## Testing datasets
test_dataset = []
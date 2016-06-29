from __future__ import print_function

import os
import sys

import h5py
import malis
import numpy as np
from scipy import ndimage

from augmentation_util import *

num_angles = 1
saving = True
using_parallel = False
n_workers = 8
chunking = False

angles = np.linspace(0, 90, num_angles + 1)[:-1]

data_folder_source = '/nobackup/turaga/singhc/'
data_folder_target = '/nobackup/turaga/singhc/SNEMI_aug/rotations'
datasets = []
neighborhood_3d = mknhood3d()

assert os.path.exists(data_folder_source), "Source folder doesn't exist: {}".format(data_folder_source)
assert os.path.exists(data_folder_target), "Target folder doesn't exist: {}".format(data_folder_target)

rotation_orders = dict(
    data=3,  # cubic spline
    components=0,  # nearest neighbor
)
scaling_factors = dict(
)
source_data_filenames = dict( #input
    data='raw_out.0.h5',
    components='seg_out.0.h5',
    # label='srini_original_aff.h5',
)
data_filenames = dict( #output
    data='raw.h5',
    components='seg.h5',
    # label='groundtruth_aff.h5',
    mask='mask.h5',
)
dtypes = dict(
    data=np.uint8,
    components=np.int32,
    label=np.uint8,
    mask=np.uint8,
)
chunk_sizes = dict(
    data=(64, 64, 64),  # 1 byte * 2^18 = 256 KB
    components=(32, 32, 32),  # 4 bytes * 2^15 = 128 KB
    label=(3, 32, 32, 32),  # 1 bytes * 3 * 2^15 = 96 KB
    mask=(64, 64, 64),  # 256 KB
)

if not chunking:
    # set all chunk sizes to None
    chunk_sizes = dict.fromkeys(chunk_sizes, None)

volumes = [
    'SNEMI_aug',
]


def create_augmented_dataset(dname, reflectz, reflecty, reflectx, swapxy, angle):
    dataset = dict()
    new_dname = '{dname}_z{z}_y{y}_x{x}_xy{swapxy}_angle{angle:05.1f}'.format(
        dname=dname, z=reflectz, y=reflecty, x=reflectx, swapxy=swapxy,
        angle=angle
    )
    dataset['name'] = new_dname
    dataset['already_saved'] = False
    new_dname_folder = os.path.join(data_folder_target, dataset['name'])
    # if os.path.exists(new_dname_folder):
    #     dataset['already_saved'] = True
    #     return dataset
    # load and transform original image and components
    for key in ['data', 'components']:
        filename = data_filenames[key]
        source_filename = source_data_filenames[key]
        source_filepath = os.path.join(data_folder_source, dname,
                                       source_filename)
        rotation_order = rotation_orders[key]
        if key in scaling_factors:
            scaling_factor = scaling_factors[key]
        else:
            scaling_factor = None
        with h5py.File(source_filepath, 'r') as h5_file:
            input_array = np.array(h5_file['main'])
        print("original", key, "from", source_filepath, "had dtype & shape", 
              input_array.dtype, input_array.shape)
        new_array = create_transformed_array(input_array, reflectz, reflecty, reflectx, swapxy, angle,
                                             rotation_order, scaling_factor)
        if new_array.dtype != dtypes[key]:
            print("converting {k} from {old} to {new}".format(
                k=key, old=new_array.dtype, new=dtypes[key]
            ))
            print(new_array.max())
            new_array = new_array.astype(dtypes[key])
        print('transformed {0}: '.format(filename),
              new_array.dtype,
              new_array.shape)
        dataset[key] = new_array
        if key == 'components':
            # make mask array
            original_shape = input_array.shape
            mask_input_array = np.ones(shape=original_shape, dtype=dtypes['mask'])
            mask_array = create_transformed_array(mask_input_array, reflectz, reflecty, reflectx, swapxy, angle,
                                                  rotation_order, scaling_factor=None)
            dataset['mask'] = mask_array.astype(dtypes['mask'])
            dataset['mask_sum'] = np.sum(dataset['mask'])
        # if new_array.dtype != array_original.dtype:
        #     print('dtype mismatch: new_array.dtype = {0}, array_original.dtype = {1}'
        #           .format(new_array.dtype, array_original.dtype
        #                   ))
            # raise ValueError
    # make affinities from transformed component values
    dataset['label'] = malis.seg_to_affgraph(
        dataset['components'], neighborhood_3d
    ).astype(dtypes['label'])
    return dataset

def create_transformed_array_from_hdf5(source_filepath, reflecty, reflectx, swapxy, angle, rotation_order):
    with h5py.File(source_filepath, 'r') as h5_file:
        array_original = np.array(h5_file['main'])
    print('original {0}: '.format(source_filepath),
          array_original.dtype,
          array_original.shape)
    array = array_original
    if array.shape[2] < array.shape[0]:
        # only swap if it looks like axes are backwards 
        array = array.swapaxes(0, 2)
    if reflecty:
        array = array[:, ::-1, :]
    if reflectx:
        array = array[:, :, ::-1]
    if swapxy:
        array = array.transpose((0, 2, 1))
    if angle > 0:
        new_array = \
            ndimage \
                .rotate(array.astype(np.float32),
                        angle,
                        axes=(1, 2),
                        order=rotation_order,
                        cval=mask_number)
    else:
        new_array = array
    if new_array.dtype != array_original.dtype:
        print('dtype mismatch: new_array.dtype = {0}, array_original.dtype = {1}'
              .format(new_array.dtype, array_original.dtype
                      ))
#        raise ValueError
    return new_array


def save_dataset(dataset):
    if dataset['already_saved']:
        return
    # new_dname_folder = os.path.join(data_folder_target, dataset['name'])
    # if not os.path.exists(new_dname_folder):
    #     os.makedirs(new_dname_folder)
    #     print('just made {dir}'.format(
    #         dir=new_dname_folder))
    # else:
    #     print('skipping ', new_dname_folder)
    #     return  # because the file already exists
    for key in data_filenames:
        if type(dataset[key]) is np.ndarray:
            filename = data_filenames[key]
            target_file_path = os.path.join(data_folder_target, filename)
            group_name = dataset['name']
            print('target: ', target_file_path)
            if saving:
                with h5py.File(target_file_path, 'a') as target_h5_file:
                    try:
                        target_h5_file[group_name][0,0,0]
                        # then it exists and is not corrupt, so skip it.
                    except:
                        try:
                            # remove it, in case it's corrupt
                            del target_h5_file[group_name]
                        except:
                            pass
                        target_h5_file.create_dataset(
                            group_name,
                            data=dataset[key],
                            chunks=chunk_sizes[key])
                    #if key is 'mask':
                    #    target_h5_file.create_dataset(
                    #        "sum",
                    #        data=dataset["mask_sum"])


def create_and_save_augmented_dataset(dname, reflectz, reflecty, reflectx, swapxy, angle):
    dataset = create_augmented_dataset(dname, reflectz, reflecty, reflectx, swapxy, angle)
    save_dataset(dataset)
    # debugging...
    for key in dataset:
        if type(dataset[key]) is np.ndarray:
            print(key, id(dataset[key]), dataset[key].dtype, dataset[key].shape,
                  dataset[key].mean(), dataset[key].flatten()[1000000])
    success_message = 'saved dataset {name}'.format(name=dataset['name'])
    return success_message


pool = None
if using_parallel:
    import multiprocessing
    pool = multiprocessing.Pool(n_workers)
#    original_stdout = sys.stdout
#    log_file_name = "{pid}.log".format(pid=os.getpid())
#    f = file(log_file_name, 'w')
#    sys.stdout = f  # override stdout to print to file for that process


print('starting...')

pool_async_results = []
data_augmentation_specs = []
for dname in volumes:
    for reflectz in range(2):
        for reflecty in range(2):
            for reflectx in range(2):
                for swapxy in range(2):
                    for angle in angles:
                        data_augmentation_specs.append(dname)
                        if pool:
                            async_result = pool.apply_async(
                                func=create_and_save_augmented_dataset,
                                args=(dname, reflectz, reflecty, reflectx, swapxy, angle)
                            )
                            pool_async_results.append(async_result)
                        else:
                            create_and_save_augmented_dataset(dname, reflectz, reflecty, reflectx, swapxy, angle)

for async_result in pool_async_results:
    print(async_result.get())

print('done.')

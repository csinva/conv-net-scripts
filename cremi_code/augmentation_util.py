from scipy import ndimage
import numpy as np


def create_transformed_array(array_original, reflectz, reflecty, reflectx, swapxy, angle, rotation_order,
                                       scaling_factor=None):
    array = array_original
    if scaling_factor is not None:
        array = array * scaling_factor
    if array.shape[2] < array.shape[0]:
        # only swap if it looks like axes are backwards 
        array = array.swapaxes(0, 2)
    if reflectz:
        array = array[::-1, :, :]
    if reflecty:
        array = array[:, ::-1, :]
    if reflectx:
        array = array[:, :, ::-1]
    if swapxy:
        array = array.transpose((0, 2, 1))
    print 'rotation_order',rotation_order
    if angle > 0:
        new_array = \
            ndimage \
                .rotate(array.astype(np.float32),
                        angle,
                        axes=(1, 2),
                        order=rotation_order,
                        cval=0)
    else:
        new_array = array
    if new_array.dtype != array_original.dtype:
        print('dtype mismatch: new_array.dtype = {0}, array_original.dtype = {1}'
              .format(new_array.dtype, array_original.dtype
                      ))
#        raise ValueError
    return new_array

def mknhood3d(radius=1):
    # Makes nhood structures for some most used dense graphs.
    # The neighborhood reference for the dense graph representation we use
    # nhood(1,:) is a 3 vector that describe the node that conn(:,:,:,1) connects to
    # so to use it: conn(23,12,42,3) is the edge between node [23 12 42] and [23 12 42]+nhood(3,:)
    # See? It's simple! nhood is just the offset vector that the edge corresponds to.

    ceilrad = np.ceil(radius)
    x = np.arange(-ceilrad,ceilrad+1,1)
    y = np.arange(-ceilrad,ceilrad+1,1)
    z = np.arange(-ceilrad,ceilrad+1,1)
    [i,j,k] = np.meshgrid(z,y,x)

    idxkeep = (i**2+j**2+k**2)<=radius**2
    i=i[idxkeep].ravel(); j=j[idxkeep].ravel(); k=k[idxkeep].ravel();
    zeroIdx = np.ceil(len(i)/2).astype(np.int32);

    nhood = np.vstack((k[:zeroIdx],i[:zeroIdx],j[:zeroIdx])).T.astype(np.int32)
    return np.ascontiguousarray(np.flipud(nhood))
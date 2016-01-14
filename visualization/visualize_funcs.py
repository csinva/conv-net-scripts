import os
import matplotlib
import matplotlib.cm as cm
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider, Button, RadioButtons
import numpy as np
import h5py
import array

#Displays three images: the raw data, the corresponding labels, and the predictions
def display_preds(raw, label, pred, im_size=250, im2_size=432):
    fig = plt.figure(figsize=(20,10))
    fig.set_facecolor('white')
    ax1,ax2,ax3 = fig.add_subplot(1,3,1),fig.add_subplot(1,3,2),fig.add_subplot(1,3,3)

    fig.subplots_adjust(left=0.2, bottom=0.25)
    depth0 = 0
    zoom0 = 250

    #Image is grayscale
    im1 = ax1.imshow(raw[1,:,:],cmap=cm.Greys_r)
    ax1.set_title('Raw Image')

    im = np.zeros((im_size,im_size,3))
    im[:,:,:]=label[1,:,:,:]
    im2 = ax2.imshow(im)
    ax2.set_title('Groundtruth')

    im_ = np.zeros((im2_size,im2_size,3))
    im_[:,:,:]=pred[1,:,:,:]
    im3 = ax3.imshow(im_)
    ax3.set_title('Predictions')

    axdepth = fig.add_axes([0.25, 0.3, 0.65, 0.03], axisbg='white')
    #axzoom  = fig.add_axes([0.25, 0.15, 0.65, 0.03], axisbg=axcolor)

    depth = Slider(axdepth, 'Min', 0, im_size, valinit=depth0,valfmt='%0.0f')
    #zoom = Slider(axmax, 'Max', 0, 250, valinit=max0)
    def update(val):
        z = int(depth.val)
        im1.set_data(raw[z,:,:])
        im[:,:,:]=label[z,:,:,:]
        im2.set_data(im)
        im_[:,:,:]=pred[z,:,:,:]
        im3.set_data(im_)
        fig.canvas.draw()
    depth.on_changed(update)
    plt.show()

#Displays three images: the raw data, the corresponding labels, and the predictions
def display_seg(raw, label, seg, im_size=250, im2_size=432):
    cmap = matplotlib.colors.ListedColormap(np.vstack(((0,0,0),np.random.rand(255,3))))
    fig = plt.figure(figsize=(20,10))
    fig.set_facecolor('white')
    ax1,ax2,ax3 = fig.add_subplot(1,3,1),fig.add_subplot(1,3,2),fig.add_subplot(1,3,3)

    fig.subplots_adjust(left=0.2, bottom=0.25)
    depth0 = 0
    zoom0 = 250

    #Image is grayscale
    im1 = ax1.imshow(raw[1,:,:],cmap=cm.Greys_r)
    ax1.set_title('Raw Image')

    im = np.zeros((im_size,im_size,3))
    im[:,:,:]=label[1,:,:,:]
    im2 = ax2.imshow(im)
    ax2.set_title('Groundtruth')

    im_ = np.zeros((im2_size,im2_size))
    im_[:,:]=seg[1,:,:]
    im3 = ax3.imshow(im_,cmap=cmap)
    ax3.set_title('Seg')

    axdepth = fig.add_axes([0.25, 0.3, 0.65, 0.03], axisbg='white')
    #axzoom  = fig.add_axes([0.25, 0.15, 0.65, 0.03], axisbg=axcolor)

    depth = Slider(axdepth, 'Min', 0, im_size, valinit=depth0,valfmt='%0.0f')
    #zoom = Slider(axmax, 'Max', 0, 250, valinit=max0)

    def update(val):
        z = int(depth.val)
        im1.set_data(raw[z,:,:])
        im[:,:,:]=label[z,:,:,:]
        im2.set_data(im)
        im_[:,:]=seg[z,:,:]
        im3.set_data(im_)
        fig.canvas.draw()
    depth.on_changed(update)
    plt.show()

def trim(data_set,label_set,aff):
    # reshape labels, image
    gt_data_dimension = label_set.shape[0]
    data_dimension = aff.shape[1]
    if gt_data_dimension != data_dimension:
        padding = (gt_data_dimension - data_dimension) / 2
        data_set = data_set[padding:(-1*padding),padding:(-1*padding),padding:(-1*padding)]
        label_set = label_set[padding:(-1*padding),padding:(-1*padding),padding:(-1*padding),:]
    return data_set,label_set

def formatAndSave(ax,outputFile):
    #plt.xlim([.5,1])
    #plt.ylim([.5,1])
    plt.legend(bbox_to_anchor=(.4,.4),bbox_transform=plt.gcf().transFigure)
    box = ax.get_position()
    ax.set_position([box.x0, box.y0, box.width * 0.7, box.height])
    ax.legend(loc='center left', bbox_to_anchor=(1, 0.5))
    plt.savefig(outputFile)
    ax.grid()
    plt.show()
'''
def load_seg_from_dat:
    f = open("/groups/turaga/home/singhc/evaluation/out/fibsem5_0-10-150/lowhigh/vout.0.000010.0.999999.out")
    pred = array.array("I")
    pred.fromfile(f, 432*432*432)
    pred = np.array(pred).reshape((432,432,432))
    print 'pred.shape:',pred.shape
    print 'max:',np.max(pred)
    print 'min:',np.min(pred)
    f.close()
'''

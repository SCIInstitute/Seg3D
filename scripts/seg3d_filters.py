#!/usr/bin/env python
import os, sys, types, threading

# how to call from Seg3D scripts directory:
#exec(open('/Users/ayla/devel/Seg3D_DHS/bin/Seg3D/test_script.py').read())

import seg3d2

# since this is a batch script, truth_region, etc. need to be set beforehand
# i.e. set as command via socket
if not truth_region:
  raise ValueError
if not truth_cropped_region:
  raise ValueError
if not im_gray_all:
  raise ValueError
if not im_cropped_gray:
  raise ValueError
if not im_cropped_chm:
  raise ValueError

class MyThread(threading.Thread):
  def __init__(self, layerID, timeout=5.0):
    self.layerID = layerID
    self.TIMEOUT = timeout
    self.condition = threading.Condition()
    threading.Thread.__init__(self)
  def run(self):
    availableState = "available"
    stateIDData = self.layerID + "::data"
    layerStatus = get(stateid=stateIDData)
    #print(self.layerID, " ", layerStatus)
    with self.condition:
      while get(stateid=stateIDData) != availableState:
        self.condition.wait_for(lambda: get(stateid=stateIDData) == availableState, timeout=self.TIMEOUT)

def update_filepath(filename, strings):
  newfilename = filename
  for key, value in strings.items():
    newfilename = newfilename.replace(key, value)
  return newfilename

def wait_on_layer(layer, timeout=2.0):
  thread = MyThread(layer, timeout)
  thread.start()
  thread.join()

truth_region_files = [os.path.join(truth_region, f) for f in os.listdir(truth_region) if os.path.isfile(os.path.join(truth_region, f)) and f.lower().endswith('.png')]
gray_files = [os.path.join(im_gray_all, f) for f in os.listdir(im_gray_all) if os.path.isfile(os.path.join(im_gray_all, f)) and f.lower().endswith('.mha')]

# ground truth segmentations:
# import -> crop -> export
for f in truth_region_files:
  layers = importlayer(filename="{}".format(f),importer='[ITK Importer]',mode='single_mask')
  wait_on_layer(layers[0])
  layers = crop(layerids="{}".format(layers[0]),origin='[-0.5,-0.5,-0.5]',size='[1024,883.5,1]',replace='true')
  wait_on_layer(layers[0])
  retval = exportsegmentation(layers="{}".format(layers[0]),file_path='{}'.format(truth_cropped_region),mode='single_mask',extension='.png')
  if not retval:
    print("exportsegmentation failed")

# input data images:
# import -> crop -> gradient anisotropic diffusion -> gradient magnitude (boundaries) -> export
for f in gray_files:
  # crop and export grayscale raw input images
  layers = importlayer(filename="{}".format(f),importer='[ITK Importer]',mode='data')
  wait_on_layer(layers[0])
  layers = transform(layerids="{}".format(layers[0]),origin='[0,0,0]',spacing='[1,1,1]',replace='true')
  wait_on_layer(layers[0])
  layers = crop(layerids="{}".format(layers[0]),origin='[-0.5,-0.5,-0.5]',size='[1024,883.5,1]',replace='true')
  layer = layers[0]
  wait_on_layer(layer)
  export_file = update_filepath(f, { im_gray_all: im_cropped_gray })
  retval = exportlayer(layer="{}".format(layer),file_path='{}'.format(export_file),extension='.mha',exporter='[ITK Data Exporter]')
  if not retval:
    print("exportlayer failed")

  # get boundaries
  #layer = gradientanisotropicdiffusionfilter(layerid="{}".format(layer),preserve_data_format='true',replace='true',iterations='20',sensitivity='0.25')
  #wait_on_layer(layer, 5.0)
  layer = gradientanisotropicdiffusionfilter(layerid="{}".format(layer),preserve_data_format='true',replace='true',iterations='2',sensitivity='0.25')
  wait_on_layer(layer)
  layer = gradientmagnitudefilter(layerid="{}".format(layer),replace='true',preserve_data_format='true')
  wait_on_layer(layer)
  export_file = update_filepath(f, { im_gray_all: im_cropped_chm })
  retval = exportlayer(layer="{}".format(layer),file_path='{}'.format(export_file),extension='.mha')
  if not retval:
    print("exportlayer failed")

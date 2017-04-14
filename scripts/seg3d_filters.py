#!/usr/bin/env python
import os, sys, types, threading

# how to call from Seg3D scripts directory:
#exec(open('/Users/ayla/devel/Seg3D_DHS/bin/Seg3D/test_script.py').read())

import seg3d2

class MyThread(threading.Thread):
  def __init__(self, layerID, timeout=2.0):
    self.layerID = layerID
    self.TIMEOUT = timeout
    self.condition = threading.Condition()
    threading.Thread.__init__(self)
  def run(self):
    stateIDData = self.layerID + "::data"
    layerStatus = get(stateid=stateIDData)
    #print(self.layerID, " ", layerStatus)
    with self.condition:
      self.condition.wait_for(lambda: "available" == get(stateid=stateIDData), timeout=self.TIMEOUT)
    #print("Layer {} done processing".format(self.layerID))

def update_filepath(filename, strings):
  newfilename = filename
  for key, value in strings.items():
    newfilename = newfilename.replace(key, value)
  return newfilename

def wait_on_layer(layer):
  thread = MyThread(layer, 1.0)
  thread.start()
  thread.join()

# since this is a batch script, truth_region and im_gray need to be set beforehand
# i.e. set as command via socket
if not truth_region:
  raise ValueError

if not im_gray:
  raise ValueError

truth_region_files = [os.path.join(truth_region, f) for f in os.listdir(truth_region) if os.path.isfile(os.path.join(truth_region, f)) and f.lower().endswith('.png')]
im_gray_files = [os.path.join(im_gray, f) for f in os.listdir(im_gray) if os.path.isfile(os.path.join(im_gray, f)) and f.lower().endswith('.mha')]

# ground truth segmentations:
# import -> crop -> export
for f in truth_region_files:
  layers = importlayer(filename="{}".format(f),importer='[ITK Importer]',mode='mask')
  print(layers[0])
  wait_on_layer(layers[0])
  layers = crop(layerids="{}".format(layers[0]),origin='[-0.0735295,-0.0735295,-0.5]',size='[150.515,129.927,1]',replace='false')
  print(layers[0])
  wait_on_layer(layers[0])
  export_file = update_filepath(f, { truth_region: truth_cropped_region })
  print(export_file)
  retval = exportlayer(layer="{}".format(layers[0]),file_path='{}'.format(export_file),extension='.mha',exporter='[ITK Mask Exporter]');

# input data images:
# import -> crop -> gradient anisotropic diffusion -> gradient magnitude (boundaries) -> export
for f in im_gray_files:
  layers = importlayer(filename="{}".format(f),importer='[ITK Importer]',mode='data')
  print(layers[0])
  wait_on_layer(layers[0])
  layers = crop(layerids="{}".format(layers[0]),origin='[-0.0735295,-0.0735295,-0.5]',size='[150.515,129.927,1]',replace='false')
  print(layers[0])
  wait_on_layer(layers[0])
  layers = gradientanisotropicdiffusionfilter(layerid=="{}".format(layers[0])preserve_data_format='true',replace='false',iterations='20',sensitivity='0.25');
  print(layers[0])
  wait_on_layer(layers[0])
  layers = gradientmagnitudefilter(layerid="{}".format(layers[0]),replace='false',preserve_data_format='true');
  print(layers[0])
  wait_on_layer(layers[0])
  export_file = update_filepath(f, { im_gray: im_cropped_gray })
  print(export_file)
  retval = exportlayer(layer="{}".format(layers[0]),file_path='{}'.format(export_file),extension='.mha',exporter='[ITK Data Exporter]');

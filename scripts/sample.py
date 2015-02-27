# example of how to call with fully qualified path:
#exec(open('/Volumes/scratch/seg3d2_irtools/scripts/sample.py').read())

import seg3d2
import configparser
import os
import sys

layerid = '<none>'
datasetName = 'Polson3319'
shrinkFactor = 8 # downsample
overlapMin = 0.1
overlapMax = 0.3

mosaicExtension = '.mosaic'
stosExtension = '.stos'

tilesDir = '/usr/sci/projects/msmg/data/Polson3319slices'
fftDir = '/Volumes/scratch/seg3d2_irtools/test/fft'
stosDir = '/Volumes/scratch/seg3d2_irtools/test/stos'
volumeDir = '/Volumes/scratch/seg3d2_irtools/test/vol'

# Polson3319 slices are made up of 3x3 tiles with naming pattern Image1-index#_tileX#-tileY#.tif.
# Break up names by prefix (Image1-) and suffix (_tileX#-tileY#.tif) and put in array in order from top left to bottom right.
filePrefix = 'Image1-'
fileSuffixList = ['_000-000.tif','_001-000.tif','_002-000.tif','_000-001.tif','_001-001.tif','_002-001.tif','_000-002.tif','_001-002.tif','_002-002.tif']

first = 39
last = 247 

for index in range(first, last+1): # python loops over range first, last-1
  outputName = "{0}_{1}".format(datasetName, index)

  # build file names in order
  files = []
  for suffix in fileSuffixList:
    filename = "%s%03d%s" % (filePrefix, index, suffix)
    files.append(filename)

  print("Tiling {0}".format(files))

  fftMosaic="{0}/{1}{2}".format(fftDir, outputName, mosaicExtension)
  seg3d2.fftfilter(layerid=layerid, shrink_factor=shrinkFactor, overlap_min=overlapMin, overlap_max=overlapMax, images=files, directory=tilesDir, output_mosaic=fftMosaic)

  files = []

for index in range(first, last):
  nextIndex = index + 1
  print('slice to slice registration: index={0}, nextIndex={1}'.format(index, nextIndex))
  outputName = "{0}_{1}".format(datasetName, index)
  nextOutputName = "{0}_{1}".format(datasetName, nextIndex)

  mosaicFixed="{0}/{1}{2}".format(fftDir, outputName, mosaicExtension)
  if not os.path.isfile(mosaicFixed):
      break
  mosaicMoving="{0}/{1}{2}".format(fftDir, nextOutputName, mosaicExtension)
  if not os.path.isfile(mosaicMoving):
      break

  stosOutputStos="{0}/{1}_{2}_{3}{4}".format(stosDir, datasetName, index, nextIndex, stosExtension)
  seg3d2.slicetoslicebrutefilter(layerid=layerid, shrink_factor=shrinkFactor, input_fixed=mosaicFixed, input_moving=mosaicMoving, output_stos=stosOutputStos)

stosfiles=[]
for f in os.listdir(stosDir):
  if f.endswith(stosExtension):
    stosPath = "%s/%s" % (stosDir, f)
    stosfiles.append(stosPath)

# Full resolution volume.
#volumeShrinkFactor = 1
volumeShrinkFactor = 8

# There is a hard limit on image size in Seg3D in the renderer: no dimension can be > 4096
seg3d2.slicetovolumefilter(layerid=layerid, input_files=stosfiles, output_prefixes=[volumeDir], image_dirs=[tilesDir], shrink_factor=volumeShrinkFactor, load_volume=True)

print("Volume written to {0}.".format(volumeDir))

# how to call from Seg3D scripts directory:
#exec(open('../scripts/polson3319_pipeline1.py').read())

import seg3d2
import configparser
import os
import sys

config = configparser.ConfigParser()
config.read('../scripts/settings.ini')

datasetName = 'Polson3319'
testRoot = config.get(datasetName, 'dir')
if not os.path.exists(testRoot):
  raise ValueError("Path %s does not exist." % testRoot)

testOutputRoot = config.get(datasetName, 'outputDir')

fftDir = "{0}/fft".format(testOutputRoot)
refineGridDir = "{0}/grid".format(testOutputRoot)
resultsDir = "{0}/results".format(testOutputRoot)
stosDir = "{0}/stos".format(testOutputRoot)
volumeDir = "{0}/volume".format(testOutputRoot)

layerid=''
outputImageExtension = config.get(datasetName, 'image_ext')
mosaicExtension = config.get(datasetName, 'mosaic_ext')
stosExtension = config.get(datasetName, 'stos_ext')
shrinkFactor = config.get(datasetName, 'shrink')
pixelSpacing = config.get(datasetName, 'spacing')
pyramidIterations = config.get(datasetName, 'pyramid_iters')
overlapMin = config.get(datasetName, 'overlap_min')
overlapMax = config.get(datasetName, 'overlap_max')
minPeak = config.get(datasetName, 'min_peak')
peakThreshold = config.get(datasetName, 'peak_threshold')
rangeMin = int(config.get(datasetName, 'first_slice'))
rangeMax = int(config.get(datasetName, 'last_slice'))

filePrefix = 'Image1-'
fileSuffixList = ['_000-000.tif','_001-000.tif','_002-000.tif','_000-001.tif','_001-001.tif','_002-001.tif','_000-002.tif','_001-002.tif','_002-002.tif']


#pyramidLevels=6 # TODO: produces segfault
pyramidLevels=2
refineGridIterations=20
cellSize=8
blendEdges="blend"

mosaicRangeMax=rangeMax+1
stosRangeMax=rangeMax

for index in range(rangeMin, mosaicRangeMax):
  outputName = "{0}_{1}".format(datasetName, index)

  files = []
  for suffix in fileSuffixList:
    filename = "%s%03d%s" % (filePrefix, index, suffix)
    files.append(filename)

  print(files)

  fftMosaic="{0}/{1}{2}".format(fftDir, outputName, mosaicExtension)
  seg3d2.fftfilter(layerid=layerid,
                   tile_strategy='top_left_book',
                   min_peak=minPeak,
                   peak_threshold=peakThreshold,
                   shrink_factor=shrinkFactor,
                   overlap_min=overlapMin,
                   overlap_max=overlapMax,
                   pixel_spacing=pixelSpacing,
                   images=files,
                   directory=testRoot,
                   output_mosaic=fftMosaic)
  #iterations_per_level=pyramidIterations, pyramid_levels=pyramidLevels

  #refineGridMosaic="{0}/{1}{2}".format(refineGridDir, outputName, mosaicExtension)
  #seg3d2.refinegridfilter(layerid=layerid, shrink_factor=shrinkFactor, pixel_spacing=pixelSpacing, iterations=refineGridIterations, input_mosaic=fftMosaic, output_mosaic=refineGridMosaic, directory=testRoot, cell_size=cellSize)


  # in case refine grid failed:
  #if os.path.getsize(refineGridMosaic) == 0:
  #  print("Refine grid failed for {0}. Using fft mosaic {1} instead.".format(refineGridMosaic, fftMosaic))
  refineGridMosaic = fftMosaic

  imageOutputFile="{0}/{1}{2}".format(resultsDir, outputName, outputImageExtension)
  seg3d2.assemblefilter(layerid=layerid, shrink_factor=1, pixel_spacing=pixelSpacing, input_mosaic=refineGridMosaic, output_image=imageOutputFile, directory=testRoot, feathering=blendEdges)

  files = []
#sys.exit()
for index in range(rangeMin, stosRangeMax):
  next_index = index + 1
  print('index={0}, next_index={1}'.format(index, next_index))
  outputName = "{0}_{1}".format(datasetName, index)
  nextOutputName = "{0}_{1}".format(datasetName, next_index)

  mosaicFixed="{0}/{1}{2}".format(fftDir, outputName, mosaicExtension)
  if not os.path.isfile(mosaicFixed):
      break
  mosaicMoving="{0}/{1}{2}".format(fftDir, nextOutputName, mosaicExtension)
  if not os.path.isfile(mosaicMoving):
      break

  stosOutputStos="{0}/{1}_{2}_{3}{4}".format(stosDir, datasetName, index, next_index, stosExtension)
  seg3d2.slicetoslicebrutefilter(layerid=layerid, shrink_factor=shrinkFactor, input_fixed=mosaicFixed, input_moving=mosaicMoving, output_stos=stosOutputStos)

stosfiles=[]
for f in os.listdir(stosDir):
  if f.endswith(stosExtension):
    stosPath = "%s/%s" % (stosDir, f)
    stosfiles.append(stosPath)
    # TODO: error, make test
    #stosfiles.append(f)

print(stosfiles)

# TODO: try 0 also
volumeShrinkFactor = 1
prefix=[volumeDir]
seg3d2.slicetovolumefilter(layerid=layerid, input_files=stosfiles, output_prefixes=prefix, image_dirs=[testRoot], shrink_factor=volumeShrinkFactor)

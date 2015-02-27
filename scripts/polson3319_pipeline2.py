# how to call from Seg3D scripts directory:
#exec(open('../scripts/polson3319_pipeline2.py').read())

import seg3d2
import configparser
import os

config = configparser.ConfigParser()
config.read('../scripts/settings.ini')

datasetName = 'Polson3319'
testRoot = config.get(datasetName, 'dir')
if not os.path.exists(testRoot):
  raise ValueError("Path %s does not exist." % testRoot)

enhancedDir = "{0}/enhanced".format(testRoot)
fftDir = "{0}/fft".format(testRoot)
refineTranslateDir = "{0}/translate".format(testRoot)
refineGridDir = "{0}/grid".format(testRoot)
resultsDir = "{0}/results".format(testRoot)
stosDir = "{0}/stos".format(testRoot)
volumeDir = "{0}/volume".format(testRoot)

layerid = ''
outputImageExtension = config.get(datasetName, 'image_ext')
mosaicExtension = config.get(datasetName, 'mosaic_ext')
stosExtension = config.get(datasetName, 'stos_ext')
shrinkFactor = config.get(datasetName, 'shrink')
pixelSpacing = config.get(datasetName, 'spacing')
pyramidIterations = config.get(datasetName, 'pyramid_iters')
claheWindowDim = config.get(datasetName, 'clahe_window_dim')
claheShrinkFactor = config.get(datasetName, 'clahe_shrink')
volumeShrinkFactor = config.get(datasetName, 'volume_shrink')
overlapMin = config.get(datasetName, 'overlap_min')
overlapMax = config.get(datasetName, 'overlap_max')
#pyramidLevels = 6 # TODO: causes segfault - investigate...
pyramidLevels = config.get(datasetName, 'pyramid_levels')
blendEdges = config.get(datasetName, 'assemble_feathering_option')

filePrefix = 'Image1-'
fileSuffixList = ['_000-000.tif','_001-000.tif','_002-000.tif','_000-001.tif','_001-001.tif','_002-001.tif','_000-002.tif','_001-002.tif','_002-002.tif']

rangeMin = 39
rangeMax = 247
mosaicRangeMax = rangeMax+1
stosRangeMax = rangeMax

window = [claheWindowDim, claheWindowDim]

files = []

# preprocessing
for index in range(rangeMin, mosaicRangeMax):
  for suffix in fileSuffixList:
    filename = "%s%03d%s" % (filePrefix, index, suffix)
    files.append(filename)

  print(files)

  for f in files:
    inputImage = "{0}/{1}".format(testRoot, f)
    outputImage = "{0}/{1}".format(enhancedDir, f)
    seg3d2.clahefilter(layerid=layerid, shrink_factor=claheShrinkFactor, input_image=inputImage, output_image=outputImage, window_x=window[0], window_y=window[1])

  outputName = "{0}_{1}".format(datasetName, index)

  fftMosaic = "{0}/{1}{2}".format(fftDir, outputName, mosaicExtension)
  seg3d2.fftfilter(layerid=layerid, shrink_factor=shrinkFactor, overlap_min=overlapMin, overlap_max=overlapMax, pixel_spacing=pixelSpacing, images=files, directory=enhancedDir, output_mosaic=fftMosaic, iterations_per_level=pyramidIterations, pyramid_levels=pyramidLevels)

  imageOutputFile = "{0}/{1}{2}".format(resultsDir, outputName, outputImageExtension)
  seg3d2.assemblefilter(layerid=layerid, shrink_factor=shrinkFactor, pixel_spacing=pixelSpacing, input_mosaic=fftMosaic, output_image=imageOutputFile, directory=testRoot) 

  files = []

for index in range(rangeMin, stosRangeMax):
  next_index = index + 1
  print('index={0}, next_index={1}'.format(index, next_index))
  outputName = "{0}_{1}".format(datasetName, index)
  nextOutputName = "{0}_{1}".format(datasetName, next_index)

  mosaicFixed = "{0}/{1}{2}".format(fftDir, outputName, mosaicExtension)
  if not os.path.isfile(mosaicFixed):
      break
  mosaicMoving = "{0}/{1}{2}".format(fftDir, nextOutputName, mosaicExtension)
  if not os.path.isfile(mosaicMoving):
      break

  stosOutputStos = "{0}/{1}_{2}_{3}{4}".format(stosDir, datasetName, index, next_index, stosExtension)
  seg3d2.slicetoslicebrutefilter(layerid=layerid, shrink_factor=shrinkFactor, input_fixed=mosaicFixed, input_moving=mosaicMoving, output_stos=stosOutputStos)

stosfiles = []
for f in os.listdir(stosDir):
  if f.endswith(stosExtension):
    stosPath = "%s/%s" % (stosDir, f)
    stosfiles.append(stosPath)
    # TODO: error, make test
    #stosfiles.append(f)

print(stosfiles)

prefix = [volumeDir]
imageDirs = [testRoot]
seg3d2.slicetovolumefilter(layerid=layerid, input_files=stosfiles, output_prefixes=prefix, image_dirs=imageDirs, shrink_factor=volumeShrinkFactor)

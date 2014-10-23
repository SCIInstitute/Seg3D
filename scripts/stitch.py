# how to call from Seg3D bin directory:
#exec(open('/home/sci/brig/Documents/Tools/seg3d/scripts/stitch.py').read())

# very basic pipeline uses MouseVolume dataset on SCI file server
# tests fft, assemble, stos brute, stom

import seg3d2
import configparser
import os
import os.path
import subprocess
import glob
import sys

# set up parameters:
config = configparser.ConfigParser()
datasetName= 'test'
config.read('/home/sci/brig/Documents/Tools/seg3d/scripts/settings.ini')
testRoot = config.get(datasetName, 'dir')
if not os.path.exists(testRoot):
  raise ValueError("Path %s does not exist." % testRoot)
outputImageExtension = config.get(datasetName, 'image_ext')
shrinkFactor = config.get(datasetName, 'shrink')
pixelSpacing = config.get(datasetName, 'spacing')
firstIndex = int(config.get(datasetName, 'first_slice'))
lastIndex = int(config.get(datasetName, 'last_slice'))
mosaicLastIndex = lastIndex+1
pixelSpacing = config.get(datasetName, 'spacing')
pyramidIterations = config.get(datasetName, 'pyramid_iters')
pyramidLevels = config.get(datasetName, 'pyramid_levels')
overlapMax = config.get(datasetName, 'overlap_max')
overlapMin = config.get(datasetName, 'overlap_min')
claheWindowDim = config.get(datasetName, 'clahe_window_dim')
claheShrinkFactor = config.get(datasetName, 'clahe_shrink')
tileStrategy = config.get(datasetName, 'tile_strategy')
minPeak = config.get(datasetName, 'min_peak')
peakThreshold = config.get(datasetName, 'peak_threshold')
numberPrefix = config.get(datasetName, 'number_prefix')
window = [claheWindowDim, claheWindowDim]
refineGridIterations=20
cellSize=8
layerid='<none>'
imageDirList=[]
outputMosaicList=[]
sliceNums=[]

for index in range(firstIndex, mosaicLastIndex):
  outputStem='slice{0}'.format(index)
  imageDir='{0}/{1}{2}'.format(testRoot, numberPrefix, index)
  ############################################
  # THIS IS THE CLAHE AND BLOBBING SECTIONS
  ############################################
  #clahe filter tool
  #print('Calling clahe filter...')
  #files = glob.glob('{0}/*.png'.format(imageDir))
  #for f in files:
  #  inputImage = f
  #  lastIdx = inputImage.rindex('/')
  #  fname = inputImage[lastIdx+1:]
  #  outputImage = "{0}_2/{1}".format(imageDir,fname)
  #  seg3d2.clahefilter(layerid=layerid, shrink_factor=claheShrinkFactor,
  #    input_image=inputImage, output_image=outputImage,
  #    window_x=window[0], window_y=window[1])
  #imageDir = "{0}_2".format(imageDir)
  #blobbing
  #print('Calling blob filter...')
  #files = glob.glob('{0}/*.png'.format(imageDir))
  #for f in files:
  #  inputImage = f
  #  lastIdx = inputImage.rindex('/')
  #  fname = inputImage[lastIdx+1:]
  #  outputImage = "{0}_3/{1}".format(imageDir,fname)
  #  seg3d2.blobfeatureenhancementfilter(layerid=layerid,
  #    input_image=inputImage, output_image=outputImage, radius=2)
  #imageDir = "{0}_3".format(imageDir)
  ############################################
  #fast fourier transform tool
  ############################################
  if not os.path.exists(imageDir): 
    continue
  numfiles=len([name for name in os.listdir(imageDir)])
  if numfiles >= 2:
    fftOutputMosaic='{0}/{1}.mosaic'.format(testRoot, outputStem)
    print("Calling fast fourier transform...")
    print("   {0}".format(fftOutputMosaic))

    try:
      seg3d2.fftfilter(layerid=layerid, shrink_factor=shrinkFactor, 
         overlap_min=overlapMin, overlap_max=overlapMax, 
         tile_strategy=tileStrategy, 
         pixel_spacing=pixelSpacing, directory=imageDir, 
         min_peak=minPeak, peak_threshold=peakThreshold,
         output_mosaic=fftOutputMosaic,
         iterations_per_level=pyramidIterations, 
         pyramid_levels=pyramidLevels)
    
      #refineGridMosaic="{0}/{1}g.mosaic".format(testRoot, outputStem)
      #print("Calling refine grid filter...")
      #print("   {0}".format(refineGridMosaic))
      #print(refineGridMosaic)
      #seg3d2.refinegridfilter(layerid=layerid, shrink_factor=shrinkFactor, 
      #    pixel_spacing=pixelSpacing, iterations=refineGridIterations, 
      #    input_mosaic=fftOutputMosaic, output_mosaic=refineGridMosaic, 
      #    directory=imageDir, cell_size=cellSize)
  
      # in case refine grid failed:
      #if os.path.getsize(refineGridMosaic) == 0:
      #  print("Refine grid failed for {0}. Using fft mosaic {1} instead.".format(
      #    refineGridMosaic, fftOutputMosaic))
      #  refineGridMosaic = fftOutputMosaic
      refineGridMosaic = fftOutputMosaic
  
      assmblOutImg='{0}/{1}{2}'.format(testRoot, outputStem, outputImageExtension)
      ############################################
      #Assembling of images
      ############################################
      print('Assembling image...')
      print("   {0}".format(assmblOutImg))
      seg3d2.assemblefilter(layerid=layerid, input_mosaic=refineGridMosaic,
        output_image=assmblOutImg, directory=imageDir, shrink_factor=1)
      imageDirList.append(imageDir)
      outputMosaicList.append(assmblOutImg)#refineGridMosaic)
      sliceNums.append(index)
    except:
      e = sys.exc_info()[0]
      print("Error: {0}".format(e))
#sys.exit()  
outputStosList=[]

for i in range(len(sliceNums) - 1):
  index = sliceNums[i]
  nextIndex = sliceNums[i + 1]
  inputFixed=outputMosaicList[i]
  inputMoving=outputMosaicList[i + 1]
  print('Assembling Volume with slice: {0}'.format(inputMoving))
  outputStos='{0}/{1}-{2}.stos'.format(testRoot, index, nextIndex)
  print(outputStos)
  outputStosList.append(outputStos)
  seg3d2.slicetoslicebrutefilter(layerid=layerid, input_fixed=inputFixed, 
      input_moving=inputMoving, output_stos=outputStos, shrink_factor=shrinkFactor)

outputVolumeDir='{0}/vol'.format(testRoot)
seg3d2.slicetovolumefilter(layerid=layerid, 
    input_files=outputStosList, 
    output_prefixes=[outputVolumeDir], 
    image_dirs=imageDirList, shrink_factor=1)

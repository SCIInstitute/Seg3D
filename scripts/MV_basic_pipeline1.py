# how to call from Seg3D bin directory:
#exec(open('../scripts/MV_basic_pipeline1.py').read())

# very basic pipeline uses MouseVolume dataset on SCI file server
# tests fft, assemble, stos brute, stom

import seg3d2
import configparser
import os

# set up parameters:
config = configparser.ConfigParser()
config.read('../scripts/MV_settings.ini')
testRoot = config.get('MouseVolume', 'dir')
if not os.path.exists(testRoot):
  raise ValueError("Path %s does not exist." % testRoot)

outputImageExtension = config.get('MouseVolume', 'image_ext')
shrinkFactor = config.get('MouseVolume', 'shrink')
pixelSpacing = config.get('MouseVolume', 'spacing')
firstIndex = int(config.get('MouseVolume', 'first_slice'))
lastIndex = int(config.get('MouseVolume', 'last_slice'))
mosaicLastIndex = lastIndex+1

layerid='<none>'
testRoot2='{0}/test_vol'.format(testRoot)
outputVolumeDir='{0}/vol'.format(testRoot2)
imageDirList=[]
outputMosaicList=[]

for index in range(firstIndex, mosaicLastIndex):
  outputStem='mv{0}'.format(index)
  imageDir='{0}/{1}'.format(testRoot, index)
  imageDirList.append(imageDir)
  testOutputRoot='{0}/test'.format(imageDir)
  fftOutputMosaic='{0}/fft/{1}.mosaic'.format(testOutputRoot, outputStem)
  print(fftOutputMosaic)
  outputMosaicList.append(fftOutputMosaic)

  seg3d2.fftfilter(layerid=layerid, directory=imageDir, output_mosaic_file=fftOutputMosaic, shrink_factor=shrinkFactor)

  assembleOutputImage='{0}/results/{1}{2}'.format(testOutputRoot, outputStem, outputImageExtension)
  print(assembleOutputImage)

  seg3d2.assemblefilter(layerid=layerid, input_mosaic=fftOutputMosaic, output_image=assembleOutputImage, directory=imageDir, shrink_factor=shrinkFactor)

fixedIndex=1
inputFixed=outputMosaicList[0]
print(inputFixed)
outputStosList=[]

for index in range(firstIndex,lastIndex):
  nextIndex = index + 1
  inputMoving=outputMosaicList[index]
  print(inputMoving)
  outputStos='{0}/stos/mv{1}{2}.stos'.format(testRoot2, fixedIndex, nextIndex)
  print(outputStos)
  outputStosList.append(outputStos)
  seg3d2.slicetoslicebrutefilter(layerid=layerid, input_fixed=inputFixed, input_moving=inputMoving, output_stos=outputStos, shrink_factor=shrinkFactor)

seg3d2.slicetovolumefilter(layerid=layerid, input_files=outputStosList, output_prefixes=[outputVolumeDir], image_dirs=imageDirList, shrink_factor=shrinkFactor)

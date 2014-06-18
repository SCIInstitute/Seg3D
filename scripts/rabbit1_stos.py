# how to call from Seg3D scripts directory:
#exec(open('../scripts/rabbit1_stos.py').read())

import seg3d2
import os

layerid=''
shrink_factor=12
assembled_image_dir='/Volumes/scratch/misc_data/Marc_rabbit_retina'
image_suffix='.png'
image_stem='_mosaic_8'
brute_stos_stem='_brute_8'
stos_suffix='.stos'
# really should be using os.sep here - change if Windows support is needed
stos_dir='{0}/stos'.format(assembled_image_dir)
volume_dir='{0}/volume'.format(assembled_image_dir)
iterations=10
num_threads=2
neighborhood=64
grid_spacing=256

image_pairs=[[2,1],[3,2],[4,3],[7,4],[8,7],[9,7],[10,9],[11,10],[12,11],[13,12],[14,13],[15,14],[16,15],[17,16],[18,17],[19,18],[20,18],[21,20],[22,20],[23,22],[24,23],[25,23],[27,25],[28,27],[29,28],[30,28]]
for pair in image_pairs:
  # really should be using os.sep here - change if Windows support is needed
  moving_image="%s%s%04d%s%s" % (assembled_image_dir, '/', pair[0], image_stem, image_suffix)
  fixed_image="%s%s%04d%s%s" % (assembled_image_dir, '/', pair[1], image_stem, image_suffix)
  output_stos="%s%s%04d-%04d%s%s" % (stos_dir, '/', pair[0], pair[1], brute_stos_stem, stos_suffix)
  print("Generating {0}.".format(output_stos))
  slicetoslicebrutefilter(layerid=layerid, shrink_factor=shrink_factor, input_fixed=fixed_image, input_moving=moving_image, output_stos=output_stos)

stosfiles = []
for f in os.listdir(stos_dir):
  if f.endswith(stos_suffix):
    # really should be using os.sep here - change if Windows support is needed
    stosPath = "%s%s%s" % (stos_dir, '/', f)
    stosfiles.append(stosPath)
 
print("Generating volume from stos files {0}".format(stosfiles))
slicetovolumefilter(layerid=layerid, input_files=stosfiles, output_prefixes=[volume_dir], image_dirs=[assembled_image_dir], shrink_factor=shrink_factor)

imagefiles = []
for f in os.listdir(volume_dir):
  if f.endswith(image_suffix):
    # really should be using os.sep here - change if Windows support is needed
    imagePath = "%s%s%s" % (volume_dir, '/', f)
    imagefiles.append(imagePath)

print(imagefiles)
data_volume_id = importseries(filenames=imagefiles, importer='[ITK FileSeries Importer]')

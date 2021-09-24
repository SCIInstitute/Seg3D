Seg3D Tools
========================================

There are many different tools and filters implemented in Seg3D. These tools and filters are there to attempt several tasks, from reformatting original data, smoothing and de-noising image data, to automatic and manual segmenting. Every tool and filter will be describe briefly in an effort to provide the user with a list of methods that may be used to segment the data that they have. These methods have been separated into four categories, general tools, mask filters, data filters, and advanced filters.

Most of the tools and filters have a ‘replace’ option. The default for most of these functions is to create a new layer, but ‘replace’ will cause the new layer to instead replace the old one. This is very useful in avoiding the generation of multiple layer that are only intermediate steps. Also, most tools and filters will by default insert the active layer as the primary input. You may disable this and choose manually which layer to modify.

Many of the tools and filters must use a certain type of data as the input, either mask or image data. If the wrong type of layer is chosen as the input, the tool/filter interface will display a warning message in a red box near the execution button informing you of this. The tool will also be disabled until the correct type of layer is chosen. This second feature applies to empty inputs also.

A final note is that when tools have multiple inputs, the inputs must all be from the same layer group (geometric grid), with exception of the registration tool. This ensures that all the voxels in the multiple inputs have corresponding voxels to every other input volume.


.. toctree::
   :maxdepth: 2
   :caption: Contents:

   Tools/AdvancedFilters/index
   Tools/DataFilters/index
   Tools/LargeVolumeTools/index
   Tools/MaskFilters/index
   Tools/Tools/index

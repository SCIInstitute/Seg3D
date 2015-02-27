#
#  For more information, please see: http://software.sci.utah.edu
#
#  The MIT License
#
#  Copyright (c) 2013 Scientific Computing and Imaging Institute,
#  University of Utah.
#
#
#  Permission is hereby granted, free of charge, to any person obtaining a
#  copy of this software and associated documentation files (the "Software"),
#  to deal in the Software without restriction, including without limitation
#  the rights to use, copy, modify, merge, publish, distribute, sublicense,
#  and/or sell copies of the Software, and to permit persons to whom the
#  Software is furnished to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice shall be included
#  in all copies or substantial portions of the Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
#  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#  DEALINGS IN THE SOFTWARE.
#

# TODO: let's rename this to IRTOOLS...

##################################################
# Set sources
##################################################

SET(CORE_ITKCOMMON_SRCS
  AsyncMosiacSave.h
  IRImageLoader.cxx
  IRImageLoader.hxx
  IRPruner.cpp
  IRPruner.h
  LineMath.cpp
  LineMath.h
  cluster.cxx
  cluster.hxx
  common.cxx
  common.hxx
  draw.hxx
  extrema.hxx
  grid_common.cxx
  grid_common.hxx
  histogram.cxx
  histogram.hxx
  match.cxx
  match.hxx
  mosaic_layout_common.cxx
  mosaic_layout_common.hxx
  mosaic_refinement_common.cxx
  mosaic_refinement_common.hxx
  pyramid.cxx
  pyramid.hxx
  the_aa_bbox.cxx
  the_aa_bbox.hxx
  the_dynamic_array.hxx
  the_text.cxx
  the_text.hxx
  the_utils.cxx
  the_utils.hxx
  threshold.hxx
  tree.hxx
  v3x1p3x1.hxx
  visualize.cxx
  visualize.hxx
)

SET(CORE_ITKCOMMON_FILTERING_SRCS
  Filtering/itkNormalizeImageFilterWithMask.h
  Filtering/itkNormalizeImageFilterWithMask.txx
  Filtering/itkStatisticsImageFilterWithMask.h
  Filtering/itkStatisticsImageFilterWithMask.txx
)

SET(CORE_ITKCOMMON_TRANSFORM_SRCS
  Transform/IRConnection.cxx
  Transform/IRConnection.hxx
  Transform/IRRefineTranslateCanvas.hxx
  Transform/IRRefineTranslateCanvas.cxx
  Transform/IRTransform.hxx
  Transform/IRTransform.cxx
  Transform/itkCascadedTransform.h
  Transform/itkDiscontinuousTransform.cxx
  Transform/itkDiscontinuousTransform.h
  Transform/itkGridTransform.h
  Transform/itkInverseTransform.h
  Transform/itkLegendrePolynomialTransform.h
  Transform/itkLegendrePolynomialTransform.txx
  Transform/itkMeshTransform.h
  Transform/itkNumericInverse.h
  Transform/itkRBFTransform.cxx
  Transform/itkRBFTransform.h
  Transform/itkRadialDistortionTransform.h
  Transform/itkRadialDistortionTransform.txx
  Transform/the_grid_transform.cxx
  Transform/the_grid_transform.hxx
)

SET(CORE_ITKCOMMON_OPTIMIZERS_SRCS
  Optimizers/itkImageMosaicVarianceMetric.h
  Optimizers/itkImageMosaicVarianceMetric.txx
  Optimizers/itkRegularStepGradientDescentOptimizer2.cxx
  Optimizers/itkRegularStepGradientDescentOptimizer2.h
  Optimizers/optimizer_observer.hxx
)

SET(CORE_ITKCOMMON_THREADUTILS_SRCS
  ThreadUtils/itk_terminator.hxx
  ThreadUtils/the_boost_mutex.cxx
  ThreadUtils/the_boost_mutex.hxx
  ThreadUtils/the_boost_thread.cxx
  ThreadUtils/the_boost_thread.hxx
  ThreadUtils/the_boost_thread_storage.hxx
  ThreadUtils/the_mutex_interface.cxx
  ThreadUtils/the_mutex_interface.hxx
  ThreadUtils/the_terminator.cxx
  ThreadUtils/the_terminator.hxx
  ThreadUtils/the_thread_interface.cxx
  ThreadUtils/the_thread_interface.hxx
  ThreadUtils/the_thread_pool.cxx
  ThreadUtils/the_thread_pool.hxx
  ThreadUtils/the_thread_storage.cxx
  ThreadUtils/the_thread_storage.hxx
  ThreadUtils/the_transaction.cxx
  ThreadUtils/the_transaction.hxx
)

SET(CORE_ITKCOMMON_FFT_SRCS
  FFT/fft.cxx
  FFT/fft.hxx
  FFT/fft_common.cxx
  FFT/fft_common.hxx
)

SET(CORE_ITKCOMMON_STOS_SRCS
  STOS/stos.hxx
  STOS/stos_common.cxx
  STOS/stos_common.hxx
)

CORE_ADD_LIBRARY(Core_ITKCommon
  ${CORE_ITKCOMMON_SRCS}
  ${CORE_ITKCOMMON_FILTERING_SRCS}
  ${CORE_ITKCOMMON_TRANSFORM_SRCS}
  ${CORE_ITKCOMMON_OPTIMIZERS_SRCS}
  ${CORE_ITKCOMMON_THREADUTILS_SRCS}
  ${CORE_ITKCOMMON_FFT_SRCS}
  ${CORE_ITKCOMMON_STOS_SRCS}
)

TARGET_LINK_LIBRARIES(Core_ITKCommon
  Core_Utils
  ${ITKCommon_LIBRARIES}
  ${ITKIOTransformBase_LIBRARIES}
  ${ITKFFT_LIBRARIES}
)

IF(BUILD_TESTING)
  ADD_SUBDIRECTORY(Transform/Tests)
ENDIF(BUILD_TESTING)


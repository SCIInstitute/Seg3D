
#
#  For more information, please see: http://software.sci.utah.edu
# 
#  The MIT License
# 
#  Copyright (c) 2009 Scientific Computing and Imaging Institute,
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

##################################################
# Set sources
##################################################

SET(APPLICATION_FILTERS_SRCS
)

SET(APPLICATION_FILTERS_ACTIONS_SRCS
  Actions/ActionAnisotropicDiffusion.h
  Actions/ActionAnisotropicDiffusion.cc
  Actions/ActionArithmetic.h
  Actions/ActionArithmetic.cc
  Actions/ActionBinaryDilateErode.h
  Actions/ActionBinaryDilateErode.cc
  Actions/ActionBoolean.h
  Actions/ActionBoolean.cc
  Actions/ActionCannyEdgeDetection.h
  Actions/ActionCannyEdgeDetection.cc
  Actions/ActionConfidenceConnected.h
  Actions/ActionConfidenceConnected.cc
  Actions/ActionConnectedComponent.h
  Actions/ActionConnectedComponent.cc
  Actions/ActionDiscreteGaussian.h
  Actions/ActionDiscreteGaussian.cc
  Actions/ActionFillHoles.h
  Actions/ActionFillHoles.cc
  Actions/ActionGradientMagnitude.h
  Actions/ActionGradientMagnitude.cc
  Actions/ActionHistogramEqualization.h
  Actions/ActionHistogramEqualization.cc
  Actions/ActionIntensityCorrection.h
  Actions/ActionIntensityCorrection.cc
  Actions/ActionInvert.h
  Actions/ActionInvert.cc
  Actions/ActionMaskData.h
  Actions/ActionMaskData.cc
  Actions/ActionMedian.h
  Actions/ActionMedian.cc
  Actions/ActionNeighborhoodConnected.h
  Actions/ActionNeighborhoodConnected.cc
  Actions/ActionOtsuThreshold.h
  Actions/ActionOtsuThreshold.cc
  Actions/ActionPaint.h
  Actions/ActionPaint.cc
  Actions/ActionPolyline.h
  Actions/ActionPolyline.cc
  Actions/ActionThresholdSegmentationLS.h
  Actions/ActionThresholdSegmentationLS.cc
  Actions/ActionThreshold.h
  Actions/ActionThreshold.cc
  )

ADD_LIBRARY(Application_Filters STATIC 
            ${APPLICATION_FILTERS_SRCS}
            ${APPLICATION_FILTERS_ACTIONS_SRCS} )
            
TARGET_LINK_LIBRARIES(Application_Filters
                      Core_Converter
                      Core_Utils
                      Core_EventHandler
                      Core_Application
                      Core_Interface
                      Core_Action
                      Core_State
                      Application_StateManager
                      Application_Layer
                      Application_LayerManager
                      ${SCI_BOOST_LIBRARY})

# Register actions
ADD_LIBRARY_ACTIONS(Application_Filters
                    ${APPLICATION_FILTERS_ACTIONS_SRCS})
                    
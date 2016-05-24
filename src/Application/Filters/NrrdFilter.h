/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2016 Scientific Computing and Imaging Institute,
 University of Utah.


 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.
 */
 
#ifndef APPLICATION_FILTERS_NRRDFILTER_H 
#define APPLICATION_FILTERS_NRRDFILTER_H
 
// Boost includes
#include <boost/smart_ptr.hpp> 
#include <boost/utility.hpp> 
 
// Core includes
#include <Core/DataBlock/NrrdData.h>
#include <Core/DataBlock/NrrdDataBlock.h>
#include <Core/DataBlock/MaskDataBlockManager.h>
#include <Core/Utils/Runnable.h>
#include <Core/Volume/DataVolume.h>
#include <Core/Volume/MaskVolume.h>
 
// Application includes
#include <Application/Layer/DataLayer.h> 
#include <Application/Layer/MaskLayer.h> 
#include <Application/Filters/LayerFilter.h>
 
namespace Seg3D
{

class NrrdFilter;


class NrrdFilter : public LayerFilter
{

public:
  NrrdFilter();
  virtual ~NrrdFilter();
    
protected:

  /// GET_NRRD_FROM_LAYER:
  /// Get a nrrd from a data or mask layer
  bool get_nrrd_from_layer( const LayerHandle& layer, Core::NrrdDataHandle& nrrd_data );

  /// CREATE_NRRD
  /// Short cut for generating nrrds
  bool create_nrrd( Core::NrrdDataHandle& nrrd_data );

  /// INSERT_NRRD_INTO_LAYER:
  /// Insert an nrrd back into a layer
  bool insert_nrrd_into_layer( const LayerHandle& layer, Core::NrrdDataHandle nrrd_data );
  
  /// UPDATE_PROGRESS:
  /// Teem does not support updating progress, hence we add a small amount before and a large
  /// amount after the filter was run
  void update_progress( LayerHandle layer, double amount );

};
  
} // end namespace Seg3D

#endif

/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2009 Scientific Computing and Imaging Institute,
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

#ifndef APPLICATION_LAYERMANAGER_LAYERMANAGER_H
#define APPLICATION_LAYERMANAGER_LAYERMANAGER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// Boost includes
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

// Core includes
#include <Utils/Core/StringUtil.h>
#include <Utils/Singleton/Singleton.h>
#include <Utils/Core/Log.h>
#include <Utils/Core/Exception.h>

// Application includes
#include <Application/Layer/Layer.h>
#include <Application/State/StateHandler.h>

namespace Seg3D
{

// Forward declaration
class LayerManager;

// Class definition
class LayerManager : public Utils::Singleton< LayerManager >
{

  // -- Constructor/Destructor --

private:
  friend class Utils::Singleton< LayerManager >;
  LayerManager();
  virtual ~LayerManager();
  
protected:
  friend class ActionCloneLayer;
  friend class ActionInsertLayerAbove;
  friend class ActionNewMaskLayer;
  friend class ActionRemoveLayer;
  
  bool insert_layer_above( const std::string& above_layer_name, std::string& below_layer_name ); 

public:
  // -- Signal/Slots --
  typedef boost::signals2::signal< void( LayerHandle ) > layer_signal_type;
  layer_signal_type layer_changed_signal;
  
};

} // end namespace seg3D

#endif

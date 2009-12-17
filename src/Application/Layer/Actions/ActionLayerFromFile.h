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

/*
 *****************************************************************************
 *
 *   ActionLayerFromFile.h
 *
 *   Create a new layer using data from a file and give it to the layer manager.
 *
 *   Also see: Layer, LayerManager
 *
 *   Authors:
 *      Kristen Zygmunt   -- initial attempt      11/19/2009
 *
 *    
 *****************************************************************************
 */

#ifndef APPLICATION_LAYER_ACTIONS_ACTIONLAYERFROMFILE_H
#define APPLICATION_LAYER_ACTIONS_ACTIONLAYERFROMFILE_H 1

//#ifdef (_MSC_VER) && (_MSC_VER >= 1020)
//# pragma once
//#endif

// STL includes

// Boost includes 

// ITK includes

// Application includes
#include <Application/Layer/Actions/ActionLayer.h>

namespace Seg3D {

// Forward declarations

// typedefs

// Class declarations
class ActionLayerFromFile : public ActionLayer
{
  SCI_ACTION_TYPE("LayerFromFile","",LAYER_E)
public:
  ActionLayerFromFile()
  {
    add_argument(filename_);
  }

  virtual ~ActionLayerFromFile()
  {}

  void set(const std::string& filename)
  {
    filename_.value() = filename;
  }

  virtual bool do_validate(ActionHandle& self,ActionContextHandle& context);
  virtual bool check_layer_availability();
  virtual bool lock_layers() const;
  virtual bool release_layers() const;
  virtual bool execute(ActionHandle& self,ActionContextHandle& context) const;

private:
  ActionParameter<std::string> filename_;

};

  typedef boost::shared_ptr<ActionLayerFromFile> ActionLayerFromFileHandle;

} // end namespace Seg3D

#endif

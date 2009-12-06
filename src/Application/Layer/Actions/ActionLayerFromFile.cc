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
 *   ActionLayerFromFile.cc
 *
 *   Also see: Layer, LayerManager
 *
 *   Authors:
 *      Kristen Zygmunt   -- initial attempt      11/19/2009
 *
 *    
 *****************************************************************************
 */

// STL includes

// Boost includes 

// ITK includes

// Application includes
#include <Application/Layer/LayerManager.h>
#include <Application/Layer/Layer.h>
#include <Application/Layer/Actions/ActionLayerFromFile.h>

namespace Seg3D {

SCI_REGISTER_ACTION(LayerFromFile);

bool
ActionLayerFromFile::check_layer_availability()
{
  // since we aren't dependent on any existing layers,
  // this is always true
  return (true);
}

bool
ActionLayerFromFile::lock_layers() const
{
  return (true);
}

bool
ActionLayerFromFile::release_layers() const
{
  // no initial layer to release, but does the new one created
  // need to be unlocked here?
  return (true);
}



bool 
ActionLayerFromFile::do_validate(ActionContextHandle& context)
{
  // TODO: verify that the filename provided is a valid, readable file
  return (true);
}

bool 
ActionLayerFromFile::execute(ActionContextHandle& context) const
{
  // Read data in from file
  // Populate a layer handle
  // Give the new layer to the LayerManager
  return (true);
}

} // end namespace Seg3D

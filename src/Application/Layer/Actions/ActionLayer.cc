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
 *   ActionLayer.cc
 *
 *   Authors:
 *      Kristen Zygmunt   -- initial attempt      11/24/2009
 *
 *    
 *****************************************************************************
 */


// STL includes

// Boost includes 
#include <boost/utility.hpp>

// ITK includes

#include <Application/Layer/LayerManager.h>
#include <Application/Layer/Actions/ActionLayer.h>

namespace Seg3D {

SCI_REGISTER_ACTION(Layer);

bool 
ActionLayer::validate(ActionContextHandle& context)
{
  return (check_layer_availability() && do_validate(context));
}

bool 
ActionLayer::run(ActionContextHandle& context)
{
  if (!lock_layers()) {
    context->report_error(std::string("Unable to lock layers"));
    return (false);
  }
  
  bool run_success = execute(context);

  if (!release_layers()) {
    context->report_error(std::string("Unable to unlock layers"));
  }
  
  return (run_success);
}

bool
ActionLayer::do_validate(ActionContextHandle& context) { return true; }

bool 
ActionLayer::check_layer_availability() { return true;}

bool
ActionLayer::lock_layers() const { return true; }

bool 
ActionLayer::release_layers() const { return true; }

bool 
ActionLayer::execute(ActionContextHandle& context) const { return true; }

} // end namespace Seg3D

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

// STL includes
#include <algorithm>

// Application includes
#include <Application/State/StateView3D.h>

namespace Seg3D {

StateView3D::StateView3D()
{
}


StateView3D::~StateView3D()
{
}  

std::string 
StateView3D::export_to_string() const
{
  return (Utils::export_to_string(value_));
}

bool 
StateView3D::import_from_string(const std::string& str,
                                bool from_interface)
{
  // Lock the state engine so no other thread will be accessing it
  StateEngine::lock_type lock(StateEngine::Instance()->get_mutex());

  return (Utils::import_from_string(str,value_));
}

void 
StateView3D::export_to_variant(ActionParameterVariant& variant) const
{
  variant.set_value(value_);
}

bool 
StateView3D::import_from_variant(ActionParameterVariant& variant,
                                 bool from_interface)
{
  // Lock the state engine so no other thread will be accessing it
  StateEngine::lock_type lock(StateEngine::Instance()->get_mutex());

  Utils::View3D value;
  if (!(variant.get_value(value))) return (false);

  if (value != value_)
  {
    value_ = value;
    value_changed_signal(value_,from_interface);
  }
  return (true);
}

bool 
StateView3D::validate_variant(ActionParameterVariant& variant, 
                              std::string& error)
{
  Utils::View3D value;
  if (!(variant.get_value(value)))
  {
    error = "Cannot convert the value '"+variant.export_to_string()+
            "' to a 3D Camera position";
    return (false);
  }
  
  error = "";
  return (true);
}

} // end namespace Seg3D

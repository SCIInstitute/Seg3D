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

#include <Core/Geometry/Measurement.h>

namespace Core
{

std::string Measurement::get_view_name() const
{
  std::string view_axis_str = "";
  if( this->view_axis_ == AXIAL_E ) view_axis_str = "AXIAL";
  else if( this->view_axis_ == CORONAL_E ) view_axis_str = "CORONAL";
  else if( this->view_axis_ == SAGITTAL_E ) view_axis_str = "SAGITTAL";
  else if( this->view_axis_ == VOLUME_E ) view_axis_str = "VOLUME";
  else if( this->view_axis_ == MIP_E ) view_axis_str = "MIP";
  else if( this->view_axis_ == OBLIQUE_E ) view_axis_str = this->oblique_name_;
  return view_axis_str;
}

std::string ExportToString( const Measurement& value )
{
  return "";
}


std::string ExportToString( const std::vector< Measurement >& value )
{
  return "";
}

bool ImportFromString( const std::string& str, Measurement& value )
{
  return true;
}

bool ImportFromString( const std::string& str, std::vector< Measurement >& value )
{
  return true;
}

} // end namespace Core
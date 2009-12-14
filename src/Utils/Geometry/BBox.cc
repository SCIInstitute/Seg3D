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

#include <Utils/Geometry/BBox.h>

namespace Utils {

bool 
BBox::overlaps(const BBox & bb) const
{
  if( bb.cmin_.x() > cmax_.x() || bb.cmax_.x() < cmin_.x())
    return false;
  else if( bb.cmin_.y() > cmax_.y() || bb.cmax_.y() < cmin_.y())
    return false;
  else if( bb.cmin_.z() > cmax_.z() || bb.cmax_.z() < cmin_.z())
    return false;

  return true;
}

bool 
BBox::overlaps_inside(const BBox & bb) const
{
  if( bb.cmin_.x() >= cmax_.x() || bb.cmax_.x() <= cmin_.x())
    return false;
  else if( bb.cmin_.y() >= cmax_.y() || bb.cmax_.y() <= cmin_.y())
    return false;
  else if( bb.cmin_.z() >= cmax_.z() || bb.cmax_.z() <= cmin_.z())
    return false;

  return true;
}

std::ostream& operator<<( std::ostream& os, const BBox& bbox )
{
  os << '[' << bbox.cmin_.x() << ' ' << bbox.cmin_.y() << ' ' << bbox.cmin_.z() << 
    " ; " << bbox.cmax_.x() << ' ' << bbox.cmax_.y() << ' ' << bbox.cmax_.z() <<']';
  return os;
}

} // End namespace Utils

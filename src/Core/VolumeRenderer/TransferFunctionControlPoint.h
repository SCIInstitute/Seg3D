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

#ifndef CORE_VOLUMERENDERER_TRANSFERFUNCTIONCONTROLPOINT_H
#define CORE_VOLUMERENDERER_TRANSFERFUNCTIONCONTROLPOINT_H

#include <vector>
#include <string>

namespace Core
{

class TransferFunctionControlPoint;
typedef std::vector< TransferFunctionControlPoint > TransferFunctionControlPointVector;

class TransferFunctionControlPoint
{
public:
  TransferFunctionControlPoint() : value_( 0 ), opacity_( 0 ) {}

  TransferFunctionControlPoint( const float value, const float opacity ) :
    value_( value ), opacity_( opacity )
  {
  }

  ~TransferFunctionControlPoint() {}

  float get_value() const
  {
    return this->value_;
  }

  float get_opacity() const
  {
    return this->opacity_;
  }

  void set_value( const float value )
  {
    this->value_ = value;
  }

  void set_opacity( const float opacity )
  {
    this->opacity_ = opacity;
  }

  bool operator==( const TransferFunctionControlPoint& rhs ) const
  {
    return this->value_ == rhs.value_ && this->opacity_ == rhs.opacity_;
  }

  bool operator!=( const TransferFunctionControlPoint& rhs ) const
  {
    return !this->operator ==( rhs );
  }

  bool operator<( const TransferFunctionControlPoint& rhs ) const
  {
    return this->value_ < rhs.value_;
  }

private:
  float value_;
  float opacity_;
};

// EXPORTTOSTRING:
/// Export a vector of control points to string.
std::string ExportToString( const TransferFunctionControlPointVector& control_points );

// IMPORTFROMSTRING:
/// Import a vector of control points from string.
/// Returns true on success, otherwise false.
bool ImportFromString( const std::string& str, TransferFunctionControlPointVector& control_points );

// EXPORTTOSTRING:
/// Export a control points to string.
std::string ExportToString( const TransferFunctionControlPoint& control_point );

// IMPORTFROMSTRING:
/// Import a control points from string.
/// Returns true on success, otherwise false.
bool ImportFromString( const std::string& str, TransferFunctionControlPoint& control_point );

} // end namespace Core

#include <Core/State/StateVector.h>

namespace Core 
{
  
typedef StateVector< TransferFunctionControlPoint > StateTransferFunctionControlPointVector;
typedef boost::shared_ptr< StateTransferFunctionControlPointVector > 
StateTransferFunctionControlPointVectorHandle;

}

#endif

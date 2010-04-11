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

#ifndef APPLICATION_INTERFACE_STATUSBAR_H
#define APPLICATION_INTERFACE_STATUSBAR_H

#include <boost/shared_ptr.hpp>
#include <boost/signals2.hpp>

#include <Utils/Core/ConnectionHandler.h>
#include <Utils/Core/Singleton.h>
#include <Utils/Geometry/Point.h>

namespace Seg3D
{

class StatusBar;
class DataPointInfo;
typedef boost::shared_ptr< DataPointInfo > DataPointInfoHandle;

class DataPointInfo
{
public:
  DataPointInfo();
  DataPointInfo( const Utils::Point& index_coord, const Utils::Point& world_coord, double value );
  DataPointInfo( const DataPointInfo& copy );
  ~DataPointInfo() {}

  DataPointInfo& operator=( const DataPointInfo& copy );

  inline const Utils::Point& world_coord() const { return this->world_coord_; }
  inline const Utils::Point& index_coord() const { return this->index_coord_; }
  inline double value() const { return this->value_; }

private:
  Utils::Point index_coord_;
  Utils::Point world_coord_;
  double value_;
};

class StatusBar : private Utils::ConnectionHandler
{
  CORE_SINGLETON( StatusBar );
  
private:
  StatusBar();
  virtual ~StatusBar();

public:
  void set_data_point_info( DataPointInfoHandle data_point );
  void set_message( int msg_type, std::string message );

public:
  boost::signals2::signal< void ( DataPointInfoHandle ) > data_point_info_updated_signal_;
  boost::signals2::signal< void ( int, std::string ) > message_updated_signal_;
};

} // end namespace Seg3D

#endif
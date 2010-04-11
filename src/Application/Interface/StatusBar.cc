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

#include <Application/Interface/Interface.h>
#include <Application/Interface/StatusBar.h>

namespace Seg3D
{

DataPointInfo::DataPointInfo() :
  index_coord_( 0, 0, 0 ),
  world_coord_( 0, 0, 0 ),
  value_( 0 )
{
}

DataPointInfo::DataPointInfo( const Utils::Point& index_coord, 
  const Utils::Point& world_coord, double value ) :
  index_coord_( index_coord ),
  world_coord_( world_coord ),
  value_( value )
{
}

DataPointInfo::DataPointInfo( const DataPointInfo& copy ) :
  index_coord_( copy.index_coord_ ),
  world_coord_( copy.world_coord_ ),
  value_( copy.value_ )
{
}

DataPointInfo& DataPointInfo::operator=( const DataPointInfo& copy )
{
  this->index_coord_ = copy.index_coord_;
  this->world_coord_ = copy.world_coord_;
  this->value_ = copy.value_;

  return ( *this );
}

CORE_SINGLETON_IMPLEMENTATION( StatusBar );

StatusBar::StatusBar()
{
  this->add_connection( Interface::Instance()->interface_action_context()->
    action_message_signal_.connect( boost::bind( &StatusBar::set_message, this, _1, _2 ) ) );
}

StatusBar::~StatusBar()
{
  this->disconnect_all();
}

void StatusBar::set_data_point_info( DataPointInfoHandle data_point )
{
  this->data_point_info_updated_signal_( data_point );
}

void StatusBar::set_message( int msg_type, std::string message )
{
  this->message_updated_signal_( msg_type, message );
}

} // end namespace Seg3D
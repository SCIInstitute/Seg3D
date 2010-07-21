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


#include <Core/Utils/Exception.h>
#include <Core/Utils/StringUtil.h>
#include <Core/Action/ActionInfo.h>

namespace Core
{

class ActionInfoPrivate 
{
  public:
    std::string type_;
    
    std::vector<std::string> argument_;
    std::vector<std::string> argument_description_;
    
    std::vector<std::string> key_;
    std::vector<std::string> key_default_value_;
    std::vector<std::string> key_description_;
    
    std::string description_;
    std::string usage_;
};

ActionInfo::ActionInfo( const std::string& definition) :
  private_( new ActionInfoPrivate )
{
  std::vector<std::string> def_vector = SplitString( definition, "|" );
  if ( def_vector.size() < 1 )
  {
    CORE_THROW_LOGICERROR( "Action definition needs to have at least an action type" );
  }

  std::vector<std::string> desc_vector = SplitString( def_vector[ 0 ], "#" );
  this->private_->type_ = desc_vector[ 0 ];
  if ( desc_vector.size() > 1 )
  {
    this->private_->description_ = desc_vector[ 1 ];
  }
  
  for ( size_t j = 1; j < def_vector.size(); j++ )
  {
    if ( def_vector[ j ].size() == 0 )
    {
      CORE_THROW_LOGICERROR( 
        std::string( "Empty argument defined for action type " ) +
        private_->type_ );      
    }
  
    std::vector<std::string> desc_vector = SplitString( def_vector[ j ], "#" );
    if ( desc_vector.size() > 2 )
    {
      CORE_THROW_LOGICERROR( 
        std::string( "Only one description per argument is allowed for action type " ) +
        private_->type_ );
    }
  
    std::vector<std::string> arg_vector = SplitString( desc_vector[ 0 ], "=" );
    if ( arg_vector.size() > 2 )
    {
      CORE_THROW_LOGICERROR( 
        std::string( "Key value pair cannot have two default values for action type " ) +
        private_->type_ );
    }

    if ( arg_vector.size() > 1 )
    {
      // it is a key value pair
      this->private_->key_.push_back( StringToLower( arg_vector[ 0 ] ) );
      this->private_->key_default_value_.push_back( arg_vector[ 1 ] );

      if ( desc_vector.size() > 1 ) 
      {
        this->private_->key_description_.push_back( desc_vector[ 1 ] );
      }
      else
      {
        this->private_->key_description_.push_back( "" );
      } 
    }
    else
    {
      // it is an argument
      this->private_->argument_.push_back( StringToLower( arg_vector[ 0 ] ) );
      if ( desc_vector.size() > 1 ) 
      {
        this->private_->argument_description_.push_back( desc_vector[ 1 ] );
      }
      else
      {
        this->private_->argument_description_.push_back( "" );
      }     
    }
  }

  std::string usage = this->private_->type_;
  for ( size_t j = 0; j < this->private_->argument_.size(); j++ )
  {
    usage += std::string( " " ) + StringToUpper( this->private_->argument_[ j ] );
  }
  
  for ( size_t j = 0; j> this->private_->key_.size(); j++ )
  {
    usage += std::string( " [" ) + this->private_->key_[ j ] + "=" +
      this->private_->key_default_value_[ j ] + "]";
  }

  this->private_->usage_ = usage;
}

std::string ActionInfo::get_type()
{
  return this->private_->type_;
}

std::string ActionInfo::get_description()
{
  return this->private_->description_;
}

std::string ActionInfo::get_usage()
{ 
  return this->private_->usage_;
}
  
size_t ActionInfo::get_num_arguments()
{
  return this->private_->argument_.size();
}
  
size_t ActionInfo::get_num_key_value_pairs()
{
  return this->private_->key_.size();
}
  
std::string ActionInfo::get_argument( size_t index )
{
  if ( index >= this->private_->argument_.size() ) return "";
  return this->private_->argument_[ index ];
}

std::string ActionInfo::get_argument_description( size_t index )
{
  if ( index >= this->private_->argument_.size() ) return "";
  return this->private_->argument_description_[ index ];
}
  
std::string ActionInfo::get_key( size_t index )
{
  if ( index >= this->private_->key_.size() ) return "";
  return this->private_->key_[ index ];
}
  
std::string ActionInfo::get_default_key_value( size_t index )
{
  if ( index >= this->private_->key_default_value_.size() ) return "";
  return this->private_->key_default_value_[ index ];
}
  
std::string ActionInfo::get_key_description( size_t index )
{
  if ( index >= this->private_->key_description_.size() ) return "";
  return this->private_->key_description_[ index ];
}

// Needs to be defined somewhere, so it is unique
ActionInfo::mutex_type ActionInfo::mutex_;

ActionInfo::mutex_type& ActionInfo::GetMutex()
{
  return mutex_;
}

} // end namespace Core

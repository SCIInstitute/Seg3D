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

// Boost Includes
#include<boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

// Core includes
#include <Core/Utils/Log.h>
#include <Core/Utils/LogHistory.h>
#include <Core/Application/Application.h>

namespace Core
{

CORE_SINGLETON_IMPLEMENTATION( Application );

Application::Application()
{
  // The event handler needs to be started manually
  // This event handler will execute all the functions
  // that are send to it on the main application thread.
  start_eventhandler();
}

Application::~Application()
{
}

//This is a function to check parameters.
//This avoids accidentally putting data into the map that we dont want
bool Application::check_command_line_parameter( const std::string &key, std::string& value )
{
  lock_type lock( get_mutex() );

  if ( this->parameters_.find( key ) == this->parameters_.end() )
  {
    return false;
  }
  else
  {
    value = this->parameters_[ key ];
    return true;
  }
}

//This function sets parameters in the parameters map.
void Application::set_command_line_parameter( const std::string& key, const std::string& value )
{
  lock_type lock( get_mutex() );
  this->parameters_[ key ] = value;
}

// Function for parsing the command line parameters
void Application::parse_command_line_parameters( int argc, char **argv )
{
  lock_type lock( get_mutex() );

  typedef boost::tokenizer< boost::char_separator< char > > tokenizer;
  boost::char_separator< char > seperator( ":-=|;" );

  // parse through the command line arguments

  for ( int count = 1; count < argc; count++ )
  {
    std::string arg( argv[ count ] );
    tokenizer tokens( arg, seperator );
    std::vector< std::string > param_vector;

    for ( tokenizer::iterator tok_iter = tokens.begin(); tok_iter != tokens.end(); ++tok_iter )
    {
      param_vector.push_back( *tok_iter );
    }

    // Create empty string
    std::string value = "1";
    
    if ( param_vector.size() > 1 ) 
    { 
      value = param_vector[ 1 ];
    }
    if ( param_vector.size() > 0 )
    {
      std::string key = param_vector[ 0 ];
      this->parameters_[ key ] = value;
      SCI_LOG_MESSAGE( std::string("Set command line parameter key: ") + 
      key + ", value: " + value);
    }
  }
}


} // end namespace Core

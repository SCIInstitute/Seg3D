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
#include <boost/filesystem.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

// Core includes
#include <Core/Utils/Log.h>
#include <Core/Utils/LogHistory.h>
#include <Core/Utils/StringUtil.h>
#include <Core/Application/Application.h>

// Includes for platform specific functions to get directory to store temp files and user data
#ifdef _WIN32
#include <shlobj.h>    
#include <tlhelp32.h>
#include <windows.h>
#include <LMCons.h>
#else
#include <stdlib.h>
#endif

// Include CMake generated files
#include "ApplicationConfiguration.h"

namespace Core
{

CORE_SINGLETON_IMPLEMENTATION( Application );

Application::Application()
{
}

Application::~Application()
{
}


bool Application::is_command_line_parameter( const std::string &key )
{
  lock_type lock( get_mutex() );

  if ( this->parameters_.find( key ) == this->parameters_.end() ) return false;
  return true;
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
    }
  }
}

bool Application::get_user_directory( boost::filesystem::path& user_dir, bool config_path )
{
#ifdef _WIN32
  TCHAR dir[MAX_PATH];

  // Try to create the local application directory
  // If it already exists return the name of the directory.

  if( config_path )
  {
    if ( SUCCEEDED( SHGetFolderPath( 0, CSIDL_LOCAL_APPDATA, 0, 0, dir ) ) )
    {
      user_dir = boost::filesystem::path( dir );
      return true;
    }
    else
    {
      CORE_LOG_ERROR( std::string( "Could not get user directory." ) );
      return false;
    }
  }
  else
  {
    if ( SUCCEEDED( SHGetFolderPath( 0, CSIDL_MYDOCUMENTS, 0, 0, dir ) ) )
    {
      user_dir = boost::filesystem::path( dir );
      return true;
    }
    else
    {
      CORE_LOG_ERROR( std::string( "Could not get user directory." ) );
      return false;
    }
  }
#else
  
  if ( getenv( "HOME" ) )
  {
    user_dir = boost::filesystem::path( getenv( "HOME" ) );
    return true;
  }
  else
  {
    CORE_LOG_ERROR( std::string( "Could not get user directory." ) );
    return false;
  }
#endif
}

bool Application::get_user_desktop_directory( boost::filesystem::path& user_desktop_dir )
{
#ifdef _WIN32
  TCHAR dir[MAX_PATH];

  // Try to create the local application directory
  // If it already exists return the name of the directory.

  if ( SUCCEEDED( SHGetFolderPath( 0, CSIDL_DESKTOPDIRECTORY, 0, 0, dir ) ) )
  {
    user_desktop_dir = boost::filesystem::path( dir );
    return true;
  }
  else
  {
    CORE_LOG_ERROR( std::string( "Could not get user desktop directory." ) );
    return false;
  }


#else

  if ( getenv( "HOME" ) )
  {
    user_desktop_dir = boost::filesystem::path( getenv( "HOME" ) ) / "Desktop" / "";
    return true;
  }
  else
  {
    CORE_LOG_ERROR( std::string( "Could not get user directory." ) );
    return false;
  }
#endif
}




bool Application::get_config_directory( boost::filesystem::path& config_dir )
{
  boost::filesystem::path user_dir;
  if ( !( get_user_directory( user_dir, true ) ) ) return false;
  
#ifdef _WIN32 
  config_dir = user_dir / GetApplicationName();
#else
  std::string dot_app_name = std::string( "." ) + GetApplicationName();
  config_dir = user_dir / dot_app_name;
#endif
  
  if ( !( boost::filesystem::exists( config_dir ) ) )
  {
    if ( !( boost::filesystem::create_directory( config_dir ) ) )
    {
      CORE_LOG_ERROR( std::string( "Could not create directory: " ) + config_dir.string() );
      return ( false );
    }
    
    CORE_LOG_MESSAGE( std::string( "Created directory: " ) + config_dir.string() );
  }
  
  return ( true );
}

bool Application::get_user_name( std::string& user_name )
{
#ifdef _WIN32
  TCHAR name[UNLEN+1];
  DWORD length = UNLEN;

  if ( GetUserName( name, &length ) )
  {
    user_name = std::string( name );
    return true;
  }
  else
  {
    CORE_LOG_ERROR( std::string( "Could not resolve user name." ) );
    return false; 
  }
#else
  if ( getenv( "USER" ) )
  {
    user_name = std::string( getenv( "USER" ) );
    return true;
  }
  else
  {
    CORE_LOG_ERROR( std::string( "Could not resolve user name." ) );
    return false;
  }
#endif

}

void Application::log_start()
{
  CORE_LOG_MESSAGE( std::string( "Application: " ) + GetApplicationName() );
  CORE_LOG_MESSAGE( std::string( "Version:" ) + GetVersion() );
  CORE_LOG_MESSAGE( std::string( "64Bit:" )  + Core::ExportToString( Is64Bit() ) );
}

void Application::log_finish()
{
  CORE_LOG_MESSAGE( std::string( "-- Finished --" ) );
}

void Application::finish()
{
  Instance()->terminate_eventhandler();
}


bool Application::IsApplicationThread()
{
  return ( Instance()->is_eventhandler_thread() );
}

void Application::PostEvent( boost::function< void() > function )
{
  Instance()->post_event( function );
}

void Application::PostAndWaitEvent( boost::function< void() > function )
{
  Instance()->post_and_wait_event( function );
}

std::string Application::GetVersion()
{
  return CORE_APPLICATION_VERSION;
}

int Application::GetMajorVersion()
{
  return CORE_APPLICATION_MAJOR_VERSION;
}

int Application::GetMinorVersion()
{
  return CORE_APPLICATION_MINOR_VERSION;
}

int Application::GetPatchVersion()
{
  return CORE_APPLICATION_PATCH_VERSION;
}

bool Application::Is64Bit()
{
  return ( sizeof(void *) == 8 );
}

bool Application::Is32Bit()
{
  return ( sizeof(void *) == 4 );
}

std::string Application::GetApplicationName()
{
  return CORE_APPLICATION_NAME;
}

} // end namespace Core

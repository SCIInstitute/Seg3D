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
#include <psapi.h>
#else
#include <stdlib.h>
#include <sys/types.h>
#ifndef __APPLE__
#include <unistd.h>
#include <sys/sysinfo.h>
#else
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/sysctl.h>
#include <sys/param.h>
#include <sys/mount.h>
#endif
#endif

// Include CMake generated files
#include "ApplicationConfiguration.h"

namespace Core
{

class ApplicationPrivate
{
public:
  boost::filesystem::path app_filepath_;
  boost::filesystem::path app_filename_;  
  
  // NOTE:
  // OSX 10.5 or less has poor OpenGL support, hence we need to disable some features there
  bool is_osx_10_5_or_less_;
};

CORE_SINGLETON_IMPLEMENTATION( Application );

Application::Application() :
  private_( new ApplicationPrivate )
{
  this->private_->app_filepath_ = boost::filesystem::current_path();
  this->private_->is_osx_10_5_or_less_ = false;
  
#if defined( __APPLE__ )
  struct utsname os_name;
  int res = uname( &os_name );
  
  if (  res == 0 )
  {
    std::string os_version = os_name.release;
    int version;
    if ( Core::ImportFromString( os_version, version ) )
    {
      if ( version < 10 )
      {
        this->private_->is_osx_10_5_or_less_ = true;
            }
    }
  }

#endif
}

Application::~Application()
{
}


bool Application::is_osx_10_5_or_less()
{
  return this->private_->is_osx_10_5_or_less_;
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

std::string Application::get_argument(int idx)
{
  lock_type lock( get_mutex() );

    if (this->arguments_.size() <= idx) {
        return "";
    }
    return this->arguments_[idx];
}


//This function sets parameters in the parameters map.
void Application::set_command_line_parameter( const std::string& key, const std::string& value )
{
  lock_type lock( get_mutex() );
  this->parameters_[ key ] = value;
}

// Function for parsing the command line parameters
void Application::parse_command_line_parameters( int argc, char **argv, int num_arguments )
{
  lock_type lock( get_mutex() );

    this->arguments_.clear();
    this->parameters_.clear();

  this->private_->app_filename_ = boost::filesystem::path( argv[0] );
  this->private_->app_filepath_ = this->private_->app_filename_.parent_path();

  typedef boost::tokenizer< boost::char_separator< char > > tokenizer;
  boost::char_separator< char > seperator( ":=|; " );

    int count = 1;
    
    for ( ; count < argc && count < (num_arguments+1); count++ )
    {
        this->arguments_.push_back(argv[count]);
    }
  
  if( ( argc > count ) && ( boost::filesystem::exists( boost::filesystem::path( argv[ count ] ) ) ) )
  {
    std::string key = "file_to_open_on_start";
    this->parameters_[ key ] = std::string( argv[ count ] );
    count++;
  }

  // parse through the command line arguments

  for ( ; count < argc; count++ )
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
      if (key.size() > 1 && key[0] == '-')
      {
        if (key.size() > 2 && key[1] == '-')
        {
          key = key.substr(2, std::string::npos);
        }
        else
        {
          key = key.substr(1, std::string::npos);
        }
      }
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
        
        if (! boost::filesystem::exists( user_dir ) )
        {
      CORE_LOG_ERROR( std::string( "Could not get user directory." ) );
      return false;            
        }
        
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

        if (! boost::filesystem::exists( user_desktop_dir ) )
        {
      CORE_LOG_ERROR( std::string( "Could not get user desktop directory." ) );
      return false;            
        }


    return true;
  }
  else
  {
    CORE_LOG_ERROR( std::string( "Could not get user desktop directory." ) );
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
      return false;
    }
    
    CORE_LOG_MESSAGE( std::string( "Created directory: " ) + config_dir.string() );
  }
  
  return true;
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
  CORE_LOG_MESSAGE( std::string( "Application: " ) + GetUtilName() );
  CORE_LOG_MESSAGE( std::string( "Version: " ) + GetVersion() );
  CORE_LOG_MESSAGE( std::string( "64Bit: " )  + Core::ExportToString( Is64Bit() ) );
}

void Application::log_finish()
{
  CORE_LOG_MESSAGE( std::string( "-- Finished --" ) );
}

void Application::finish()
{
  this->terminate_eventhandler();
}

void Application::reset()
{
  // Make sure this function is being called from the application thread
  ASSERT_IS_APPLICATION_THREAD();
  // Lock the mutex so the reset operation will be atomic in the application thread.
  lock_type lock( this->get_mutex() );
  // Trigger the signal.
  this->reset_signal_();
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

std::string Application::GetApplicationNameAndVersion()
{
  return GetApplicationName() + " " + GetVersion();
}

std::string Application::GetAbout()
{
  return CORE_APPLICATION_ABOUT;
}

std::string util_name_ = "";

std::string Application::GetUtilName()
{
    if (util_name_.size() == 0)
    {
        return CORE_APPLICATION_NAME;
    }
    else
    {
        return util_name_;
    }
}

void Application::SetUtilName(const std::string& name )
{
    util_name_ = name;
}

Application::mutex_type& Application::GetMutex()
{
  return Instance()->get_mutex();
}

void Application::Reset()
{
  Instance()->reset();
}

long long Application::get_total_virtual_memory()
{
#ifdef _WIN32
  MEMORYSTATUSEX info;
    info.dwLength = sizeof( MEMORYSTATUSEX );
    GlobalMemoryStatusEx( &info );
    return info.ullTotalPageFile;
#else
#ifdef __APPLE__
  struct statfs stats;
  if ( statfs( "/", &stats ) == 0 )
  {
    return static_cast<long long>( stats.f_bsize * stats.f_bfree );
  }
  else
  {
    return 0;
  }
#else

  struct sysinfo info;
    sysinfo (&info);
    long long total_virtual_memory = info.totalram;

    total_virtual_memory += info.totalswap;
    total_virtual_memory *= info.mem_unit;
  return total_virtual_memory;
#endif  
#endif
}


long long Application::get_total_physical_memory()
{
#ifdef _WIN32
  MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof( MEMORYSTATUSEX );
    GlobalMemoryStatusEx( &memInfo );
    return memInfo.ullTotalPhys;
#else
#ifdef __APPLE__
  unsigned long long total_physical_memory;
  int name[ 2 ];
  name[ 0 ] = CTL_HW;
  name[ 1 ] = HW_MEMSIZE;
  size_t length = sizeof( unsigned long long );
  sysctl( name, 2, &total_physical_memory, &length, NULL, 0);
  return total_physical_memory;
#else
  struct sysinfo info;
    
    sysinfo (&info);
    long long total_physical_memory = info.totalram;
    total_physical_memory *= info.mem_unit;

  return total_physical_memory;
#endif
#endif
}

long long Application::get_total_addressable_memory()
{
#ifdef _WIN32
  if ( sizeof ( void* ) == 8 )
  {
    return 1ULL<<62;
  }
  else
  {
    return 1ULL<<31;
  }
#else
  if ( sizeof ( void* ) == 8 )
  {
    return 1ULL<<62;
  }
  else
  {
    return 1ULL<<32;
  }
#endif
}

long long Application::get_total_addressable_physical_memory()
{
  long long addressable = this->get_total_addressable_memory();
  long long physical = this->get_total_physical_memory();
  if ( physical > addressable ) return addressable; else return physical;
}


long long Application::get_my_virtual_memory_used()
{
#ifdef _WIN32
  /*PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo( GetCurrentProcess(), 
    reinterpret_cast< PPROCESS_MEMORY_COUNTERS >( &pmc ), 
    sizeof( pmc ) );
    return pmc.PrivateUsage;*/
  return 0;
#else
  return 0;
#endif
}

long long Application::get_my_physical_memory_used()
{
#ifdef _WIN32
  /*PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo( GetCurrentProcess(), 
    reinterpret_cast< PPROCESS_MEMORY_COUNTERS >( &pmc ), 
    sizeof( pmc ) );
    return pmc.WorkingSetSize;*/
  return 0;
#else
  return 0;
#endif
}

int Application::get_process_id()
{
#ifdef _WIN32
  return GetCurrentProcessId();
#else
  return getpid();
#endif 
}

bool Application::get_application_filepath( boost::filesystem::path& app_filepath )
{
  app_filepath = this->private_->app_filepath_;
  return true;
}

bool Application::get_application_filename( boost::filesystem::path& app_filename )
{
  app_filename = this->private_->app_filename_;
  return true;
}


} // end namespace Core

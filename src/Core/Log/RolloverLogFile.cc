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

// STL includes
#include <fstream>
#include <string>

// Boost includes
#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread/mutex.hpp>

// Core includes
#include <Core/Application/Application.h>
#include <Core/Log/RolloverLogFile.h>
#include <Core/Utils/Lockable.h>
#include <Core/Utils/Log.h>
#include <Core/Utils/StringUtil.h>

namespace Core
{

class RolloverLogFilePrivate : public RecursiveLockable
{
public:
  void log_message( unsigned int type, std::string message );
  // Rollover log files if needed, create new log file
  void rollover_log_files();
  bool create_new_log_file();

  unsigned int log_flags_; // Which type of log messages should be written to file
  std::ofstream ofstream_; // Closes file automatically on destruction
  int max_files_;
  int max_lines_;
  int max_age_days_;  
  int line_count_;
  boost::filesystem::path log_dir_;
  std::string log_file_prefix_;

  const static int SECONDS_PER_DAY_C;
  const static int DEFAULT_MAX_FILES_C;
  const static int DEFAULT_MAX_LINES_C;
  const static int DEFAULT_MAX_AGE_DAYS_C;
};

const int RolloverLogFilePrivate::SECONDS_PER_DAY_C = 60 * 60 * 24;
const int RolloverLogFilePrivate::DEFAULT_MAX_FILES_C = 10;
const int RolloverLogFilePrivate::DEFAULT_MAX_LINES_C = 5000;
const int RolloverLogFilePrivate::DEFAULT_MAX_AGE_DAYS_C = 7;

void RolloverLogFilePrivate::log_message( unsigned int type, std::string message )
{
  lock_type lock( this->get_mutex() );

  // Check whether we need to rollover the log files
  if( this->line_count_ >= this->max_lines_ )
  {
    this->rollover_log_files();
  }

  // Write message to file
  if ( this->ofstream_.is_open() && type & this->log_flags_  )
  {
    this->ofstream_ << message << std::endl;
    this->line_count_++;
  }
}

void RolloverLogFilePrivate::rollover_log_files()
{
  lock_type lock( this->get_mutex() );

  // Close the current log file
  if( this->ofstream_.is_open() )
  {
    this->ofstream_.close();
  }

  // Remove old/excess log files
  try // Catch any boost exceptions
  {
    // Make sure log directory exists 
    if( !boost::filesystem::exists( this->log_dir_ ) )
    {
      return;
    }

    // Remove all logs older than max age
    std::vector< std::pair< boost::filesystem::path, std::time_t > > log_files;
    boost::filesystem::directory_iterator end_itr; // default construction yields past-the-end
    // Iterator represents no particular order of files
    for ( boost::filesystem::directory_iterator itr( this->log_dir_ ); itr != end_itr; ++itr )
    {
      if ( boost::filesystem::is_regular_file( itr->status() ) )
      {
        // Log files are identified as starting with app name and ending with .log
        if( boost::algorithm::starts_with( itr->path().filename().string(), 
          Application::GetUtilName() ) && boost::filesystem::extension( itr->path() ) == ".log" )
        {
          std::time_t file_write_time = boost::filesystem::last_write_time( itr->path() );
          std::time_t current_time = std::time( 0 );
          double elapsed_days = static_cast< double >( ( current_time - file_write_time ) )
            / static_cast< double >( SECONDS_PER_DAY_C ); 
          if( elapsed_days > this->max_age_days_ )
          {
            boost::filesystem::remove( itr->path() );
          }
          else
          {
            log_files.push_back( std::make_pair( itr->path(), file_write_time ) );
          }
        }
      }
    }

    // If num log files exceeds (or will exceed) max log files, remove oldest logs 
    if( log_files.size() >= static_cast< size_t >( this->max_files_ ) )
    {
      // Sort files by date
      std::sort(  log_files.begin(), log_files.end(),
        boost::bind( &std::pair< boost::filesystem::path, std::time_t >::second, _1 ) <
        boost::bind( &std::pair< boost::filesystem::path, std::time_t >::second, _2 ) );

      for( size_t i = 0; i < log_files.size() - this->max_files_ + 1; i++ )
      {
        boost::filesystem::remove( log_files[ i ].first );
      }
    }
  }
  catch ( const std::exception & )
  {
    // Do nothing.  There is no current log to write an error to.
  }
  
  // Create new log file
  this->create_new_log_file();
}

bool RolloverLogFilePrivate::create_new_log_file()
{
  lock_type lock( this->get_mutex() );

  // Make sure log directory exists 
  if( !boost::filesystem::exists( this->log_dir_ ) )
  {
    return false;
  }

  // Create log file name 
  std::string date_time_str = 
    boost::posix_time::to_iso_string( boost::posix_time::second_clock::local_time() );
  // Using the process id prevents multiple instances of Seg3D from trying to create the same
  // log file.
  std::string process_id = ExportToString( Application::Instance()->get_process_id() );
  std::string log_filename = this->log_file_prefix_ + "_" + date_time_str + "_" + 
    process_id + ".log";
  
  boost::filesystem::path log_path = this->log_dir_ / log_filename;

  // Close any previously open stream
  if( this->ofstream_.is_open() ) 
  {
    this->ofstream_.close(); 
  }
  // Open a new output filestream
  this->ofstream_.open( log_path.string().c_str() );
  if( !this->ofstream_.is_open() ) 
  {
    return false;
  }

  // Reset line count to 0
  this->line_count_ = 0;

  return true;
}

RolloverLogFile::RolloverLogFile( unsigned int log_flags ) :
  private_( new RolloverLogFilePrivate )
{ 
  // post_log_signal could cause asynchronous call to log_message(), so need to protect private
  // members with mutex.
  RolloverLogFilePrivate::lock_type lock( this->private_->get_mutex() );

  // Initialize all private variables
  this->private_->log_flags_ = log_flags;
  this->private_->max_files_ = RolloverLogFilePrivate::DEFAULT_MAX_FILES_C;
  this->private_->max_lines_ = RolloverLogFilePrivate::DEFAULT_MAX_LINES_C;
  this->private_->max_age_days_ = RolloverLogFilePrivate::DEFAULT_MAX_AGE_DAYS_C;
  this->private_->line_count_ = 0;

  // Get/create log directory
  Application::Instance()->get_config_directory( this->private_->log_dir_ );

  // Build log file prefix
  this->private_->log_file_prefix_ = Application::GetUtilName() + "_" + Application::GetVersion();

  // Rollover log files if needed, and create new log file
  this->private_->rollover_log_files();

  // If the internals are destroyed, so should the connection:
  // we use the signals2 system to track the shared_ptr and destroy the connection
  // if the object is destroyed. This will ensure that when the slot is called
  // the shared_ptr is locked so that the object is not destroyed while the
  // call back is evaluated.
  Log::Instance()->post_log_signal_.connect(
    Log::post_log_signal_type::slot_type( &RolloverLogFilePrivate::log_message, 
    this->private_.get(), _1, _2 ).track( this->private_ ) );
}

void RolloverLogFile::set_max_files( int num_files )
{
  RolloverLogFilePrivate::lock_type lock( this->private_->get_mutex() );

  this->private_->max_files_ = num_files; 
}

void RolloverLogFile::set_max_lines( int num_lines )
{
  RolloverLogFilePrivate::lock_type lock( this->private_->get_mutex() );

  this->private_->max_lines_ = num_lines;
}

void RolloverLogFile::set_max_age( int num_days )
{
  RolloverLogFilePrivate::lock_type lock( this->private_->get_mutex() );

  this->private_->max_age_days_ = num_days;
}

} // end namespace

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

#ifndef APPLICATION_PROJECTMANAGER_PROJECTINFO_H
#define APPLICATION_PROJECTMANAGER_PROJECTINFO_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// STL includes
#include <string>
#include <vector>

// boost includes
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>

namespace Seg3D
{

// Forward declaration
class ProjectInfo;
typedef std::vector< ProjectInfo > ProjectInfoList;
typedef boost::shared_ptr< ProjectInfoList > ProjectInfoListHandle;

/// CLASS ProjectInfo
/// This helper class is for storing recent projects in the database
class ProjectInfo
{
public:
  typedef boost::posix_time::ptime timestamp_type;

  // Constructor
  ProjectInfo( const std::string& project_name, const boost::filesystem::path& project_path, 
    const timestamp_type& last_access_time ) :
    project_name_( project_name ),
    path_( project_path ),
    last_access_time_( last_access_time )
  {
  }

  // Default constructor
  ProjectInfo() {}

  // Destructor
  ~ProjectInfo() {}
  
  const std::string& name() const
  {
    return this->project_name_;
  }

  const boost::filesystem::path& path() const
  {
    return this->path_;
  }

  const timestamp_type& last_access_time() const
  {
    return this->last_access_time_;
  }
  
private:
  /// Name of the project
  std::string project_name_;
  /// Path of the project file
  boost::filesystem::path path_;
  /// Timestamp of when the project was last opened in UTC time
  timestamp_type last_access_time_;
};  

} // end namespace Seg3D

#endif

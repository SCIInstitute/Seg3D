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

#ifndef APPLICATION_PROJECTMANAGER_PROJECTMANAGER_H
#define APPLICATION_PROJECTMANAGER_PROJECTMANAGER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 



// Boost includes
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

// Core includes
#include <Core/Utils/StringUtil.h>
#include <Core/Utils/Singleton.h>
#include <Core/Utils/Log.h>
#include <Core/Utils/Exception.h>

// Application includes
#include <Core/State/StateHandler.h>
#include <Application/Project/Project.h>

namespace Seg3D
{

// Forward declaration
class ProjectManager;
  

// Class definition
class ProjectManager : public Core::StateHandler
{
  CORE_SINGLETON( ProjectManager );

  // -- Constructor/Destructor --
private:
  ProjectManager();
  virtual ~ProjectManager();
  

  
public:
  void new_project( const std::string& project_name, bool consolidate );
  void open_project( const std::string& project_path, const std::string& project_name );
  void save_project();
  void save_project_as();
  void save_projectmanager_state();
  
public:
  Core::StateStringVectorHandle recent_projects_state_;
  Core::StateStringHandle current_project_path_state_;
  ProjectHandle current_project_;
  
private:
  void initialize();
  void add_to_recent_projects( const std::string& project_path, const std::string& project_name );
  
private:
  
  std::vector< Core::Color > project_colors_;
  boost::filesystem::path local_projectmanager_path_;

};

} // end namespace seg3D

#endif


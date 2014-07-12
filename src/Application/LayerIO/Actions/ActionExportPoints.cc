/*
 For more information, please see: http://software.sci.utah.edu
 
 The MIT License
 
 Copyright (c) 2014 Scientific Computing and Imaging Institute,
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

// Boost includes
#include <boost/filesystem.hpp>

#include <sstream>
#include <iomanip>
#include <fstream>

// Application includes
#include <Application/LayerIO/Actions/ActionExportPoints.h>
#include <Application/LayerIO/LayerIO.h>
#include <Application/Layer/LayerManager.h>
#include <Application/ProjectManager/ProjectManager.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
CORE_REGISTER_ACTION( Seg3D, ExportPoints )

namespace Seg3D
{

bool ActionExportPoints::validate( Core::ActionContextHandle& context )
{
  if ( this->file_path_.empty() )
  {
    context->report_error( std::string("File path is empty.") );
    return false;
  }

  if ( this->points_.empty() )
  {
    context->report_error( std::string("No points available to save.") );
    return false;
  }
  return true; // validated
}

bool ActionExportPoints::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{
  std::string message = std::string( "Exporting list of points." );
  
  Core::ActionProgressHandle progress = 
  Core::ActionProgressHandle( new Core::ActionProgress( message ) );
  
  progress->begin_progress_reporting();
  
  boost::filesystem::path filename = boost::filesystem::path( this->file_path_ );

  std::ofstream outputfile;
  outputfile.exceptions( std::ofstream::failbit | std::ofstream::badbit );
  
  try
  {
    outputfile.open( filename.string().c_str() );            
    
    std::ios_base::fmtflags ff;
    ff = outputfile.flags();
    ff |= outputfile.showpoint; // write floating-point values including always the decimal point
    ff |= outputfile.fixed; // write floating point values in fixed-point notation
    outputfile.flags(ff);

    PointVector::const_iterator iter = this->points_.begin();
    PointVector::const_iterator end = this->points_.end();
    
    while (iter != end)
    {
      outputfile << iter->x() << " " << iter->y() << " " << iter->z() << std::endl;
      ++iter;
    }
  }     
  catch (...)
  {
    context->report_error( "Could not open and write to file: " + filename.string() );
    return false;
  }
  outputfile.close();
  
  progress->end_progress_reporting();
  
  return true;
}

void ActionExportPoints::Dispatch( Core::ActionContextHandle context,
                                   const std::string& file_path,
                                   const PointVector& points )
{
  // Create new action
  ActionExportPoints* action = new ActionExportPoints;
  
  action->file_path_ = file_path;
  action->points_ = points;
  
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

} // end namespace Seg3D

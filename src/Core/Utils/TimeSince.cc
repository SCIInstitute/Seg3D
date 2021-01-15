/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2021 Scientific Computing and Imaging Institute,
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

//Remove if not needed!
/*#ifdef __APPLE__
// for Mac URL handling
#include <CoreFoundation/CoreFoundation.h>
#endif*/

// Boost
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>

// Qt
#include <QtGui>

//Core Includes
#include <Core/Interface/Interface.h>
#include <Core/Utils/TimeSince.h>

//Application Includes
//Remove what isn't needed
#include <Application/Tool/Tool.h>
#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerManager.h>
#include <Application/Layer/LayerGroup.h>
#include <Application/LayerIO/LayerIO.h>
#include <Application/LayerIO/Actions/ActionExportLayer.h>
#include <Application/LayerIO/NrrdLayerExporter.h>
#include <Application/ProjectManager/ProjectManager.h>
#include <Application/ScriptManager/ScriptManager.h>

//Remove if not needed!
/*#include <Corview/Modules/Region/RegionModule.h>
#include <Corview/Modules/Region/RegionManager.h>
#include <Corview/Modules/Analysis/AnalysisModule.h>
#include <Corview/Threading/AnalysisThread.h>*/

#include <tinyxml.h>

// STL
#include <iostream>
#include <map>

#ifdef WIN32
#include <windows.h>
#include <stdio.h>
#include <psapi.h>
#endif

namespace Core {

class TimeSincePrivate {
public:
  static std::map<std::string, boost::posix_time::ptime> timer_map_;
};
std::map<std::string, boost::posix_time::ptime> TimeSincePrivate::timer_map_;

//---------------------------------------------------------------------------
void TimeSince::start_timer( std::string name )
{
  boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
  TimeSincePrivate::timer_map_[name] = time;
}

//---------------------------------------------------------------------------
double TimeSince::get_time_since( std::string name )
{
  boost::posix_time::ptime time_before = TimeSincePrivate::timer_map_[name];
  boost::posix_time::ptime time_now = boost::posix_time::microsec_clock::local_time();
  boost::posix_time::time_duration msdiff = time_now - time_before;
  return msdiff.total_milliseconds();
}

//---------------------------------------------------------------------------
double TimeSince::get_seconds_since( std::string name )
{
  double ms = TimeSince::get_time_since( name );
  return ms / 1000.0;
}
//---------------------------------------------------------------------------
} //end namespace Core

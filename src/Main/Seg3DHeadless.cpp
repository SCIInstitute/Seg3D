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

#ifdef _MSC_VER
#pragma warning( disable: 4244 )
#endif

#include <csignal>
#include <string>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>

#include "Core/Utils/StringUtil.h"
#include <Core/Utils/Log.h>

#include "Seg3DHeadless.h"

namespace Seg3D
{

bool seg3d_forever = true;
static void sighandler(int sig)
{
  CORE_LOG_MESSAGE( std::string("Received signal: ") + Core::ExportToString( sig ) );
  seg3d_forever = false;
}

void Seg3DHeadless::warning(std::string& message)
{
  CORE_LOG_WARNING(message);
}

bool Seg3DHeadless::keep_running()
{
  if (this->python_script != "")
    return false;
  else
    return seg3d_forever;
}

bool Seg3DHeadless::run()
{
  signal(SIGABRT, &sighandler);
  signal(SIGTERM, &sighandler);
  signal(SIGINT, &sighandler);
  while(this->keep_running())
  {
    // do nothing, check every second for a termination signal
    boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
  }

  return true;
}

} //namespace Seg3D

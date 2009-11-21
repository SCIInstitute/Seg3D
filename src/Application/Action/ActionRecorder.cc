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

#include <Application/Action/ActionDispatcher.h>
#include <Application/Action/ActionRecorder.h>

namespace Seg3D {

ActionRecorder::ActionRecorder(std::ostream* stream) :
  action_stream_(stream)
{
}

ActionRecorder::~ActionRecorder()
{
}

void
ActionRecorder::start()
{
  if (!(connection_.connected()))
  {
    connection_ = ActionDispatcher::instance()->connect_observer(
      boost::bind(&ActionRecorder::record,this,_1));
  }
}

void 
ActionRecorder::stop()
{
  if (connection_.connected())
  {
    connection_.disconnect();
  }
}

void
ActionRecorder::record(ActionHandle action)
{
  (*action_stream_) << "ACTION: " << action->export_action_to_string() << std::endl;
}

} // end namespace Seg3D


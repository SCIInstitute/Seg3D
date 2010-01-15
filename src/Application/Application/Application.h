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

#ifndef APPLICATION_APPLICATION_APPLICATION_H
#define APPLICATION_APPLICATION_APPLICATION_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// Utils includes
#include <Utils/Core/Log.h>
#include <Utils/Singleton/Singleton.h>
#include <Utils/EventHandler/EventHandler.h>

namespace Seg3D {

// CLASS APPLICATION:
// Application is the thread that processes all the actions in the program.
 
class Application;
 
class Application : public Utils::EventHandler  {

// -- Constructor/Destructor --
  public:
    Application();

// -- Application thread --    
    
    // ISAPPLICATIONTHREAD:
    // Test whether the current thread is the application thread
    static bool IsApplicationThread()
      { return (Instance()->is_eventhandler_thread()); }

    // POSTEVENT:
    // Short cut to the event handler
    static void PostEvent(boost::function<void ()> function)
      { Instance()->post_event(function); }

    // POSTANDWAITEVENT:
    // Short cut to the event handler
    static void PostAndWaitEvent(boost::function<void ()> function)
      { Instance()->post_and_wait_event(function); }
  
    // CHECKCOMMANDLINEPARAMETERS
    // check to see if a particular parameter has been placed into the map
    // if so it returns the value as a string
    std::string checkCommandLineParameter( const std::string &key );
  
    // SETPARAMETER
    // put parameters from the command line into a map
    void setParameter( const std::string &key, const std::string &val );

// -- Singleton interface --
  public:
    
    // INSTANCE:
    // Get the singleton interface
    static Application* Instance() { return instance_.instance(); }
    
  private:  
    static Utils::Singleton<Application> instance_;
    
};

} // end namespace Seg3D

#endif

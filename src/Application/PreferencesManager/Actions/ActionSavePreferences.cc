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

#include <Core/Action/ActionDispatcher.h>
#include <Core/Action/ActionFactory.h>

// Application Includes
#include <Application/PreferencesManager/PreferencesManager.h>
#include <Application/PreferencesManager/Actions/ActionSavePreferences.h>
#include <Application/Tool/Actions/ActionSaveToolPreferences.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
CORE_REGISTER_ACTION( Seg3D, SavePreferences )

namespace Seg3D
{

bool ActionSavePreferences::validate( Core::ActionContextHandle& context )
{
  return true; // validated
}

bool ActionSavePreferences::run( Core::ActionContextHandle& context, 
  Core::ActionResultHandle& result )
{
//  std::string message = std::string("Please wait, while your preferences are being saved...");
//
//  Core::ActionProgressHandle progress = 
//    Core::ActionProgressHandle( new Core::ActionProgress( message ) );
//
//  progress->begin_progress_reporting();

  PreferencesManager::Instance()->save_state();
  ActionSaveToolPreferences::Dispatch( Core::Interface::GetWidgetActionContext() );

//  progress->end_progress_reporting();

  return true;
}

void ActionSavePreferences::Dispatch( Core::ActionContextHandle context )
{
  ActionSavePreferences* action = new ActionSavePreferences;
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

} // end namespace Seg3D

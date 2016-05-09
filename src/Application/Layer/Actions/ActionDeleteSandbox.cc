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

// Core includes
#include <Core/Action/ActionFactory.h>

// Application includes
#include <Application/Clipboard/Clipboard.h>
#include <Application/Layer/Actions/ActionDeleteSandbox.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
CORE_REGISTER_ACTION( Seg3D, DeleteSandbox )

namespace Seg3D
{

bool ActionDeleteSandbox::validate( Core::ActionContextHandle& context )
{
  return true; // validated
}

bool ActionDeleteSandbox::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{ 
  if ( !LayerManager::Instance()->delete_sandbox( this->sandbox_ ) )
  {
    context->report_error( "Sandbox " + Core::ExportToString( this->sandbox_ ) + " doesn't exist." );
    return false;
  }

  // NOTE: sandbox must be deleted from both the layer manager and the clipboard
  Clipboard::Instance()->delete_sandbox( this->sandbox_ );
    
  return true;
}

void ActionDeleteSandbox::Dispatch( Core::ActionContextHandle context, SandboxID sandbox )
{
  ActionDeleteSandbox* action = new ActionDeleteSandbox;
  action->sandbox_ = sandbox;
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

} // end namespace Seg3D

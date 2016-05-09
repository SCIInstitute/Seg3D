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

// Application includes
#include <Application/Clipboard/Clipboard.h>
#include <Application/Layer/LayerManager.h>
#include <Application/Layer/Actions/ActionCreateSandbox.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
CORE_REGISTER_ACTION( Seg3D, CreateSandbox )

namespace Seg3D
{

bool ActionCreateSandbox::validate( Core::ActionContextHandle& context )
{
  // Nothing to validate
  return true;
}

bool ActionCreateSandbox::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{ 
  SandboxID sandbox_id = LayerManager::Instance()->create_sandbox();
  // A valid sandbox ID is no less than 0
  if ( sandbox_id < 0 )
  {
    context->report_error( "Failed to create a layer sandbox." );
    return false;
  }

  // NOTE: sandbox must be created in both the layer manager and the clipboard
  Clipboard::Instance()->create_sandbox( sandbox_id );
  
  // Report the sandbox ID to action result
  result.reset( new Core::ActionResult( sandbox_id ) );

  return true;
}

} // end namespace Seg3D

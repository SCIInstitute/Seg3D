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
#pragma warning( disable: 4244 4267 )
#endif

// Core includes
#include <Core/Application/Application.h>

// QtUtils includes
#include <QtUtils/Utils/QtApplication.h>

#include "Seg3DGui.h"
#include "Seg3DHeadless.h"

///////////////////////////////////////////////////////////
// Main Seg3D entry point
///////////////////////////////////////////////////////////

using namespace Seg3D;


int main( int argc, char **argv )
{
  Core::Application::Instance()->parse_command_line_parameters( argc, argv );
  Seg3DBase* app = NULL;
  bool headless = Core::Application::Instance()->is_command_line_parameter( "headless" );
  if ( headless )
    app = new Seg3DHeadless();
  else
  {
	app = new Seg3DGui();
	if (!(QtUtils::QtApplication::Instance()->setup(argc, argv))) return (-1);
  }

  if (app->information_only())
	return 0; 

  if (!app->initialize())
    return -1;

  app->run();
  app->close();

  delete app;

  if (headless)
    return ( 0 );

#if defined (_WIN32) || defined(__APPLE__)
  return ( 0 );
#else
    // NOTE: On Linux Qt tends to crash in one of its static destructors. Since we do not need these
    // destructors to be executed, we just exit in stead. For Windows we return to WinMain, hence
    // we need to return in that case.
    exit ( 0 );
#endif
}

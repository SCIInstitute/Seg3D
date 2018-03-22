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

#include <string>

#include <Core/Utils/Log.h>

// Interface includes
#include <Interface/Application/ApplicationInterface.h>

// QtUtils includes
#include <QtUtils/Utils/QtApplication.h>

#include "Seg3DGui.h"

namespace Seg3D
{

void Seg3DGui::warning(std::string& message)
{
  QMessageBox::information( 0,
    QString::fromStdString( Core::Application::GetApplicationNameAndVersion() ),
    QString::fromStdString( message ) );
}

bool Seg3DGui::run()
{
  ApplicationInterface* app_interface = new ApplicationInterface();

  bool opened_init_project = app_interface->open_initial_project( this->file_to_view );

  if (!opened_init_project && this->display_splash_screen) {
//Trying to catch osx file open event when double click on .nrrd file. hasPendingEvents is obsolete at 5.10.
#if defined(__APPLE__)
      if (QtUtils::QtApplication::Instance()->qt_application()->hasPendingEvents())
      {
        QtUtils::QtApplication::Instance()->qt_application()->processEvents();
        if (this->file_to_view == "") {
            app_interface->activate_splash_screen();
          }
      }
#else
    app_interface->activate_splash_screen();
#endif
  }

  // Show the full interface
  app_interface->show();

  // Put the interface on top of all the other windows
  app_interface->raise();

  // The application window needs the qApplication as parent, which is
  // defined in the QtApplication, which integrates the Qt eventloop with
  // the interface eventloop of the Application layer.

  // -- Run QT event loop --
  return QtUtils::QtApplication::Instance()->exec();
}

} //namespace Seg3D

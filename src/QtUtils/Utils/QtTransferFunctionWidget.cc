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

// STL includes
#include <iostream>

// Glew includes
#include <GL/glew.h>

// Qt includes


// Core includes
#include <Core/Interface/Interface.h>
#include <Core/RenderResources/RenderResources.h> 

// QtUtils includes
#include <QtUtils/Utils/QtTransferFunctionWidget.h>
#include <QtUtils/Utils/QtApplication.h>

namespace QtUtils
{

//////////////////////////////////////////////////////////////////////////
// Class QtTransferFunctionWidgetPrivate
//////////////////////////////////////////////////////////////////////////

class QtTransferFunctionWidgetPrivate
{
public:
  Core::Texture2DHandle bkg_tex_;
  Core::TransferFunctionHandle transfer_function_;
  QtTransferFunctionWidget* tf_widget_;
};

//////////////////////////////////////////////////////////////////////////
// Class QtTransferFunctionWidget
//////////////////////////////////////////////////////////////////////////

typedef QPointer< QtTransferFunctionWidget > QtTransferFunctionWidgetWeakHandle;

QtTransferFunctionWidget::QtTransferFunctionWidget( const QGLFormat& format, QWidget* parent, 
                 QGLWidget* share, Core::TransferFunctionHandle tf ) :
  QGLWidget( format, parent, share ),
  private_( new QtTransferFunctionWidgetPrivate )
{
  this->private_->transfer_function_ = tf;
  this->private_->tf_widget_ = this;
}

QtTransferFunctionWidget::~QtTransferFunctionWidget()
{
  this->disconnect_all();
}

static void UpdateDisplay( QtTransferFunctionWidgetWeakHandle qpointer )
{
  if ( !Core::Interface::IsInterfaceThread() )
  {
    Core::Interface::PostEvent( boost::bind( &UpdateDisplay, qpointer ) );
    return;
  }

  if ( !qpointer.isNull() && !QCoreApplication::closingDown() )
  {
    qpointer->updateGL();
  }
}

void QtTransferFunctionWidget::initializeGL()
{
  glClearColor( 0.3f, 0.3f, 0.3f, 1.0f );

  if ( Core::RenderResources::Instance()->valid_render_resources() )
  {
    Core::Texture::SetActiveTextureUnit( 0 );
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
    Core::Texture::SetActiveTextureUnit( 1 );
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

    unsigned char checker[] = { 255, 0, 0, 255 };
    this->private_->bkg_tex_.reset( new Core::Texture2D );
    this->private_->bkg_tex_->bind();
    this->private_->bkg_tex_->set_wrap_s( GL_REPEAT );
    this->private_->bkg_tex_->set_wrap_t( GL_REPEAT );
    this->private_->bkg_tex_->set_min_filter( GL_NEAREST );
    this->private_->bkg_tex_->set_mag_filter( GL_NEAREST );
    this->private_->bkg_tex_->set_image( 2, 2, GL_LUMINANCE, checker, 
      GL_LUMINANCE, GL_UNSIGNED_BYTE );
    this->private_->bkg_tex_->unbind();

    QtTransferFunctionWidgetWeakHandle qpointer( this );
    this->add_connection( this->private_->transfer_function_->transfer_function_changed_signal_.
      connect( boost::bind( &UpdateDisplay, qpointer ) ) );
  }
}

void QtTransferFunctionWidget::paintGL()
{
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  if ( !Core::RenderResources::Instance()->valid_render_resources() )
  {
    return;
  }
  
  Core::TextureHandle tf_texture = this->private_->transfer_function_->get_diffuse_lut();

  if ( !this->private_->bkg_tex_ || !tf_texture )
  {
    return;
  }

  // draw a window size quad and map the render texture onto it
  QSize view_size = QWidget::size();
  int width = view_size.width();
  int height = view_size.height();
  float aspect_ratio = width * 1.0f / height;
  float bkg_repeat_y = 1.0f;
  float bkg_repeat_x = bkg_repeat_y * aspect_ratio;

  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();

  Core::Texture::lock_type tf_texture_lock( tf_texture->get_mutex() );
  Core::Texture::SetActiveTextureUnit( 0 );
  this->private_->bkg_tex_->enable();
  Core::Texture::SetActiveTextureUnit( 1 );
  tf_texture->enable();

  glBegin( GL_QUADS );
  glMultiTexCoord2f( GL_TEXTURE0, 0.0f, 0.0f );
  glMultiTexCoord1f( GL_TEXTURE1, 0.0f );
  glVertex2f( 0.0f, 0.0f );
  glMultiTexCoord2f( GL_TEXTURE0, bkg_repeat_x, 0.0f );
  glMultiTexCoord1f( GL_TEXTURE1, 1.0f );
  glVertex2f( width, 0.0f );
  glMultiTexCoord2f( GL_TEXTURE0, bkg_repeat_x, bkg_repeat_y );
  glMultiTexCoord1f( GL_TEXTURE1, 1.0f );
  glVertex2f( width, height );
  glMultiTexCoord2f( GL_TEXTURE0, 0.0f, bkg_repeat_y );
  glMultiTexCoord1f( GL_TEXTURE1, 0.0f );
  glVertex2f( 0.0f, height );
  glEnd();

  tf_texture->disable();
  Core::Texture::SetActiveTextureUnit( 0 );
  this->private_->bkg_tex_->disable();
}

void QtTransferFunctionWidget::resizeGL( int width, int height )
{
  glViewport( 0, 0, width, height );
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  gluOrtho2D( 0, width, 0, height );
}

} // end namespace QtUtils

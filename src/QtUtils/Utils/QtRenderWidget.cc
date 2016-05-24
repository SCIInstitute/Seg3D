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
#include <Core/State/Actions/ActionSet.h>
#include <Core/Utils/Exception.h>
#include <Core/Utils/Log.h>

// QtUtils includes
#include <QtUtils/Utils/QtRenderWidget.h>
#include <QtUtils/Utils/QtApplication.h>

namespace QtUtils
{

typedef QPointer< QtRenderWidget > QtRenderWidgetWeakHandle;

//////////////////////////////////////////////////////////////////////////
// Class QtRenderWidgetPrivate
//////////////////////////////////////////////////////////////////////////

class QtRenderWidgetPrivate
{
public:
  void enter_size_move();
  void exit_size_move();

  Core::AbstractViewerHandle viewer_;
  Core::MouseHistory mouse_history_;
  QtRenderWidget* render_widget_;
  bool in_size_move_;
};

void QtRenderWidgetPrivate::enter_size_move()
{
  this->in_size_move_ = true;
}

void QtRenderWidgetPrivate::exit_size_move()
{
  this->in_size_move_ = false;
  this->viewer_->resize( this->render_widget_->width(), this->render_widget_->height() );
}

//////////////////////////////////////////////////////////////////////////
// Class QtRenderWidget
//////////////////////////////////////////////////////////////////////////

QtRenderWidget::QtRenderWidget( const QGLFormat& format, QWidget* parent, 
                 QGLWidget* share, Core::AbstractViewerHandle viewer ) :
  QGLWidget( format, parent, share ),
  private_( new QtRenderWidgetPrivate )
{
  this->private_->viewer_ = viewer;
  this->private_->render_widget_ = this;
  this->private_->in_size_move_ = false;

  this->setAutoFillBackground( false );
  this->setAttribute( Qt::WA_OpaquePaintEvent );
  this->setMouseTracking( true );
  this->setFocusPolicy( Qt::StrongFocus );

  this->add_connection( QtApplication::Instance()->enter_size_move_signal_.connect(
    boost::bind( &QtRenderWidgetPrivate::enter_size_move, this->private_ ) ) );
  this->add_connection( QtApplication::Instance()->exit_size_move_signal_.connect(
    boost::bind( &QtRenderWidgetPrivate::exit_size_move, this->private_ ) ) );
}

QtRenderWidget::~QtRenderWidget()
{
  this->disconnect_all();
}

static void UpdateDisplay( QtRenderWidgetWeakHandle qpointer )
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

void QtRenderWidget::initializeGL()
{
  glClearColor( 0.3f, 0.3f, 0.3f, 1.0f );

  if ( Core::RenderResources::Instance()->valid_render_resources() )
  {
    Core::Texture::SetActiveTextureUnit( 0 );
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

    QtRenderWidgetWeakHandle qpointer( this );
    this->add_connection( this->private_->viewer_->update_display_signal_.connect(
      boost::bind( &UpdateDisplay, qpointer ) ) );
  }
}

void QtRenderWidget::paintGL()
{
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  if ( !Core::RenderResources::Instance()->valid_render_resources() )
  {
    glColor3f( 1.0f, 0.5f, 0.0f );
    QString error_str( "Rendering disabled because the system doesn't have required OpenGL support." );
    this->renderText( 5, 20, error_str );
    return;
  }
  
  Core::Texture2DHandle texture = this->private_->viewer_->get_texture();
  Core::Texture2DHandle overlay_texture = this->private_->viewer_->get_overlay_texture();

  if ( !texture || !overlay_texture )
  {
    return;
  }

  Qt::CursorShape cursor = static_cast< Qt::CursorShape >(
    int( this->private_->viewer_->get_cursor() ) );
  this->setCursor( cursor );

  // draw a window size quad and map the render texture onto it
  QSize view_size = QWidget::size();
  int width = view_size.width();
  int height = view_size.height();

  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();

  Core::Texture::lock_type texture_lock( texture->get_mutex() );
  Core::Texture::lock_type overlay_texture_lock( overlay_texture->get_mutex() );

  texture->enable();
  glBegin( GL_QUADS );
  glTexCoord2f( 0.0f, 0.0f );
  glVertex2f( 0.0f, 0.0f );
  glTexCoord2f( 1.0f, 0.0f );
  glVertex2f( width, 0.0f );
  glTexCoord2f( 1.0f, 1.0f );
  glVertex2f( width, height );
  glTexCoord2f( 0.0f, 1.0f );
  glVertex2f( 0.0f, height );
  glEnd();
  texture->disable();

  // Enable blending to render the overlay texture.
  // NOTE: The overlay texture can NOT be simply rendered using multi-texturing because
  // its color channels already reflect the effect of transparency, and its alpha channel 
  // actually stores the value of "1-alpha"
  glEnable( GL_BLEND );
  glBlendFunc( GL_ONE, GL_SRC_ALPHA );
  overlay_texture->enable();
  glBegin( GL_QUADS );
  glTexCoord2f( 0.0f, 0.0f );
  glVertex2f( 0.0f, 0.0f );
  glTexCoord2f( 1.0f, 0.0f );
  glVertex2f( width, 0.0f );
  glTexCoord2f( 1.0f, 1.0f );
  glVertex2f( width, height );
  glTexCoord2f( 0.0f, 1.0f );
  glVertex2f( 0.0f, height );
  glEnd();
  overlay_texture->disable();
  glDisable( GL_BLEND );
}

void QtRenderWidget::resizeGL( int width, int height )
{
  if ( width <= 0 || height <= 0 ) return;
  
  glViewport( 0, 0, width, height );
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  gluOrtho2D( 0, width, 0, height );
  CORE_CHECK_OPENGL_ERROR();

  if ( !this->private_->in_size_move_ )
  {
    this->private_->viewer_->resize( width, height );
  }
}

void QtRenderWidget::mouseMoveEvent( QMouseEvent * event )
{
  event->accept();
  this->private_->mouse_history_.previous_ = this->private_->mouse_history_.current_;
  this->private_->mouse_history_.current_.x_ = event->x();
  this->private_->mouse_history_.current_.y_ = event->y();

  this->private_->viewer_->mouse_move_event( this->private_->mouse_history_, 
    event->button(), event->buttons(), event->modifiers() );
}

void QtRenderWidget::mousePressEvent( QMouseEvent * event )
{
  this->activate_signal_();

  this->private_->mouse_history_.current_.x_ = 
    this->private_->mouse_history_.previous_.x_ = event->x();
  this->private_->mouse_history_.current_.y_ = 
    this->private_->mouse_history_.previous_.y_ = event->y();
  
  if ( event->button() == Qt::LeftButton )
  {
    this->private_->mouse_history_.left_start_.x_ = event->x();
    this->private_->mouse_history_.left_start_.y_ = event->y();
  }
  else if ( event->button() == Qt::RightButton )
  {
    this->private_->mouse_history_.right_start_.x_ = event->x();
    this->private_->mouse_history_.right_start_.y_ = event->y();
  }
  else if ( event->button() == Qt::MidButton )
  {
    this->private_->mouse_history_.mid_start_.x_ = event->x();
    this->private_->mouse_history_.mid_start_.y_ = event->y();
  }

  this->private_->viewer_->mouse_press_event( 
    this->private_->mouse_history_, event->button(), event->buttons(),
      event->modifiers() );
}

void QtRenderWidget::saveTexture(const std::string& name, Core::Texture2DHandle texture)
{
  Core::Texture::lock_type texture_lock( texture->get_mutex() );

  texture->enable();

  int width = 0;
  int height = 0;
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

  // Build images.
  QImage image(width, height, QImage::Format_RGB32);
  GLubyte *buffer = new GLubyte[width * height * 4];

  // Grab OpenGL Image.
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

  // We extracted RGBA from OpenGL, but QImage expects ARGB.
  QRgb value;
  for (int y = 0; y < height; y++)
  {
    for (int x = 0; x < width; x++)
    {
      int offset = ((y * width) + x) * 4;
      int r = buffer[offset + 0];
      int g = buffer[offset + 1];
      int b = buffer[offset + 2];
      int a = buffer[offset + 3];

      value = qRgb(r,g,b);
      image.setPixel(x,y,value);
    }
  }

  // Delete the OpenGL buffer.
  delete []buffer;

  // Flip the image vertically. OpenGL's coordinate system starts in the bottom
  // left corner instead of the upper left corner.
  QImage mirrored = image.mirrored(false, true);

  // Save the image.
  mirrored.save(name.c_str());

  texture->disable();
}

void QtRenderWidget::saveSceneOnly(const std::string& name)
{
  Core::Texture2DHandle texture = this->private_->viewer_->get_texture();
  if (texture)
  {
    saveTexture(name, texture);
  }
}

void QtRenderWidget::saveOverlayOnly(const std::string& name)
{
  Core::Texture2DHandle overlay_texture = this->private_->viewer_->get_overlay_texture();
  if (overlay_texture)
  {
    saveTexture(name, overlay_texture);
  }
}

void QtRenderWidget::saveComposite(const std::string& name)
{
  // Force a normal rendering (compositing) of the two textures.
  glDraw();
  QImage img = grabFrameBuffer();
  img.save(name.c_str());
}

void QtRenderWidget::mouseReleaseEvent( QMouseEvent * event )
{
  this->private_->mouse_history_.previous_ = this->private_->mouse_history_.current_;
  this->private_->mouse_history_.current_.x_ = event->x();
  this->private_->mouse_history_.current_.y_ = event->y();

  this->private_->viewer_->mouse_release_event( this->private_->mouse_history_, event->button(), 
    event->buttons(), event->modifiers() );
}

void QtRenderWidget::wheelEvent( QWheelEvent* event )
{
  this->activate_signal_();

  int delta = 0;
  if ( event->delta() < 0 )
  {
    delta = -Core::RoundUp( -event->delta() / 120.0 );
  } 
  else
  {
    delta = Core::RoundUp( event->delta() / 120.0 );
  }
  if ( this->private_->viewer_->wheel_event( delta, event->x(), event->y(), 
    event->buttons(), event->modifiers() ) )
  {
    event->accept();
  }
  else
  {
    event->ignore();
  }
}

void QtRenderWidget::keyPressEvent( QKeyEvent* event )
{
  this->activate_signal_();
  
  QPoint cursor_pos = this->mapFromGlobal( QCursor::pos() );
  int x = cursor_pos.x();
  int y = cursor_pos.y();
  
  if ( this->private_->viewer_->key_press_event( event->key(), event->modifiers(), x, y ) )
  {
  }
  else
  {
    QWidget::keyPressEvent( event );
  }
}

void QtRenderWidget::keyReleaseEvent( QKeyEvent* event )
{
  this->activate_signal_();

  QPoint cursor_pos = this->mapFromGlobal( QCursor::pos() );
  int x = cursor_pos.x();
  int y = cursor_pos.y();

  if ( this->private_->viewer_->key_release_event( event->key(), event->modifiers(), x, y ) )
  {
  }
  else
  {
    QWidget::keyReleaseEvent( event );
  }
}

void QtRenderWidget::enterEvent( QEvent* event )
{
  event->accept();
  setFocus();
  QPoint cursor_pos = this->mapFromGlobal( QCursor::pos() );
  this->private_->mouse_history_.current_.x_ = cursor_pos.x();
  this->private_->mouse_history_.current_.y_ = cursor_pos.y();
  this->private_->mouse_history_.previous_ = this->private_->mouse_history_.current_;
  this->private_->viewer_->mouse_enter_event( cursor_pos.x(), cursor_pos.y() );
}

void QtRenderWidget::leaveEvent( QEvent* event )
{
  event->accept();
  clearFocus();
  this->private_->viewer_->mouse_leave_event();
}

} // end namespace QtUtils

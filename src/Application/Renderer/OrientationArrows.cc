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

#include <Core/Math/MathFunctions.h>
#include <Core/RenderResources/RenderResources.h>

#include <Application/Renderer/OrientationArrows.h>
#include <Application/Renderer/OrientationArrowsShader.h>

namespace Seg3D
{

//////////////////////////////////////////////////////////////////////////
// Class OrientationArrowsPrivate
//////////////////////////////////////////////////////////////////////////

class OrientationArrowsPrivate
{
public:
  void draw_arrow();

  GLUquadric* quadric_;
  OrientationArrowsShaderHandle shader_;
};

void OrientationArrowsPrivate::draw_arrow()
{
  gluCylinder( this->quadric_, 0.04, 0.04, 0.9, 20, 20 );
  glTranslatef( 0.0f, 0.0f, 0.9f );
  gluDisk( this->quadric_, 0.04, 0.1, 20, 1 );
  gluCylinder( this->quadric_, 0.1, 0.0, 0.2, 20, 10 );
}

//////////////////////////////////////////////////////////////////////////
// Class OrientationArrowsPrivate
//////////////////////////////////////////////////////////////////////////

OrientationArrows::OrientationArrows() :
  private_( new OrientationArrowsPrivate )
{
  {
    Core::RenderResources::lock_type lock( Core::RenderResources::GetMutex() );
    this->private_->quadric_ = gluNewQuadric();
    this->private_->shader_.reset( new OrientationArrowsShader );
    this->private_->shader_->initialize();
  }

  if ( this->private_->quadric_ == 0 )
  {
    CORE_LOG_ERROR( "Failed to create new GLU quadric object." );
  }
}

OrientationArrows::~OrientationArrows()
{
  Core::RenderResources::lock_type lock( Core::RenderResources::GetMutex() );
  if ( this->private_->quadric_ != 0 )
  {
    gluDeleteQuadric( this->private_->quadric_ );
  }
}

void OrientationArrows::draw()
{
  if ( !this->private_->quadric_ )
  {
    return;
  }

  this->private_->shader_->enable();

  glPushAttrib( GL_TRANSFORM_BIT );

  glMatrixMode( GL_MODELVIEW );

  // Draw the positive z-axis
  glColor3f( 0.0f, 0.0f, 1.0f );
  glPushMatrix();
  this->private_->draw_arrow();
  glPopMatrix();

  // Draw the positive x-axis
  glColor3f( 1.0f, 0.0, 0.0 );
  glPushMatrix();
  glRotatef( 90.0f, 0.0f, 1.0f, 0.0f );
  this->private_->draw_arrow();
  glPopMatrix();

  // Draw the positive y-axis
  glColor3f( 0.0f, 1.0f, 0.0f );
  glPushMatrix();
  glRotatef( -90.0f, 1.0f, 0.0f, 0.0f );
  this->private_->draw_arrow();
  glPopMatrix();

  float intensity = 0.3f;
  // Draw the negative x-axis
  glColor3f( intensity, 0.0f, 0.0f );
  glPushMatrix();
  glRotatef( -90.0f, 0.0f, 1.0f, 0.0f );
  this->private_->draw_arrow();
  glPopMatrix();

  // Draw the negative y-axis
  glColor3f( 0.0f, intensity, 0.0f );
  glPushMatrix();
  glRotatef( 90.0f, 1.0f, 0.0f, 0.0f );
  this->private_->draw_arrow();
  glPopMatrix();

  // Draw the negative z-axis
  glColor3f( 0.0f, 0.0f, intensity );
  glPushMatrix();
  glRotatef( 180.0f, 1.0f, 0.0f, 0.0f );
  this->private_->draw_arrow();
  glPopMatrix();

  glPopAttrib();

  this->private_->shader_->disable();
}

} // end namespace Core

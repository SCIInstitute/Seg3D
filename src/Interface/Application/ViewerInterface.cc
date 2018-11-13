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

// boost includes
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

// QT includes
#include <QSplitter>
#include <QVBoxLayout>
#include <QLabel>

// Core includes
#include <Core/Utils/Log.h>
#include <Core/Utils/Exception.h>
#include <Core/Interface/Interface.h>
#include <Core/Renderer/DummyRenderer.h>
#include <Core/RenderResources/RenderResources.h>

// QtUtils includes
#include <QtUtils/Bridge/QtBridge.h>

// Application
#include <Application/Renderer/Renderer.h>
#include <Application/ViewerManager/ViewerManager.h>

// Interface includes
#include <Interface/Application/ViewerInterface.h>
#include <Interface/Application/ViewerWidget.h>

namespace Seg3D
{

class ViewerInterfacePrivate
{
  // -- constructor/destructor --
public:
  ViewerInterfacePrivate();

  // -- Setup the UI --
public:
  void setup_ui( QWidget* parent ); 

  // -- The UI pieces of the interface --
public:
  // Add a layout that allows a small widget to be entered beneath the
  // the viewers
  QVBoxLayout* layout_;
  QLabel* facade_widget_;

  // Splitters
  // Horizontal splitter separates the left and right set of viewers
  QSplitter* horiz_splitter_;
  // The first vertical splitter is for the viewers on the left and
  // the second one is for the viewers on the right
  QSplitter* vert_splitter1_;
  QSplitter* vert_splitter2_;

  // List of all the viewers
  std::vector< ViewerWidget* > viewer_;
};

ViewerInterfacePrivate::ViewerInterfacePrivate() :
  layout_( 0 ),
  horiz_splitter_( 0 ),
  vert_splitter1_( 0 ),
  vert_splitter2_( 0 )
{
}


void ViewerInterfacePrivate::setup_ui( QWidget* parent ) 
{
  this->layout_ = new QVBoxLayout( parent );
  this->layout_->setContentsMargins( 0, 0, 0, 0 );
  this->layout_->setSpacing( 0 );

  this->vert_splitter1_ = new QSplitter( Qt::Vertical, parent );
  this->vert_splitter2_ = new QSplitter( Qt::Vertical, parent );
  this->horiz_splitter_ = new QSplitter( Qt::Horizontal, parent );

  this->horiz_splitter_->addWidget( this->vert_splitter1_ );
  this->horiz_splitter_->addWidget( this->vert_splitter2_ );
  this->horiz_splitter_->setOpaqueResize( false );

  this->viewer_.resize( 6 );
  for ( size_t j = 0; j < 6; j++ )
  {
    // Step 1: Get the viewer class that maintains the state of the viewer
    ViewerHandle viewer = ViewerManager::Instance()->get_viewer( j );
    // Step 2: Generate a renderer for the viewer
    // Only create renderer if the render resources are valid.
    if ( Core::RenderResources::Instance()->valid_render_resources() )
    {
      RendererHandle renderer( new Renderer( j ) );
      renderer->initialize();
      viewer->install_renderer( renderer );
    }
    else
    {
      viewer->install_renderer( Core::AbstractRendererHandle( 
        new Core::DummyRenderer ) );
    }
    // Step 3: Generate the widget
    this->viewer_[ j ] = new ViewerWidget( viewer, parent );
    QtUtils::QtBridge::Show( this->viewer_[ j ], viewer->viewer_visible_state_ );
  }
  
  this->vert_splitter1_->addWidget( this->viewer_[ 0 ] );
  this->vert_splitter1_->addWidget( this->viewer_[ 1 ] );
  this->vert_splitter1_->addWidget( this->viewer_[ 2 ] );
  vert_splitter1_->setOpaqueResize( false );

  this->vert_splitter2_->addWidget( this->viewer_[ 3 ] );
  this->vert_splitter2_->addWidget( this->viewer_[ 4 ] );
  this->vert_splitter2_->addWidget( this->viewer_[ 5 ] );
  vert_splitter2_->setOpaqueResize( false );

  this->layout_->addWidget( this->horiz_splitter_ );
  
  
  //this->facade_widget_ = new QLabel( parent );
//  this->layout_->addWidget( this->facade_widget_ );
//  this->facade_widget_->hide();
  
  parent->setLayout( this->layout_ );

}

ViewerInterface::ViewerInterface( QWidget *parent ) :
  QWidget( parent ),
  private_( new ViewerInterfacePrivate )
{
  // Create the internals of the interface
  this->private_->setup_ui( this );

  // We need a lock here as connecting to the state engine and getting the
  // the current value needs to be atomic
  {
    Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

    qpointer_type qpointer( this );
    
    // Connect signals
    ViewerManager::Instance()->layout_state_->value_changed_signal_.connect( 
      boost::bind( &ViewerInterface::SetViewerLayout, qpointer, _1 ) );
      
    ViewerManager::Instance()->active_viewer_state_->value_changed_signal_.connect(
        boost::bind( &ViewerInterface::SetActiveViewer, qpointer, _1 ) );

    // set the default state
    set_layout( ViewerManager::Instance()->layout_state_->get() );
    set_active_viewer( ViewerManager::Instance()->active_viewer_state_->get() );
  }
}

ViewerInterface::~ViewerInterface()
{
}
  
void ViewerInterface::set_pic_mode( bool pic_mode )
{
  for ( size_t j = 0; j < 6; j++ )
  {
    this->private_->viewer_[ j ]->image_mode( pic_mode );
  }
}

void ViewerInterface::set_active_viewer( int viewer_id )
{
  std::for_each( this->private_->viewer_.begin(), this->private_->viewer_.end(),
    boost::lambda::bind( &ViewerWidget::deselect, boost::lambda::_1 ) );
  this->private_->viewer_[ viewer_id ]->select();
}

void ViewerInterface::set_layout( const std::string& layout )
{
  if( layout == ViewerManager::VIEW_SINGLE_C )
  {
    private_->vert_splitter1_->show();
    private_->vert_splitter2_->hide();

    QList< int > sizes;
    sizes.push_back( 1000 );
    sizes.push_back( 0 );
    private_->horiz_splitter_->setSizes( sizes );
    private_->horiz_splitter_->repaint();
  }
  else if( layout == ViewerManager::VIEW_1AND1_C )
  {
    private_->vert_splitter1_->show();
    private_->vert_splitter2_->show();

    QList< int > sizes;
    sizes.push_back( 1000 );
    sizes.push_back( 1000 );
    private_->horiz_splitter_->setSizes( sizes );
    private_->horiz_splitter_->repaint();
  }
  else if( layout == ViewerManager::VIEW_1AND2_C )
  {
    private_->vert_splitter1_->show();
    private_->vert_splitter2_->show();

    QList< int > sizes;
    sizes.push_back( 3000 );
    sizes.push_back( 1000 );
    private_->horiz_splitter_->setSizes( sizes );

    QList< int > vsizes;
    vsizes.push_back( 1000 );
    vsizes.push_back( 1000 );
    vsizes.push_back( 0 );
    private_->vert_splitter2_->setSizes( vsizes );
    private_->horiz_splitter_->repaint();
  }
  else if( layout == ViewerManager::VIEW_1AND3_C )
  {
    private_->vert_splitter1_->show();
    private_->vert_splitter2_->show();

    QList< int > sizes;
    sizes.push_back( 3000 );
    sizes.push_back( 1000 );
    private_->horiz_splitter_->setSizes( sizes );

    QList< int > vsizes;
    vsizes.push_back( 1000 );
    vsizes.push_back( 1000 );
    vsizes.push_back( 1000 );
    private_->vert_splitter2_->setSizes( vsizes );
    private_->horiz_splitter_->repaint();
  }
  else if( layout == ViewerManager::VIEW_2AND2_C )
  {
    private_->vert_splitter1_->show();
    private_->vert_splitter2_->show();

    QList< int > sizes;
    sizes.push_back( 1000 );
    sizes.push_back( 1000 );
    private_->horiz_splitter_->setSizes( sizes );

    QList< int > vsizes;
    vsizes.push_back( 1000 );
    vsizes.push_back( 1000 );
    vsizes.push_back( 0 );
    private_->vert_splitter1_->setSizes( vsizes );
    private_->vert_splitter2_->setSizes( vsizes );
    private_->horiz_splitter_->repaint();
  }
  else if( layout == ViewerManager::VIEW_2AND3_C )
  {
    private_->vert_splitter1_->show();
    private_->vert_splitter2_->show();

    QList< int > sizes;
    sizes.push_back( 1000 );
    sizes.push_back( 1000 );
    private_->horiz_splitter_->setSizes( sizes );

    QList< int > vsizes;
    vsizes.push_back( 1000 );
    vsizes.push_back( 1000 );
    vsizes.push_back( 0 );
    private_->vert_splitter1_->setSizes( vsizes );
    vsizes.last() = 1000;
    private_->vert_splitter2_->setSizes( vsizes );
    private_->horiz_splitter_->repaint();
  }
  else if( layout == ViewerManager::VIEW_3AND3_C )
  {
    private_->vert_splitter1_->show();
    private_->vert_splitter2_->show();

    QList< int > sizes;
    sizes.push_back( 1000 );
    sizes.push_back( 1000 );
    private_->horiz_splitter_->setSizes( sizes );

    QList< int > vsizes;
    vsizes.push_back( 1000 );
    vsizes.push_back( 1000 );
    vsizes.push_back( 1000 );
    private_->vert_splitter1_->setSizes( vsizes );
    private_->vert_splitter2_->setSizes( vsizes );
    private_->horiz_splitter_->repaint();
  }
}

void ViewerInterface::SetViewerLayout( qpointer_type qpointer, std::string layout )
{
  if( !( Core::Interface::IsInterfaceThread() ) )
  {
    Core::Interface::Instance()->post_event( boost::bind( &ViewerInterface::SetViewerLayout,
        qpointer, layout ) );
    return;
  }

  if( qpointer.data() ) qpointer->set_layout( layout );
}

void ViewerInterface::SetActiveViewer( qpointer_type qpointer, int active_viewer )
{
  if( !( Core::Interface::IsInterfaceThread() ) )
  {
    Core::Interface::Instance()->post_event( boost::bind( &ViewerInterface::SetActiveViewer,
        qpointer, active_viewer ) );
    return;
  }

  if( qpointer.data() ) qpointer->set_active_viewer( active_viewer );
}

} // end namespace Seg3D

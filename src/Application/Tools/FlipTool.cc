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

#include <Application/Filters/Actions/ActionPermute.h>
#include <Application/Tool/ToolFactory.h>
#include <Application/Tools/FlipTool.h>

// Register the tool into the tool factory
SCI_REGISTER_TOOL( Seg3D, FlipTool)

namespace Seg3D
{

FlipTool::FlipTool( const std::string& toolid ) :
  GroupTargetTool( Core::VolumeType::ALL_REGULAR_E, toolid )
{
  this->add_state( "replace", this->replace_state_, false );
}

FlipTool::~FlipTool()
{
  this->disconnect_all();
}

void FlipTool::dispatch_flip( Core::ActionContextHandle context, int index )
{
  std::vector< int > permutation( 3 );
  for ( int i = 0; i < 3; ++i )
  {
    permutation[ i ] = i + 1;
  }
  permutation[ index ] *= -1;

  this->dispatch( context, permutation );
}

void FlipTool::dispatch_rotate( Core::ActionContextHandle context, int axis, bool ccw )
{
  std::vector< int > permutation( 3 );
  permutation[ axis ] = axis + 1;
  
  // NOTE: Since coronal view is already rotated, the definition of counterclockwise
  // needs to be inverted.
  if ( axis == 1 )
  {
    ccw = !ccw;
  }
  
  int sign1 = ccw ? -1 : 1;
  int sign2 = ccw ? 1 : -1;

  permutation[ ( axis + 1 ) % 3 ] = sign1 * ( ( axis + 2 ) % 3 + 1 );
  permutation[ ( axis + 2 ) % 3 ] = sign2 * ( ( axis + 1 ) % 3 + 1 );

  this->dispatch( context, permutation );
}

void FlipTool::dispatch( Core::ActionContextHandle context, const std::vector< int >& permutation )
{
  // NOTE: Need to lock state engine as this function is run from the interface thread
  Core::StateEngine::lock_type lock( Core::StateEngine::GetMutex() );

  // Reverse the order of target layers
  std::vector< std::string > target_layers = this->target_layers_state_->get();
  std::reverse( target_layers.begin(), target_layers.end() );

  ActionPermute::Dispatch( context, target_layers, permutation, this->replace_state_->get() );
}


} // end namespace Seg3D

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

#include <Application/ViewerManager/Actions/ActionDeleteFeature.h>
#include <Application/ViewerManager/ViewerManager.h>

CORE_REGISTER_ACTION( Seg3D, DeleteFeature )

namespace Seg3D
{

ActionDeleteFeature::ActionDeleteFeature()
{
  this->add_parameter( this->feature_id_ );
}

ActionDeleteFeature::~ActionDeleteFeature()
{
}

bool ActionDeleteFeature::validate( Core::ActionContextHandle& context )
{
  return true;
}

bool ActionDeleteFeature::run( Core::ActionContextHandle& context,
              Core::ActionResultHandle& result )
{
  ViewerManager::Instance()->delete_feature( this->feature_id_ );
  return true;
}

void ActionDeleteFeature::Dispatch( Core::ActionContextHandle context,
                   const std::string& feature_id )
{
  ActionDeleteFeature* action = new ActionDeleteFeature;
  action->feature_id_ = feature_id;
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

} // end namespace Seg3D

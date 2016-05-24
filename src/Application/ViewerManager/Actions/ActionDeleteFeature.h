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

#ifndef APPLICATION_VIEWERMANAGER_ACTIONS_ACTIONDELETEFEATURE_H
#define APPLICATION_VIEWERMANAGER_ACTIONS_ACTIONDELETEFEATURE_H

#include <Core/Action/Action.h>

namespace Seg3D
{

class ActionDeleteFeature : public Core::Action
{

CORE_ACTION(
  CORE_ACTION_TYPE( "DeleteFeature", "Delete a transfer function feature." )
  CORE_ACTION_ARGUMENT( "feature_id", "The string ID of the feature to be deleted." )
  CORE_ACTION_CHANGES_PROJECT_DATA()
)

public:
  ActionDeleteFeature();
  virtual ~ActionDeleteFeature();

  virtual bool validate( Core::ActionContextHandle& context );
  virtual bool run( Core::ActionContextHandle& context, Core::ActionResultHandle& result );

private:
  std::string feature_id_;

public:
  static void Dispatch( Core::ActionContextHandle context, const std::string& feature_id );
};

} // end namespace Seg3D

#endif

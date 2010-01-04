/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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

/*
 *****************************************************************************
 *
 *   ActionLayer.h
 *
 *   Base action for modifying layers.  Takes care of locking and unlocking
 *   resources.  Implements a run method that calls each subclass's overridden
 *   execute method.
 *
 *   Also see: Layer, LayerManager
 *
 *   Authors:
 *      Kristen Zygmunt   -- initial attempt      11/24/2009
 *
 *    
 *****************************************************************************
 */

#ifndef APPLICATION_LAYER_ACTIONS_ACTIONLAYER_H
#define APPLICATION_LAYER_ACTIONS_ACTIONLAYER_H 1

#include <Application/Action/Action.h>

namespace Seg3D {

class ActionLayer : public Action 
{
    SCI_ACTION_TYPE("Layer","",LAYER_E)

  public:
    virtual ~ActionLayer() {}
    
    virtual bool run(ActionContextHandle& context, ActionResultHandle& result);

  // TODO could have base validate call check_layer_availability and then call subclass's
  // do_validate method.
    virtual bool validate(ActionContextHandle& context);
    virtual bool do_validate(ActionContextHandle& context); // cannot make pure virtual and register the action
    virtual bool check_layer_availability(); // cannot make pure virtual and register the action
    virtual bool lock_layers() const; // cannot make pure virtual and register the action
    virtual bool release_layers() const; // cannot make pure virtual and register the action
    virtual bool execute(ActionContextHandle& context) const; // cannot make pure virtual and register the action

  private:

};

  typedef boost::intrusive_ptr<ActionLayer> ActionLayerHandle;

} // end namespace Seg3D

#endif

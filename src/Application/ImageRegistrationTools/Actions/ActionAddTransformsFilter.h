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

#ifndef APPLICATION_IMAGEREGISTRATIONTOOLS_ACTIONS_ACTIONADDTRANSFORMSFILTER_H
#define APPLICATION_IMAGEREGISTRATIONTOOLS_ACTIONS_ACTIONADDTRANSFORMSFILTER_H

#include <Core/Action/Actions.h>
#include <Core/Interface/Interface.h>

#include <Application/Layer/Layer.h>
#include <Application/Layer/LayerAction.h>
#include <Application/Layer/LayerManager.h>

namespace Seg3D
{

class ActionAddTransformsFilter : public LayerAction
{

CORE_ACTION(
  CORE_ACTION_TYPE( "AddTransformsFilter", "ir-add-transforms" )
  CORE_ACTION_ARGUMENT( "layerid", "The layerid on which this filter needs to be run." )
  CORE_ACTION_ARGUMENT( "input_files", "Stos data file names." )
  CORE_ACTION_ARGUMENT( "output_prefixes", "Output file prefix(es) (exactly one, or one for each slice)." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "slice_dirs", "[]", "Optional slice directory(ies)." )
  CORE_ACTION_OPTIONAL_ARGUMENT( "sandbox", "-1", "The sandbox in which to run the action." )
  CORE_ACTION_ARGUMENT_IS_NONPERSISTENT( "sandbox" )
)
  
public:
  ActionAddTransformsFilter()
  {
    this->add_layer_id( this->target_layer_ );
    this->add_parameter( this->input_files_ );
    this->add_parameter( this->output_prefixes_ );
    this->add_parameter( this->slice_dirs_ );
    this->add_parameter( this->sandbox_ );
  }
  
  virtual bool validate( Core::ActionContextHandle& context );
  virtual bool run( Core::ActionContextHandle& context, Core::ActionResultHandle& result );
  
  // DISPATCH:
  // Create and dispatch action that inserts the new layer
  static void Dispatch(Core::ActionContextHandle context,
                       std::string target_layer,
                       std::vector<std::string> input_files,
                       std::vector<std::string> output_prefixes,
                       std::vector<std::string> slice_dirs);
  
private:
  std::string target_layer_;
  SandboxID sandbox_;

  std::vector<std::string> input_files_;
  std::vector<std::string> output_prefixes_;
  std::vector<std::string> slice_dirs_;
};

}

#endif

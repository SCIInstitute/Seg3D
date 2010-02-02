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

#include <Application/Tool/ToolFactory.h>
#include <Application/Tools/PolylineTool.h>
// #include <Application/Layer/LayerManager.h>

namespace Seg3D {

// Register the tool into the tool factory
SCI_REGISTER_TOOL(PolylineTool)

PolylineTool::PolylineTool(const std::string& toolid) :
  Tool(toolid)
{
  // Need to set ranges and default values for all parameters
  add_state("target",target_layer_,"<none>","<none>");
  
  // No constraints are needed for this tool
  
  // If a layer is added or deleted update the lists
//  add_connection(LayerManager::instance()->connect_layers_changed(
//    boost:bind(&PaintTool::handle_layers_changed,this)));
  
  // Trigger a fresh update
  handle_layers_changed();
}

PolylineTool::~PolylineTool()
{
  disconnect_all();
}

void
PolylineTool::handle_layers_changed()
{
/*
  std::vector<std::string> target_layers;
  LayerManager::instance()->get_layers(LayerManager::DATALAYER_E|
                                       LayerManager::MASKLAYER_E|
                                       LayerManager::ACTIVE_E|
                                       LayerManager::NONE_E,
                                       target_layers );
                                       
  target_layer_->set_option_list(target_layers);

*/
}

void
PolylineTool::activate()
{
}

void
PolylineTool::deactivate()
{
}

void 
PolylineTool::dispatch_fill_within_polyline() const
{
}

void 
PolylineTool::dispatch_erase_within_polyline() const
{
}  

void 
PolylineTool::dispatch_reset_polyline() const
{
}

} // end namespace Seg3D

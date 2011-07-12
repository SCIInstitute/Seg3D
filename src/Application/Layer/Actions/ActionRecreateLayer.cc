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

#ifdef _MSC_VER
#pragma warning( disable: 4244 )
#endif

// boost includes
#include <boost/regex.hpp>

// Core includes
#include <Core/Utils/Exception.h>
#include <Core/Utils/StringParser.h>
#include <Core/Python/PythonInterpreter.h>

// Application includes
#include <Application/Clipboard/Clipboard.h>
#include <Application/Layer/LayerManager.h>
#include <Application/Layer/LayerRecreationUndoBufferItem.h>
#include <Application/Layer/Actions/ActionRecreateLayer.h>
#include <Application/ProjectManager/ProjectManager.h>
#include <Application/UndoBuffer/UndoBuffer.h>

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
CORE_REGISTER_ACTION( Seg3D, RecreateLayer )

namespace Seg3D
{

class ActionRecreateLayerPrivate
{
public:
  // -- Action Parameters --
  ProvenanceID prov_id_;

  // -- Internal variables --
  ProvenanceTrailHandle prov_trail_;
};

ActionRecreateLayer::ActionRecreateLayer() :
  private_( new ActionRecreateLayerPrivate )
{
  this->add_parameter( this->private_->prov_id_ );
}

ActionRecreateLayer::~ActionRecreateLayer()
{
}

bool ActionRecreateLayer::validate( Core::ActionContextHandle& context )
{
  // Make sure there is no running recreation
  if ( LayerManager::Instance()->is_sandbox( 0 ) )
  {
    context->report_error( "A layer recreation process is already running." );
    return false;
  }

  // Get the provenance trail that leads to the desired provenance ID
  ProvenanceTrailHandle prov_trail = ProjectManager::Instance()->get_current_project()->
    get_provenance_trail( this->private_->prov_id_ );
  if ( !prov_trail || prov_trail->size() == 0 )
  {
    context->report_error( "Provenance trail for provenance ID " + 
      Core::ExportToString( this->private_->prov_id_ ) + " doesn't exist." );
    return false;
  }

  this->private_->prov_trail_.reset( new ProvenanceTrail );

  // The provenance ID already exists
  if ( LayerManager::FindLayer( this->private_->prov_id_ ) ) return true;

  // For every entry in the provenance trail, if all of its outputs already exist, ignore,
  // otherwise, put it in the private prov_trail_ structure.
  for ( size_t i = 0; i < prov_trail->size(); ++i )
  {
    ProvenanceStepHandle prov_step = prov_trail->at( i );
    const ProvenanceIDList& output_prov_ids = prov_step->get_output_provenance_ids();
    bool skippable = true;
    for ( size_t j = 0; j < output_prov_ids.size(); ++j )
    {
      if ( !LayerManager::FindLayer( output_prov_ids[ j ] ) )
      {
        skippable = false;
        break;
      }
    }
    if ( !skippable )
    {
      this->private_->prov_trail_->push_back( prov_step );
    }
  }
  
  return true; // validated
}

static std::string ProvenanceInputFormatter( boost::smatch what, 
      const std::map< ProvenanceID, std::string >& prov_id_map,
      const ProvenanceIDList& prov_input_ids )
{
  std::string match = what.str();
  size_t input_index;
  std::map< ProvenanceID, std::string >::const_iterator it;
  if ( !Core::ImportFromString( match.substr( 2, match.size() - 3 ), input_index ) ||
    input_index >= prov_input_ids.size() || 
    ( it = prov_id_map.find( prov_input_ids[ input_index ] ) ) == prov_id_map.end() )
  {
    CORE_THROW_EXCEPTION( "Invalid provenance record." );
  }
  return it->second;
}

bool ActionRecreateLayer::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{ 
  size_t num_steps = this->private_->prov_trail_->size();
  // If the provenance trail required to recreate the layer is empty,
  // the layer should already exists in the layer manager.
  if ( num_steps == 0 )
  {
    LayerHandle layer = LayerManager::FindLayer( this->private_->prov_id_ );
    LayerManager::Instance()->set_active_layer( layer );
    result.reset( new Core::ActionResult( layer->get_layer_id() ) );
    return true;
  }

  // Get the current counters for groups and layers, so we can undo the changes to those counters
  // NOTE: This needs to be done before a new layer is created
  LayerManager::id_count_type id_count = LayerManager::GetLayerIdCount();

  // Build a lookup table for all the provenance IDs involved in the provenance trail.
  // All the input IDs of a provenance step must either already exist in the layer manager, 
  // or are outputs from previous steps.
  std::map< ProvenanceID, std::string > prov_id_map;
  std::vector< LayerHandle > input_layers;
  for ( size_t i = 0; i < num_steps; ++i )
  {
    ProvenanceStepHandle prov_step = this->private_->prov_trail_->at( i );
    const ProvenanceIDList& input_ids = prov_step->get_input_provenance_ids();
    const ProvenanceIDList& output_ids = prov_step->get_output_provenance_ids();
    for ( size_t j = 0; j < input_ids.size(); ++j )
    {
      if ( prov_id_map.find( input_ids[ j ] ) == prov_id_map.end() )
      {
        LayerHandle layer = LayerManager::FindLayer( input_ids[ j ] );
        if ( layer )
        {
          LayerHandle duplicate_layer = layer->duplicate();
          duplicate_layer->provenance_id_state_->set( layer->provenance_id_state_->get() );
          input_layers.push_back( duplicate_layer );
          prov_id_map[ input_ids[ j ] ] = "'" + duplicate_layer->get_layer_id() + "'";
        }
        else
        {
          context->report_error( "The provenance trail for provenance ID " + 
            Core::ExportToString( this->private_->prov_id_ ) + " is incomplete." );
          return false;
        }
      }
    }

    for ( size_t j = 0; j < output_ids.size(); ++j )
    {
      prov_id_map[ output_ids[ j ] ] = "output" + Core::ExportToString( i ) + "[" +
        Core::ExportToString( j ) + "]";
    }
  }

  // Create a sandbox for running the script
  // NOTE: sandbox must be created in both the layer manager and the clipboard
  // NOTE: provenance playback always uses sandbox 0 to guarantee that only one
  // playback will be running at a time.
  SandboxID sandbox = 0;
  LayerManager::Instance()->create_sandbox( sandbox );
  Clipboard::Instance()->create_sandbox( sandbox );
  std::string sandbox_str = Core::ExportToString( sandbox );

  // Add the input layers to the sandbox
  for ( size_t i = 0; i < input_layers.size(); ++i )
  {
    LayerManager::Instance()->insert_layer( input_layers[ i ], sandbox );
  }
  
  // == Generate the script ==

  // Regular expression that matches input provenances in the action params string
  boost::regex input_regex( "\\$\\{[0-9]+\\}" );
  // Regular expression that matches a word in the action name
  boost::regex action_name_regex( "[[:upper:]]?[[:lower:]]*" );

  // Generate python commands to run the necessary actions
  bool succeeded = true;
  std::string script( "try:\n" );
  script += "\tbeginscriptstatusreport(sandbox=" + sandbox_str + 
    ", script_name='[Provenance Playback]')\n";
  for ( size_t i = 0 ; i < num_steps && succeeded; ++i )
  {
    ProvenanceStepHandle prov_step = this->private_->prov_trail_->at( i );
    const ProvenanceIDList& input_ids = prov_step->get_input_provenance_ids();
    const std::string& action_params = prov_step->get_action_params();
    // Convert the action name to human readable format
    std::string action_display_name = boost::regex_replace( prov_step->get_action_name(),
      action_name_regex, "$& " );
    std::string output_name = "output" + Core::ExportToString( i );
    // Report script progress
    script += "\treportscriptstatus(sandbox=" + sandbox_str + ", current_step='[" + 
      action_display_name + "]', steps_done=" + Core::ExportToString( i ) +
      ", total_steps=" + Core::ExportToString( num_steps ) + ")\n";

    script += "\t" + output_name + "=" + Core::StringToLower( prov_step->get_action_name() ) + "(";
    std::string key, value, error;
    std::string::size_type start = 0;
    
    // The functor for replacing input strings with correct input values
    // NOTE: boost.regex library couldn't deduce the correct functor type from the boost::bind
    // result, so we need to explicitly tell it
    boost::function< std::string ( boost::smatch ) > formatter = boost::bind( 
      &ProvenanceInputFormatter, _1, prov_id_map, input_ids );

    while ( true ) 
    {
      if ( !Core::ScanKeyValuePair( action_params, start, key, value, error ) )
      {
        context->report_error( "Syntax error in provenance string: " + error );
        succeeded = false;
        break;
      }
      if ( key.empty() ) break;

      std::string formatted_val;
      try
      {
        formatted_val = boost::regex_replace( value, input_regex, formatter, 
          boost::regex_constants::format_literal );
      }
      catch ( ... )
      {
        context->report_error( "Incorrectly constructed provenance record encountered." );
        succeeded = false;
        break;
      }

      if ( formatted_val != value )
      {
        script += key + "=" + formatted_val + ",";
      }
      else
      {
        script += key + "='" + Core::PythonInterpreter::EscapeSingleQuotedString( value ) + "',";
      }
    }
    script += "sandbox=" + sandbox_str + ")\n";

    // sync on the output
    script += "\tsynchronize(layerids=" + output_name + ", sandbox=" + sandbox_str + ")\n";

    // Make sure that the output is a python list
    script += "\tif type(" + output_name + ")!=list:\n" 
      "\t\ttmp=list()\n" + "\t\ttmp.append(" + output_name + ")\n"
      "\t\t" + output_name + "=tmp\n";
    // Print the output
    script += "\tprint('" + output_name + " =', " + output_name + ")\n";
  }

  // Error happened, destroy the sandbox and return
  if ( !succeeded )
  {
    LayerManager::Instance()->delete_sandbox( sandbox );
    Clipboard::Instance()->delete_sandbox( sandbox );
    return false;
  }

  // Report script progress
  script += "\treportscriptstatus(sandbox=" + sandbox_str + ", current_step='', steps_done=" +
    Core::ExportToString( num_steps ) + ", total_steps=" + Core::ExportToString( num_steps ) + ")\n";

  // Get the layer ID of the desired output
  std::string layer_id = prov_id_map[ this->private_->prov_id_ ];
  // Set the layer name
  std::string layer_name = "Recreated_Provenance_ID_" + 
    Core::ExportToString( this->private_->prov_id_ );
  script += "\tset(stateid=" + layer_id + "+'::name', value='" +
    layer_name + "')\n";
  // Move the final output layer out of the sandbox and assign the 
  // desired provenance ID to it.
  script += "\tmigratesandboxlayer(layerid=" + layer_id +
    ", sandbox=" + sandbox_str + ", prov_id=" +
    Core::ExportToString( this->private_->prov_id_ ) + ")\n";
  // Activate the layer
  script += "\tactivatelayer(layerid=" + layer_id + ")\n";

  // Exception handling
  script += "except:\n\tprint('Layer recreation failed:', sys.exc_info()[:2], file=sys.stderr)\n";
  // Clean up, ignore any exceptions that might happen
  script += "try:\n\tendscriptstatusreport(sandbox=" + sandbox_str +
    ")\nexcept:\n\tpass\n";
  script += "try:\n\tdeletesandbox(sandbox=" + sandbox_str + 
    ")\nexcept:\n\tpass\n";

  // Create undo/redo record
  LayerRecreationUndoBufferItemHandle item( new LayerRecreationUndoBufferItem( 
    this->private_->prov_id_, sandbox ) );
  item->set_redo_action( this->shared_from_this() );
  item->add_id_count_to_restore( id_count );
  UndoBuffer::Instance()->insert_undo_item( context, item );
  
  // Run the script to recreate the layer
  Core::PythonInterpreter::Instance()->run_script( script );
  
  return true;
}

void ActionRecreateLayer::Dispatch( Core::ActionContextHandle context, ProvenanceID prov_id )
{
  ActionRecreateLayer* action = new ActionRecreateLayer;
  action->private_->prov_id_ = prov_id;
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

} // end namespace Seg3D

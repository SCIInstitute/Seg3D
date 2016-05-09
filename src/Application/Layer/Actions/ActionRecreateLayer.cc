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

#ifdef _MSC_VER
#pragma warning( disable: 4244 )
#endif

// boost includes
#include <boost/regex.hpp>
#include <boost/timer.hpp>
#include <boost/unordered_map.hpp>

// Core includes
#include <Core/Utils/Exception.h>
#include <Core/Utils/StringParser.h>
#include <Core/Utils/StringContainer.h>
#ifdef BUILD_WITH_PYTHON
#include <Core/Python/PythonInterpreter.h>
#endif

// Application includes
#include <Application/Clipboard/Clipboard.h>
#include <Application/Layer/LayerManager.h>
#include <Application/Layer/LayerRecreationUndoBufferItem.h>
#include <Application/Layer/Actions/ActionRecreateLayer.h>
#include <Application/ProjectManager/ProjectManager.h>
#include <Application/UndoBuffer/UndoBuffer.h>
#include "Core/Utils/Log.h"

// REGISTER ACTION:
// Define a function that registers the action. The action also needs to be
// registered in the CMake file.
CORE_REGISTER_ACTION( Seg3D, RecreateLayer )

namespace Seg3D
{

class ActionRecreateLayerPrivate
{
public:
  // GENERATE_SCRIPT:
  // Template function for generating a python script from the provenance trail.
  template< class LAYER_LUT_TYPE, class PROV_USE_LUT_TYPE >
  bool generate_script( Core::ActionContextHandle context, 
    LAYER_LUT_TYPE& prov_input_lut, PROV_USE_LUT_TYPE& prov_use_lut, 
    std::vector< LayerHandle >& input_layers, std::vector< std::string >& script );

  // -- Action Parameters --
  ProvenanceIDList prov_ids_;

  // -- Internal variables --
  ProvenanceTrailHandle prov_trail_;
};

template< class LAYER_LUT_TYPE, class PROV_USE_LUT_TYPE >
bool ActionRecreateLayerPrivate::generate_script( Core::ActionContextHandle context,
          LAYER_LUT_TYPE& layer_lut, PROV_USE_LUT_TYPE& prov_use_lut,
          std::vector< LayerHandle >& input_layers, std::vector< std::string >& script )
{
  const size_t num_steps = this->prov_trail_->size();
  const std::string num_steps_str = Core::ExportToString( num_steps );

  boost::timer performance_timer;

  // == Build lookup tables for provenance IDs ==
  // All the input IDs of a provenance step must either already exist in the layer manager, 
  // or are outputs from previous steps.
  for ( size_t i = 0; i < num_steps; ++i )
  {
    ProvenanceStepHandle prov_step = this->prov_trail_->at( i );
    const ProvenanceIDList& input_ids = prov_step->get_input_provenance_ids();
    const ProvenanceIDList& output_ids = prov_step->get_output_provenance_ids();
    for ( size_t j = 0; j < input_ids.size(); ++j )
    {
      if ( layer_lut[ input_ids[ j ] ].empty() )
      {
        LayerHandle layer = LayerManager::FindLayer( input_ids[ j ] );
        if ( layer )
        {
          LayerHandle duplicate_layer = layer->duplicate();
          duplicate_layer->provenance_id_state_->set( layer->provenance_id_state_->get() );
          input_layers.push_back( duplicate_layer );
          layer_lut[ input_ids[ j ] ] = "'" + duplicate_layer->get_layer_id() + "'";
        }
        else
        {
          context->report_error( "The provenance trail for provenance IDs " + 
            Core::ExportToString( this->prov_ids_ ) + " is incomplete." );
          return false;
        }
      }
      prov_use_lut[ input_ids[ j ] ] = i;
    }

    for ( size_t j = 0; j < output_ids.size(); ++j )
    {
      layer_lut[ output_ids[ j ] ] = "output" + Core::ExportToString( i ) + "[" +
        Core::ExportToString( j ) + "]";
    }
  }

  double elapsed_time = performance_timer.elapsed();
  CORE_LOG_MESSAGE( "Time spent on building lookup tables: " + Core::ExportToString( elapsed_time ) );
  performance_timer.restart();

  // == Generate the script ==

  // Regular expression that matches input provenances in the action params string
  const static boost::regex input_regex( "\\$\\{[0-9]+\\}" );
  // Regular expression that matches a word in the action name
  const static boost::regex action_name_regex( "[[:upper:]]?[[:lower:]]*" );

  // Reserve roughly enough memory for the script
  script.reserve( 5 * num_steps + 10 + 3 * this->prov_ids_.size() );

  // Generate python commands to run the necessary actions
  script.push_back( "try:\n" );
  script.push_back( "\tbeginscriptstatusreport(sandbox=0, "
    "script_name='[Provenance Playback]')\n" );
  for ( size_t i = 0 ; i < num_steps; ++i )
  {
    const std::string i_str = Core::ExportToString( i );
    ProvenanceStepHandle prov_step = this->prov_trail_->at( i );
    const ProvenanceIDList& input_ids = prov_step->get_input_provenance_ids();
    const std::string& action_params = prov_step->get_action_params();
    // Convert the action name to human readable format
    std::string action_display_name = boost::regex_replace( prov_step->get_action_name(),
      action_name_regex, "$& " );
    std::string output_name = "output" + i_str;
    // Report script progress
    script.push_back( "\treportscriptstatus(sandbox=0, current_step='[" + 
      action_display_name + "]', steps_done=" + i_str +
      ", total_steps=" + num_steps_str + ")\n" );

    // == Build the command for running the action ==
    std::string action_cmd = "\t" + output_name + "=" + Core::StringToLower( prov_step->get_action_name() ) + "(";
    // Look for provenance inputs and replace them with proper layer IDs
    std::string::size_type start_pos = 0;
    for ( size_t j = 0; j < input_ids.size(); ++j )
    {
      std::string layer_id = layer_lut[ input_ids[ j ] ];
      if ( layer_id.empty() )
      {
        context->report_error( "The provenance trail is incomplete." );
        return false;
      }
      std::string input_pattern = "${" + Core::ExportToString( j ) + "}";
      std::string::size_type pos = action_params.find( input_pattern, start_pos );
      if ( pos == std::string::npos )
      {
        // The input is used implicitly by the filter (most likely an ActionPaste)
        continue;
      }
      action_cmd += action_params.substr( start_pos, pos - start_pos ) + layer_id;
      start_pos = pos + input_pattern.size();
    }
    // Append the rest of the parameter string to the command
    if ( start_pos < action_params.size() )
    {
      action_cmd += action_params.substr( start_pos );
    }
    // Append the sandbox parameter
    action_cmd += ", sandbox=0)\n";

    // Before running the action, duplicate any inputs that will be replaced by new data but
    // the old data is needed but following steps.
    const ProvenanceIDList& replaced_ids = prov_step->get_replaced_provenance_ids();
    for ( size_t j = 0; j < replaced_ids.size(); ++j )
    {
      // If the provenance ID is still needed by a later step, duplicate the layer
      if ( prov_use_lut[ replaced_ids[ j ] ] > i )
      {
        std::string dup_name = "input" + i_str + "_" + Core::ExportToString( j ) + "_dup";
        script.push_back( "\t" + dup_name + "=duplicatelayer(layerid=" + layer_lut[ replaced_ids[ j ] ] +
          ", sandbox=0)\n" );
        // Store the duplicate as the new input for the replaced provenance ID
        layer_lut[ replaced_ids[ j ] ] = dup_name;
      }
    }

    // Issue the action
    script.push_back( action_cmd );
    // Make sure that the output is a python list
    script.push_back( "\tif type(" + output_name + ")!=list:\n" 
      "\t\ttmp=list()\n" + "\t\ttmp.append(" + output_name + ")\n"
      "\t\t" + output_name + "=tmp\n" );
    // Print the output
    //script.push_back( "\tprint('" + output_name + " =', " + output_name + ")\n" );
  }

  // Report script progress
  script.push_back( "\treportscriptstatus(sandbox=0, current_step='', steps_done=" +
    num_steps_str + ", total_steps=" + num_steps_str + ")\n" );

  for ( size_t i = 0; i < this->prov_ids_.size(); ++i )
  {
    // Get the layer ID of the desired output
    std::string layer_id = layer_lut[ this->prov_ids_[ i ] ];
    // Set the layer name
    std::string layer_name = "Recreated_Provenance_ID_" + 
      Core::ExportToString( this->prov_ids_[ i ] );
    script.push_back( "\tset(stateid=" + layer_id + "+'::name', value='" +
      layer_name + "')\n" );
    // Move the final output layer out of the sandbox and assign the 
    // desired provenance ID to it.
    script.push_back( "\tmigratesandboxlayer(layerid=" + layer_id +
      ", sandbox=0, prov_id=" + Core::ExportToString( this->prov_ids_[ i ] ) + ")\n" );
    // Activate the layer
    script.push_back( "\tactivatelayer(layerid=" + layer_id + ")\n" );
  }

  // Exception handling
  script.push_back( "except:\n\tprint('Layer recreation failed:', sys.exc_info()[:2], file=sys.stderr)\n" );

  elapsed_time = performance_timer.elapsed();
  CORE_LOG_MESSAGE( "Time spent on generating the script: " + Core::ExportToString( elapsed_time ) );
  CORE_LOG_MESSAGE( "Script length: " + Core::ExportToString( script.size() ) );

  return true;
}

//////////////////////////////////////////////////////////////////////////
// Class ActionRecreateLayer
//////////////////////////////////////////////////////////////////////////

ActionRecreateLayer::ActionRecreateLayer() :
  private_( new ActionRecreateLayerPrivate )
{
  this->add_parameter( this->private_->prov_ids_ );
}

ActionRecreateLayer::~ActionRecreateLayer()
{
}

bool ActionRecreateLayer::validate( Core::ActionContextHandle& context )
{
#if !defined( BUILD_WITH_PYTHON )
  context->report_error( "Python is required to run ActionRecreateLayer." );
  return false;
#endif

  // Make sure there is no running recreation
  if ( LayerManager::Instance()->is_sandbox( 0 ) )
  {
    context->report_error( "A layer recreation process is already running." );
    return false;
  }

  ProvenanceIDList tmp_list;
  for ( size_t i = 0; i < this->private_->prov_ids_.size(); ++i )
  {
    LayerHandle layer = LayerManager::FindLayer( this->private_->prov_ids_[ i ] );
    if ( layer )
    {
      context->report_warning( "The requested provenance ID " + 
        Core::ExportToString( this->private_->prov_ids_[ i ] ) + " already exists as layer '" +
        layer->get_layer_name() + "'." );
    }
    else
    {
      tmp_list.push_back( this->private_->prov_ids_[ i ] );
    }
  }

  // Only keep provenance IDs that don't exist yet
  this->private_->prov_ids_ = tmp_list;
  if ( this->private_->prov_ids_.size() == 0 ) 
  {
    this->private_->prov_trail_.reset();
    return true;
  }

  ProvenanceTrailHandle prov_trail;

  // If a provenance trail was given, try to use it
  if ( this->private_->prov_trail_ && this->private_->prov_trail_->size() > 0 )
  {
    // Check the last step of the trail for the desired provenance outputs
    ProvenanceStepHandle prov_step = this->private_->prov_trail_->back();
    const ProvenanceIDList& prov_output_ids = prov_step->get_output_provenance_ids();
    bool trail_usable = true;
    for ( size_t i = 0; i < this->private_->prov_ids_.size(); ++i )
    {
      if ( std::find( prov_output_ids.begin(), prov_output_ids.end(), 
        this->private_->prov_ids_[ i ] ) == prov_output_ids.end() )
      {
        trail_usable = false;
        context->report_warning( "The provided provenance trail is incorrect." );
        break;
      }
    }

    if ( trail_usable )
    {
      prov_trail.reset( new ProvenanceTrail( *this->private_->prov_trail_ ) );
    }
  }

  boost::timer performance_timer;

  if ( !prov_trail )
  {
    // Get the provenance trail that leads to the desired provenance ID
    prov_trail = ProjectManager::Instance()->get_current_project()->
      get_provenance_trail( this->private_->prov_ids_ );
    if ( !prov_trail || prov_trail->size() == 0 )
    {
      context->report_error( "Provenance trail for provenance IDs " + 
        Core::ExportToString( this->private_->prov_ids_ ) + " doesn't exist." );
      return false;
    }
  }

  double elapsed_time = performance_timer.elapsed();
  CORE_LOG_MESSAGE( "Time spent on provenance trail query: " + Core::ExportToString( elapsed_time ) );
  performance_timer.restart();

  this->private_->prov_trail_.reset( new ProvenanceTrail );

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

  elapsed_time = performance_timer.elapsed();
  CORE_LOG_MESSAGE( "Time spent on provenance trail cleanup: " + Core::ExportToString( elapsed_time ) );

  return true; // validated
}

typedef boost::unordered_map< ProvenanceID, std::string > ProvenanceIDLayerIDMap;
typedef boost::unordered_map< ProvenanceID, size_t > ProvenanceIDStepIDMap;

bool ActionRecreateLayer::run( Core::ActionContextHandle& context, Core::ActionResultHandle& result )
{ 
  size_t num_steps;

  // If the provenance trail required to recreate the layer is empty,
  // the layer should already exists in the layer manager.
  if ( !this->private_->prov_trail_ ||
    ( num_steps = this->private_->prov_trail_->size() ) == 0 )
  {
    return true;
  }

  // Get the current counters for groups and layers, so we can undo the changes to those counters
  // NOTE: This needs to be done before a new layer is created
  LayerManager::id_count_type id_count = LayerManager::GetLayerIdCount();

  // Figure out the maximum provenance ID the provenance trail contains
  ProvenanceStepHandle last_prov_step = this->private_->prov_trail_->back();
  ProvenanceID max_prov_id = *std::max_element( last_prov_step->get_output_provenance_ids().begin(),
    last_prov_step->get_output_provenance_ids().end() );

  // Generate the Python script
  // Depending on the value of the maximum provenance ID involved,
  // used a std::vector or boost::unordered_map for lookup tables.
  Core::StringVectorHandle script( new Core::StringVector );
  std::vector< LayerHandle > input_layers;
  bool succeeded = false;
  if ( max_prov_id < 2000000 )
  {
    CORE_LOG_MESSAGE( "Generating script using std::vector" );
    std::vector< std::string > layer_lut( static_cast< size_t >( max_prov_id + 1 ) );
    std::vector< size_t > prov_use_lut( static_cast< size_t >( max_prov_id + 1 ) );
    succeeded = this->private_->generate_script( context, layer_lut, prov_use_lut, input_layers, *script );
  }
  else
  {
    CORE_LOG_MESSAGE( "Generating script using boost::unordered_map" );
    ProvenanceIDLayerIDMap layer_lut;
    ProvenanceIDStepIDMap prov_use_lut;
    succeeded = this->private_->generate_script( context, layer_lut, prov_use_lut, input_layers, *script );
  }

  if ( !succeeded ) return false;

  // Create a sandbox for running the script
  // NOTE: sandbox must be created in both the layer manager and the clipboard
  // NOTE: provenance playback always uses sandbox 0 to guarantee that only one
  // playback will be running at a time.
  LayerManager::Instance()->create_sandbox( 0 );
  Clipboard::Instance()->create_sandbox( 0 );

  // Add the input layers to the sandbox
  for ( size_t i = 0; i < input_layers.size(); ++i )
  {
    LayerManager::Instance()->insert_layer( input_layers[ i ], 0 );
  }
  
  // Create undo/redo record
  LayerRecreationUndoBufferItemHandle item( new LayerRecreationUndoBufferItem( 
    this->private_->prov_ids_, 0 ) );
  item->set_redo_action( this->shared_from_this() );
  item->add_id_count_to_restore( id_count );
  UndoBuffer::Instance()->insert_undo_item( context, item );

#ifdef BUILD_WITH_PYTHON
  // Run the script to recreate the layer
  Core::PythonInterpreter::Instance()->run_script( script );

  // Clean up, ignore any exceptions that might happen
  // NOTE: This part is separated out so that if the previous script failed to compile or run,
  // the sandbox can still be deleted properly.
  std::string cleanup_cmd( "try:\n\tendscriptstatusreport(sandbox=0)\nexcept:\n\tpass\n"
    "try:\n\tdeletesandbox(sandbox=0)\nexcept:\n\tpass\n" );
  Core::PythonInterpreter::Instance()->run_script( cleanup_cmd );
#endif

  return true;
}

void ActionRecreateLayer::clear_cache()
{
  this->private_->prov_trail_.reset();
}

void ActionRecreateLayer::Dispatch( Core::ActionContextHandle context, 
    const std::vector< ProvenanceID >& prov_ids, ProvenanceTrailHandle prov_trail )
{
  ActionRecreateLayer* action = new ActionRecreateLayer;
  action->private_->prov_ids_ = prov_ids;
  action->private_->prov_trail_ = prov_trail;
  Core::ActionDispatcher::PostAction( Core::ActionHandle( action ), context );
}

} // end namespace Seg3D

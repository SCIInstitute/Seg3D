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
 
#ifndef APPLICATION_LAYER_LAYERACTION_H
#define APPLICATION_LAYER_LAYERACTION_H 
 
// Core includes 
#include <Core/Action/Action.h> 

// Application includes
#include <Application/Provenance/Provenance.h>
#include <Application/Layer/LayerFWD.h> 

namespace Seg3D
{

// Forward declarations
class LayerAction;
class LayerActionPrivate;
typedef boost::shared_ptr<LayerAction> LayerActionHandle;
typedef boost::shared_ptr<LayerActionPrivate> LayerActionPrivateHandle;

class LayerAction : public Core::Action
{
  // -- constructor --
public:
  // Constructor that generates the private class
  LayerAction();

  // -- adding parameters --
public:

  /// ADD_GROUP_ID
  /// Add group id as an input parameter
  /// NOTE: This will switch on provenance tracking
  void add_group_id( std::string& group_id );

  /// ADD_LAYER_ID
  /// Add layer id as an input parameter
  /// NOTE: This will switch on provenance tracking
  void add_layer_id( std::string& layer_id );

  /// ADD_LAYER_ID_LIST
  /// Add layer id lsit as an input parameter
  /// NOTE: This will switch on provenance tracking
  void add_layer_id_list( std::vector<std::string>& layer_id_list );

  // -- translate provenance information --
public:
  /// TRANSLATE:
  /// Some actions need to be translated before they can be validated. Translate takes
  /// care of most provenance related issue, by for example translating the provenance
  /// information into real action information. This function is called before validate
  /// NOTE: This function is *not* const and may alter the values of the parameters
  ///       and correct faulty input.
  virtual bool translate( Core::ActionContextHandle& context );
  
  // -- deal with dependencies for provenance --
public:
  /// NOTE: Dependency lists need to inserted into a ProvenanceRecord and hence this function can
  /// only be called from within run_layer_action or validate_layer_action
  
  /// GET_INPUT_PROVENANCE_IDS:
  /// Get the dependencies that this action is depending on
  ProvenanceIDList get_input_provenance_ids();
  
  // -- deal with new provenance ids for output --
public: 
  /// SET_OUTPUT_PROVENANCE_IDS:
  /// Set the output provenance ids
  void set_output_provenance_ids( const ProvenanceIDList& provenance_ids );
  
  /// GET_OUTPUT_PROVENANCE_ID:
  /// Get the provenance id of output layer indexed by index. If no provenance id was assigned
  /// a new one is created
  ProvenanceID get_output_provenance_id( size_t index = 0 );

  /// GET_OUTPUT_PROVENANCE_IDS:
  /// Get or create all the assigned provenance ids
  ProvenanceIDList get_output_provenance_ids( size_t num_provenance_ids = 0 );

  /// GENERATE_OUTPUT_PROVENANCE_IDS:
  /// Generate all needed provenance ids
  void generate_output_provenance_ids( size_t num_provenance_ids );

  // -- Export provenance string --
public:
  /// EXPORT_PARAMS_TO_PROVENANCE_STRING:
  /// Export the action parameters to a string and mark the provenance inputs
  /// with special placeholders.
  std::string export_params_to_provenance_string( bool single_input = false ) const;
  
  // -- internals --
private:  
  LayerActionPrivateHandle private_;
};

} // end namespace Seg3D

#endif

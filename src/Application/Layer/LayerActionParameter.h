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
 
#ifndef APPLICATION_LAYER_LAYERACTIONPARAMETER_H
#define APPLICATION_LAYER_LAYERACTIONPARAMETER_H 
 
// Core includes 
#include <Core/Action/ActionParameter.h> 

// Application includes
#include <Application/Provenance/Provenance.h>
#include <Application/Layer/LayerFWD.h> 

namespace Seg3D {


class LayerActionParameter : public Core::ActionParameterBase 
{
  // -- destructor --
public:
  virtual ~LayerActionParameter();

  // -- functions for accessing data --
public:

  /// TRANSLATE_PROVENANCE
  /// Translate provenance inputs into current object. The function will also
  /// add input provenance ids to a list recording dependencies.
  virtual bool translate_provenance( ProvenanceIDList& input_provenance ) = 0;

  /// EXPORT_TO_PROVENANCE_STRING
  /// Export the contents of the parameter to a provenance string.
  /// This means layer ids will be translated to provenance id.
  virtual std::string export_to_provenance_string( size_t& input_counter, bool single_input ) const = 0;
  
  /// HAS_EXTENSION
  /// Has extended information in the derived class.
  virtual bool has_extension() const;
};

class LayerActionLayerID : public LayerActionParameter
{

  // -- constructor/destructor --
public:
  LayerActionLayerID( std::string& layer_id );
  virtual ~LayerActionLayerID();
  
  // -- functions for accessing data --
public:

  /// IMPORT_FROM_STRING
  /// Import a parameter from a string. The function returns true
  /// if the import succeeded
  virtual bool import_from_string( const std::string& str );

  /// EXPORT_TO_STRING
  /// Export the contents of the parameter to string
  virtual std::string export_to_string() const;

  /// TRANSLATE_PROVENANCE
  /// Translate provenance inputs into current object. The function will also
  /// add input provenance ids to a list recording dependencies.
  virtual bool translate_provenance( ProvenanceIDList& input_provenance );

  /// EXPORT_TO_PROVENANCE_STRING
  /// Export the contents of the parameter to a provenance string.
  /// This means layer ids will be translated to provenance id.
  virtual std::string export_to_provenance_string( size_t& input_counter, bool single_input ) const;

private:
  std::string& layer_id_;
  ProvenanceID provenance_id_;
};

class LayerActionLayerIDList : public LayerActionParameter
{

  // -- constructor/destructor --
public:
  LayerActionLayerIDList( std::vector<std::string>& layer_id_list );
  virtual ~LayerActionLayerIDList();
  
  // -- functions for accessing data --
public:

  /// IMPORT_FROM_STRING
  /// Import a parameter from a string. The function returns true
  /// if the import succeeded
  virtual bool import_from_string( const std::string& str );

  /// EXPORT_TO_STRING
  /// Export the contents of the parameter to string
  virtual std::string export_to_string() const;

  /// TRANSLATE_PROVENANCE
  /// Translate provenance inputs into current object. The function will also
  /// add input provenance ids to a list recording dependencies.
  virtual bool translate_provenance( ProvenanceIDList& input_provenance );

  /// EXPORT_TO_PROVENANCE_STRING
  /// Export the contents of the parameter to a provenance string.
  /// This means layer ids will be translated to provenance id.
  virtual std::string export_to_provenance_string( size_t& input_counter, bool single_input ) const;

private:
  std::vector<std::string>& layer_id_list_;
  std::vector<ProvenanceID> provenance_id_list_;
};

class LayerActionGroupID : public LayerActionParameter
{

  // -- constructor/destructor --
public:
  LayerActionGroupID( std::string& group_id );
  virtual ~LayerActionGroupID();
  
  // -- functions for accessing data --
public:

  /// IMPORT_FROM_STRING
  /// Import a parameter from a string. The function returns true
  /// if the import succeeded
  virtual bool import_from_string( const std::string& str );

  /// EXPORT_TO_STRING
  /// Export the contents of the parameter to string
  virtual std::string export_to_string() const;

  /// TRANSLATE_PROVENANCE
  /// Translate provenance inputs into current object. The function will also
  /// add input provenance ids to a list recording dependencies.
  virtual bool translate_provenance( ProvenanceIDList& input_provenance );

  /// EXPORT_TO_PROVENANCE_STRING
  /// Export the contents of the parameter to a provenance string.
  /// This means layer ids will be translated to provenance id.
  virtual std::string export_to_provenance_string( size_t& input_counter, bool single_input ) const;

private:
  std::string& group_id_;
  ProvenanceID provenance_id_;
};

} // end namespace Seg3D

#endif

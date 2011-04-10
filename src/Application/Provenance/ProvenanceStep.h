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

#ifndef APPLICATION_PROVENANCE_PROVENANCESTEP_H
#define APPLICATION_PROVENANCE_PROVENANCESTEP_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// Boost includes
#include <boost/utility.hpp>
#include <boost/smart_ptr.hpp>

// Application includes
#include <Application/Provenance/Provenance.h>

namespace Seg3D
{

// Forward Declaration
class ProvenanceStep;
typedef boost::shared_ptr<ProvenanceStep> ProvenanceStepHandle;


class ProvenanceStep : boost::noncopyable
{
  // -- constructor/destructor --
public:
  ProvenanceStep();

  virtual ~ProvenanceStep();

  // -- accessors --
public:
  // GET_INPUT_PROVENANCE_IDS:
  // Get the input provenance ids
  const ProvenanceIDList& get_input_provenance_ids() const;
  
  // SET_INPUT_PROVENANCE_IDS:
  // Set the input provenance ids
  void set_input_provenance_ids( const ProvenanceIDList& input_provenance_ids );

  // GET_OUTPUT_PROVENANCE_IDS:
  // Get the input provenance ids
  const ProvenanceIDList& get_output_provenance_ids() const;
  
  // SET_OUTPUT_PROVENANCE_IDS:
  // Set the input provenance ids
  void set_output_provenance_ids( const ProvenanceIDList& output_provenance_ids );

  // GET_DELETED_PROVENANCE_IDS:
  // Get the deleted provenance ids
  const ProvenanceIDList& get_deleted_provenance_ids() const;
  
  // SET_DELETED_PROVENANCE_IDS:
  // Set the input provenance ids
  void set_deleted_provenance_ids( const ProvenanceIDList& deleted_provenance_ids );

  // SET_ACTION:
  // Set the provenance action
  void set_action( const std::string& action );
  
  // GET_ACTION:
  // Get the provenance action
  const std::string& get_action() const;
  
  // GET_USER:
  // get the user's name
  const std::string& get_username() const;
  
  // GET_TIMESTAMP:
  // get the timestamp of the provenance step
  const std::string& get_timestamp() const;

  // -- properties --
public:
  virtual bool is_collapsable();

  // -- debug tools --
public:
  // PRINT
  // Print the provenance record to cout
  void print();

  // -- internals --
protected:
  // List of provenance ids that this step depends on
  ProvenanceIDList input_provenance_ids_;
  
  // List of provenance ids that this action will generate
  ProvenanceIDList output_provenance_ids_;
  
  // List of provenance ids that this action will delete
  ProvenanceIDList deleted_provenance_ids_;

  // Description of the action
  std::string action_;

  // User that generated action
  std::string user_;
  
  // Time stamp for action
  std::string time_stamp_;
};

} // end namespace Seg3D

#endif

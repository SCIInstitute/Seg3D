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

// Core includes
#include <Core/Application/Application.h>

// Application includes
#include <Application/Provenance/ProvenanceStep.h>

namespace Seg3D
{

ProvenanceStep::ProvenanceStep() :
  inputfiles_id_( -1 )
{
  // Record user for GLP
  Core::Application::Instance()->get_user_name( this->user_ );
}

ProvenanceStep::~ProvenanceStep()
{
}

const ProvenanceIDList& ProvenanceStep::get_input_provenance_ids() const
{
  return this->input_provenance_ids_;
}

void ProvenanceStep::set_input_provenance_ids( const ProvenanceIDList& input_provenance_ids )
{
  this->input_provenance_ids_ = input_provenance_ids;
}

const ProvenanceIDList& ProvenanceStep::get_output_provenance_ids() const
{
  return this->output_provenance_ids_;
}

void ProvenanceStep::set_output_provenance_ids( const ProvenanceIDList& output_provenance_ids )
{
  this->output_provenance_ids_ = output_provenance_ids;
}

const ProvenanceIDList& ProvenanceStep::get_replaced_provenance_ids() const
{
  return this->replaced_provenance_ids_;
}

void ProvenanceStep::set_replaced_provenance_ids( const ProvenanceIDList& deleted_provenance_ids )
{
  this->replaced_provenance_ids_ = deleted_provenance_ids;
}

void ProvenanceStep::set_inputfiles_id( const InputFilesID inputfiles_id )
{
  this->inputfiles_id_ = inputfiles_id;
}
  
InputFilesID ProvenanceStep::get_inputfiles_id() const
{
  return this->inputfiles_id_;
}

void ProvenanceStep::set_username( const std::string& username )
{
  this->user_ = username;
}

const std::string& ProvenanceStep::get_username() const
{
  return this->user_;
}

void ProvenanceStep::set_provenance_ids_of_interest( const ProvenanceIDList& poi )
{
  this->prov_ids_of_interest_ = poi;
}

const ProvenanceIDList& ProvenanceStep::get_provenance_ids_of_interest() const
{
  return this->prov_ids_of_interest_;
}

void ProvenanceStep::set_action_name( const std::string& action_name )
{
  this->action_name_ = action_name;
}

const std::string& ProvenanceStep::get_action_name() const
{
  return this->action_name_;
}

void ProvenanceStep::set_action_params( const std::string& action_params )
{
  this->action_params_ = action_params;
}

const std::string& ProvenanceStep::get_action_params() const
{
  return this->action_params_;
}

void ProvenanceStep::set_timestamp( timestamp_type timestamp )
{
  this->timestamp_ = timestamp;
}

ProvenanceStep::timestamp_type ProvenanceStep::get_timestamp() const
{
  return this->timestamp_;
}

} // end namespace Seg3D

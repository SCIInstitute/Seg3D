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

// STL includes
#include <iostream>

// Boost includes
#include <boost/date_time.hpp>

// Core includes
#include <Core/Utils/StringUtil.h> 
#include <Core/Application/Application.h>

// Application includes
#include <Application/Provenance/ProvenanceStep.h>

namespace Seg3D
{

ProvenanceStep::ProvenanceStep()
{
  // Record user for GLP
  Core::Application::Instance()->get_user_name( this->user_ );
  // Record timestamp for GLP
  this->time_stamp_ = boost::posix_time::to_simple_string( 
    boost::posix_time::second_clock::local_time() );
}

ProvenanceStep::~ProvenanceStep()
{
}

bool ProvenanceStep::is_collapsable()
{
  return false;
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

const ProvenanceIDList& ProvenanceStep::get_deleted_provenance_ids() const
{
  return this->deleted_provenance_ids_;
}

void ProvenanceStep::set_deleted_provenance_ids( const ProvenanceIDList& deleted_provenance_ids )
{
  this->deleted_provenance_ids_ = deleted_provenance_ids;
}

void ProvenanceStep::set_action( const std::string& action )
{
  this->action_ = action;
}

const std::string& ProvenanceStep::get_action() const
{
  return this->action_;
}

const std::string& ProvenanceStep::get_username() const
{
  return this->user_;
}

const std::string& ProvenanceStep::get_timestamp() const
{
  return this->time_stamp_;
}

void ProvenanceStep::print()
{
  std::cout << "input ids = " << Core::ExportToString( this->input_provenance_ids_ ) << std::endl;
  std::cout << "output ids = " << Core::ExportToString( this->output_provenance_ids_ ) << std::endl;
  std::cout << "deleted ids = " << Core::ExportToString( this->deleted_provenance_ids_ ) << std::endl;
  std::cout << "action = " << this->action_ << std::endl;
  std::cout << "user = " << this->user_ << std::endl;
  std::cout << "timestamp = " << this->time_stamp_ << std::endl;
}





} // end namespace Seg3D

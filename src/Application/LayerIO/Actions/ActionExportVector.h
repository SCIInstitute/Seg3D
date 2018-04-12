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

#ifndef APPLICATION_LAYERIO_ACTIONS_ACTIONEXPORTVECTOR_H
#define APPLICATION_LAYERIO_ACTIONS_ACTIONEXPORTVECTOR_H

// Core includes
#include <Core/Action/Actions.h>
#include <Core/Interface/Interface.h>

#include <Core/Geometry/Point.h>

#include <fstream>

namespace Seg3D
{

template <class T, class WriterT>
class ActionExportVector : public Core::Action
{
public:
  using Vector = std::vector<T>;

  ActionExportVector(const std::string& file_path, const Vector& vector, WriterT writer) :
    file_path_(file_path),
    vector_(vector),
    writer_(writer)
  {
  }

  ActionExportVector()
  {
  }

  // -- Functions that describe action --
public:
  // VALIDATE:
  // Each action needs to be validated just before it is posted. This way we
  // enforce that every action that hits the main post_action signal will be
  // a valid action to execute.
  virtual bool validate( Core::ActionContextHandle& context ) override
  {
    if (this->file_path_.empty())
    {
      context->report_error(std::string("File path is empty."));
      return false;
    }

    if (this->vector_.empty())
    {
      context->report_error(std::string("No vector available to save."));
      return false;
    }
    return true; // validated
  }
  
  // RUN:
  // Each action needs to have this piece implemented. It spells out how the
  // action is run. It returns whether the action was successful or not.
  virtual bool run( Core::ActionContextHandle& context, Core::ActionResultHandle&) override
  {
    std::string message("Exporting vector.");

    Core::ActionProgressHandle progress =
      Core::ActionProgressHandle(new Core::ActionProgress(message));

    progress->begin_progress_reporting();

    boost::filesystem::path filename = boost::filesystem::path(this->file_path_);

    std::ofstream outputfile;
    outputfile.exceptions(std::ofstream::failbit | std::ofstream::badbit);

    try
    {
      outputfile.open(filename.string().c_str());

      std::ios_base::fmtflags ff;
      ff = outputfile.flags();
      ff |= outputfile.showpoint; // write floating-point values including always the decimal point
      ff |= outputfile.fixed; // write floating point values in fixed-point notation
      outputfile.flags(ff);

      for (const auto& t : vector_)
      {
        writer_(outputfile, t);
      }
    }
    catch (...)
    {
      context->report_error("Could not open and write to file: " + filename.string());
      return false;
    }
    outputfile.close();

    progress->end_progress_reporting();

    return true;
  }

  void set_file_path( const std::string& file_path ) { file_path_ = file_path; }
  void set_vector(const Vector& vector) { vector_ = vector; }

  // -- Action parameters --
protected:
  void init_parameters()
  {
    //TODO figure out
    this->add_parameter( this->file_path_ );
    this->add_parameter(this->vector_);
  }
private:
  // Where the layer should be exported
  std::string file_path_;
  // Vector to be exported
  Vector vector_;

  WriterT writer_;
};

} // end namespace Seg3D

#endif

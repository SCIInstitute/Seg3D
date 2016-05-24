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

#ifndef CORE_PARSER_ARRAYMATHENGINE_H 
#define CORE_PARSER_ARRAYMATHENGINE_H 

// Core includes
#include <Core/Parser/ArrayMathInterpreter.h>
#include <Core/Parser/Parser.h>
#include <Core/Parser/ParserFWD.h>

namespace Core
{

// Hide header includes, private interface and implementation
class ArrayMathEnginePrivate;
typedef boost::shared_ptr< ArrayMathEnginePrivate > ArrayMathEnginePrivateHandle;

class ArrayMathEngine : public Parser, public ArrayMathInterpreter
{
public:
  // CALLS TO THIS CLASS SHOULD BE MADE IN THE ORDER
  // THAT THE FUNCTIONS ARE GIVEN HERE

  // Make sure it starts with a clean definition file
  ArrayMathEngine();

  /// Generate input arrays
  bool add_input_data_block( std::string name, DataBlockHandle data_block, std::string& error );
  bool add_input_mask_data_block( std::string name, MaskDataBlockHandle mask_data_block, 
    std::string& error );

  //bool add_index( std::string name );
  //bool add_size( std::string name );

  bool add_output_data_block( std::string name, size_t nx, size_t ny, size_t nz, 
    Core::DataType type, std::string& error );
  
  /// Setup the expression                        
  bool add_expressions( std::string& expressions );

  /// Parse and validate the inputs/outputs/expression.
  bool parse_and_validate( std::string& error );

  /// Run the expressions in parallel
  bool run( std::string& error );

  /// Extract handles to the results
  bool get_data_block( std::string name, DataBlockHandle& data_block );

  /// Clean up the engine
  void clear();

  typedef boost::signals2::signal< void (double) > update_progress_signal_type;

  // UPDATE_PROGRESS:
  /// When new information on progress is available this signal is triggered. If this signal is 
  /// triggered it should end with a value 1.0 indicating that progress reporting has finised.
  /// Progress is measured between 0.0 and 1.0.
  update_progress_signal_type update_progress_signal_;

private:
  void update_progress( double amount );

  ArrayMathEnginePrivateHandle private_;
};

}

#endif

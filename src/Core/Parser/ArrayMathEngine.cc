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
#include <Core/DataBlock/DataBlock.h>
#include <Core/DataBlock/MaskDataBlock.h>
#include <Core/DataBlock/StdDataBlock.h>
#include <Core/Parser/ArrayMathEngine.h>
#include <Core/Parser/ArrayMathFunctionCatalog.h>
#include <Core/Parser/ArrayMathProgram.h>
#include <Core/Parser/ParserEnums.h>
#include <Core/Utils/StringUtil.h>

namespace Core
{

class ArrayMathEnginePrivate
{
public:

  class OutputDataBlock
  {
  public:
    std::string array_name_;
    std::string data_block_name_; 
    DataBlockHandle data_block_;
  };

  // We don't support outputting a MaskDataBlock because it would have to be registered with the
  // MaskDataBlockManager and locked to prevent conflicts with other masks.  Also, we can avoid 
  // bit operations when copying back the parser result.

  // Parser program : the structure of the expressions and simple reduction
  // of the expressions to simple function calls
  ParserProgramHandle pprogram_;
  // Wrapper around the function calls, this piece actually executes the code
  ArrayMathProgramHandle mprogram_;

  // Expression to evaluate before the main expression
  // This one is to extract the variables from the data sources
  // This reduces the amount of actual functions we need to implement
  std::string pre_expression_;
  // The user defined expression
  std::string expression_;
  // Expression to get the data back into the data sinks
  std::string post_expression_;

  // The size of the array engine, the first call that add an array that is
  // bigger than 1, will set this variable
  // Any subsequent array that does not match the size will cause an error
  size_type array_size_;

  // Data that needs to be stored as it is needed before and after the parser is
  // done. An output needs to be set before the parser, otherwise it is optimized
  // away, but the type is only know when the parser has validated and optimized
  // the expression tree
  std::vector< OutputDataBlock > data_block_data_;
};

ArrayMathEngine::ArrayMathEngine() :
  private_( new ArrayMathEnginePrivate )
{
  this->clear();
}

void ArrayMathEngine::clear()
{
  // Reset all values
  this->private_->pprogram_.reset();
  this->private_->mprogram_.reset();

  this->private_->pre_expression_.clear();
  this->private_->expression_.clear();
  this->private_->post_expression_.clear();
  this->private_->array_size_ = 1;
}

bool ArrayMathEngine::add_input_data_block( std::string name, DataBlockHandle data_block, std::string& error )
{
  if ( data_block->get_size() == 0 )
  {
    error = "Input data block '" + name + "' is empty.";
    return false;
  }

  size_type size = data_block->get_size();

  // Check whether size is OK
  if ( size > 1 )
  {
    if ( this->private_->array_size_ == 1 ) 
    {
      this->private_->array_size_ = size;
    }
    if ( this->private_->array_size_ != size )
    {
      error = "The number of array elements in '" + name
        + "' does not match the size of the other objects.";
      return false;
    }
  }

  // Add __NAME as an additional variable that extracts the field data
  // from the field

  std::string tname = "__" + name;

  this->private_->pre_expression_ += name + "=get_scalar(" + tname + ");";

  int flags = 0;
  if ( size > 1 ) 
  {
    flags = SCRIPT_SEQUENTIAL_VAR_E;
  }

  // Add the variable to the interpreter
  if ( !( this->add_data_block_source( this->private_->mprogram_, tname, data_block, error ) ) )
  {
    return false;
  }
  // Add the variable to the parser
  if ( !( this->add_input_variable( this->private_->pprogram_, tname, "DATA", flags ) ) )
  {
    return false;
  }
  return true;
}


bool ArrayMathEngine::add_input_mask_data_block( std::string name, 
  MaskDataBlockHandle mask_data_block, std::string& error )
{
  if ( mask_data_block->get_size() == 0 )
  {
    error = "Input mask data block '" + name + "' is empty.";
    return false;
  }

  size_type size = mask_data_block->get_size();

  // Check whether size is OK
  if ( size > 1 )
  {
    if ( this->private_->array_size_ == 1 ) 
    {
      this->private_->array_size_ = size;
    }
    if ( this->private_->array_size_ != size )
    {
      error = "The number of array elements in '" + name
        + "' does not match the size of the other objects.";
      return false;
    }
  }

  // Add __NAME as an additional variable that extracts the field data
  // from the field

  std::string tname = "__" + name;

  this->private_->pre_expression_ += name + "=get_scalar(" + tname + ");";

  int flags = 0;
  if ( size > 1 ) 
  {
    flags = SCRIPT_SEQUENTIAL_VAR_E;
  }

  // Add the variable to the interpreter
  if ( !( this->add_mask_data_block_source( this->private_->mprogram_, tname, mask_data_block, error ) ) )
  {
    return false;
  }
  // Add the variable to the parser
  if ( !( this->add_input_variable( this->private_->pprogram_, tname, "MASK", flags ) ) )
  {
    return false;
  }
  return true;
}

/*bool ArrayMathEngine::add_index( std::string name )
{
  this->pre_expression_ += name + "=index();";
  return true;
}

bool ArrayMathEngine::add_size( std::string name )
{
  this->pre_expression_ += name + "=" + ExportToString( this->array_size_ ) + ";";
  return true;
}*/

bool ArrayMathEngine::add_output_data_block( std::string name, size_t nx, size_t ny, size_t nz, 
  Core::DataType type, std::string& error )
{
  size_t size = nx * ny * nz;
  if ( static_cast<size_t>( this->private_->array_size_ ) != size )
  {
    error = "The output field '" + name + 
      "' does not have the same number of elements as the other objects.";
    return false;  
  }

  std::string tname = "__" + name;

  if ( size == 0 )
  {
    return false;
  }

  int flags = 0;
  if ( size > 1 ) 
  {
    flags = SCRIPT_SEQUENTIAL_VAR_E;  
  }

  this->private_->post_expression_ += tname + "=to_data_block(" + name + ");";

  // Add the variable to the parser
  // Temporary buffer
  if ( !( add_output_variable( this->private_->pprogram_, name, "S", flags ) ) )
  {
    return false;
  }
  // Final buffer
  if ( !( add_output_variable( this->private_->pprogram_, tname, "DATA", flags ) ) )
  {
    return false;
  }

  // Create new output data block based on input data block
  Core::DataBlockHandle output_data_block( Core::StdDataBlock::New( nx, ny, nz, type ) );

  // Store information for processing after parsing has succeeded
  ArrayMathEnginePrivate::OutputDataBlock m;
  m.array_name_ = name;
  m.data_block_name_ = tname;
  m.data_block_ = output_data_block;

  this->private_->data_block_data_.push_back( m );

  return true;
}

bool ArrayMathEngine::add_expressions( std::string& expressions )
{
  this->private_->expression_ += expressions;
  return true;
}

bool ArrayMathEngine::parse_and_validate( std::string& error )
{
  // Link everything together
  std::string full_expression = this->private_->pre_expression_ + ";" + this->private_->expression_ + ";" + 
    this->private_->post_expression_;

  // Parse the full expression
  if ( !( this->parse( this->private_->pprogram_, full_expression, error ) ) )
  {
    return false;
  }

  // Get the catalog with all possible functions
  ParserFunctionCatalogHandle catalog = ArrayMathFunctionCatalog::get_catalog();

  // Validate the expressions
  if ( !( this->validate( this->private_->pprogram_, catalog, error ) ) )
  {
    return false;
  }
  return true;
}

bool ArrayMathEngine::run( std::string& error )
{
  // Optimize the expressions
  if ( !( this->optimize( this->private_->pprogram_, error ) ) )
  {
    return false;
  }

  // DEBUG CALL
#ifdef DEBUG
  this->private_->pprogram_->print();
#endif

  for ( size_t j = 0; j < this->private_->data_block_data_.size(); j++ )
  {
    if ( this->private_->data_block_data_[ j ].data_block_.get() )
    {
      std::string arrayname = this->private_->data_block_data_[ j ].data_block_name_;
      if ( !( this->add_data_block_sink( this->private_->mprogram_, arrayname,
        this->private_->data_block_data_[ j ].data_block_, error ) ) )
      {
        return false;
      }
    }
  }

  // Translate the code
  if ( !( this->translate( this->private_->pprogram_, this->private_->mprogram_, error ) ) )
  {
    return false;
  }
  // Set the final array size
  if ( !( this->set_array_size( this->private_->mprogram_, this->private_->array_size_ ) ) )
  {
    return false;
  }

  // Connect the ArrayMathProgram update progress signal to the engine update progress signal
  this->private_->mprogram_->update_progress_signal_.connect( 
    boost::bind( &ArrayMathEngine::update_progress, this, _1 ) );

  // Run the program
  if ( !( ArrayMathInterpreter::run( this->private_->mprogram_, error ) ) )
  {
    return false;
  }
  return true;
}

bool ArrayMathEngine::get_data_block( std::string name, DataBlockHandle& data_block )
{
  for ( size_t j = 0; j < this->private_->data_block_data_.size(); j++ )
  {
    if ( this->private_->data_block_data_[ j ].array_name_ == name )
    {
      data_block = this->private_->data_block_data_[ j ].data_block_;
      return true;
    }
  }

  return false;
}

void ArrayMathEngine::update_progress( double amount )
{
  this->update_progress_signal_( amount );
}

} // end namespace





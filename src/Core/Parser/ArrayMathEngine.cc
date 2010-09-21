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

// Core includes
#include <Core/DataBlock/StdDataBlock.h>
#include <Core/Parser/ArrayMathEngine.h>
#include <Core/Parser/ArrayMathFunctionCatalog.h>
#include <Core/Parser/ParserEnums.h>
#include <Core/Utils/Log.h>
#include <Core/Utils/StringUtil.h>

namespace Core
{

ArrayMathEngine::ArrayMathEngine()
{
  clear();
}

void ArrayMathEngine::clear()
{
  // Reset all values
  this->pprogram_.reset();
  this->mprogram_.reset();

  this->pre_expression_.clear();
  this->expression_.clear();
  this->post_expression_.clear();
  this->array_size_ = 1;
}

bool ArrayMathEngine::add_input_bool_array( std::string name, std::vector< bool >* array )
{
  std::string error_str;
  if ( array->size() == 0 )
  {
    error_str = "Input boolean array '" + name + "' is empty.";
    CORE_LOG_ERROR( error_str );
    return false;
  }

  size_type size = array->size();

  // Check whether size is OK
  if ( size > 1 )
  {
    if ( this->array_size_ == 1 ) this->array_size_ = size;
    if ( this->array_size_ != size )
    {
      error_str = "The number of array elements in '" + name
          + "' does not match the size of the other objects.";
      CORE_LOG_ERROR( error_str );
      return false;
    }
  }

  // Add __NAME as an additional variable that extracts the field data
  // from the field

  std::string tname = "__" + name;

  this->pre_expression_ += name + "=get_scalar(" + tname + ");";

  int flags = 0;
  if ( size > 1 ) 
  {
    flags = SCRIPT_SEQUENTIAL_VAR_E;
  }

  // Add the variable to the interpreter
  if ( !( add_bool_array_source( this->mprogram_, tname, array, error_str ) ) )
  {
    CORE_LOG_ERROR( error_str );
    return false;
  }
  // Add the variable to the parser
  if ( !( add_input_variable( this->pprogram_, tname, "AB", flags ) ) )
  {
    CORE_LOG_ERROR( error_str );
    return false;
  }
  return true;
}

bool ArrayMathEngine::add_input_double_array( std::string name, std::vector< double >* array )
{
  std::string error_str;
  if ( array->size() == 0 )
  {
    error_str = "Input double array '" + name + "' is empty.";
    CORE_LOG_ERROR( error_str );
    return false;
  }

  size_type size = array->size();

  // Check whether size is OK
  if ( size > 1 )
  {
    if ( this->array_size_ == 1 ) 
    {
      this->array_size_ = size;
    }
    if ( this->array_size_ != size )
    {
      error_str = "The number of array elements in '" + name
          + "' does not match the size of the other objects.";
      CORE_LOG_ERROR( error_str );
      return false;
    }
  }

  // Add __NAME as an additional variable that extracts the field data
  // from the field

  std::string tname = "__" + name;

  this->pre_expression_ += name + "=get_scalar(" + tname + ");";

  int flags = 0;
  if ( size > 1 ) 
  {
    flags = SCRIPT_SEQUENTIAL_VAR_E;
  }

  // Add the variable to the interpreter
  if ( !( add_double_array_source( this->mprogram_, tname, array, error_str ) ) )
  {
    CORE_LOG_ERROR( error_str );
    return false;
  }
  // Add the variable to the parser
  if ( !( add_input_variable( this->pprogram_, tname, "AD", flags ) ) )
  {
    CORE_LOG_ERROR( error_str );
    return false;
  }
  return true;
}

bool ArrayMathEngine::add_input_data_block( std::string name, DataBlockHandle data_block )
{
  std::string error_str;
  if ( data_block->get_size() == 0 )
  {
    error_str = "Input data block '" + name + "' is empty.";
    CORE_LOG_ERROR( error_str );
    return false;
  }

  size_type size = data_block->get_size();

  // Check whether size is OK
  if ( size > 1 )
  {
    if ( this->array_size_ == 1 ) 
    {
      this->array_size_ = size;
    }
    if ( this->array_size_ != size )
    {
      error_str = "The number of array elements in '" + name
        + "' does not match the size of the other objects.";
      CORE_LOG_ERROR( error_str );
      return false;
    }
  }

  // Add __NAME as an additional variable that extracts the field data
  // from the field

  std::string tname = "__" + name;

  this->pre_expression_ += name + "=get_scalar(" + tname + ");";

  int flags = 0;
  if ( size > 1 ) 
  {
    flags = SCRIPT_SEQUENTIAL_VAR_E;
  }

  // Add the variable to the interpreter
  if ( !( this->add_data_block_source( this->mprogram_, tname, data_block, error_str ) ) )
  {
    CORE_LOG_ERROR( error_str );
    return false;
  }
  // Add the variable to the parser
  if ( !( this->add_input_variable( this->pprogram_, tname, "DATA", flags ) ) )
  {
    CORE_LOG_ERROR( error_str );
    return false;
  }
  return true;
}


bool ArrayMathEngine::add_input_mask_data_block( std::string name, MaskDataBlockHandle mask_data_block )
{
  std::string error_str;
  if ( mask_data_block->get_size() == 0 )
  {
    error_str = "Input mask data block '" + name + "' is empty.";
    CORE_LOG_ERROR( error_str );
    return false;
  }

  size_type size = mask_data_block->get_size();

  // Check whether size is OK
  if ( size > 1 )
  {
    if ( this->array_size_ == 1 ) 
    {
      this->array_size_ = size;
    }
    if ( this->array_size_ != size )
    {
      error_str = "The number of array elements in '" + name
        + "' does not match the size of the other objects.";
      CORE_LOG_ERROR( error_str );
      return false;
    }
  }

  // Add __NAME as an additional variable that extracts the field data
  // from the field

  std::string tname = "__" + name;

  this->pre_expression_ += name + "=get_scalar(" + tname + ");";

  int flags = 0;
  if ( size > 1 ) 
  {
    flags = SCRIPT_SEQUENTIAL_VAR_E;
  }

  // Add the variable to the interpreter
  if ( !( this->add_mask_data_block_source( this->mprogram_, tname, mask_data_block, error_str ) ) )
  {
    CORE_LOG_ERROR( error_str );
    return false;
  }
  // Add the variable to the parser
  if ( !( this->add_input_variable( this->pprogram_, tname, "MASK", flags ) ) )
  {
    CORE_LOG_ERROR( error_str );
    return false;
  }
  return true;
}

bool ArrayMathEngine::add_index( std::string name )
{
  this->pre_expression_ += name + "=index();";
  return true;
}

bool ArrayMathEngine::add_size( std::string name )
{
  this->pre_expression_ += name + "=" + ExportToString( this->array_size_ ) + ";";
  return true;
}

bool ArrayMathEngine::add_output_bool_array( std::string name, std::vector< bool >* array )
{
  std::string error_str;
  std::string tname = "__" + name;

  this->post_expression_ += tname + "=to_bool_array(" + name + ");";

  // Add the variable to the parser
  if ( !( add_output_variable( this->pprogram_, name, "S", 0 ) ) )
  {
    CORE_LOG_ERROR( error_str );
    return false;
  }
  if ( !( add_output_variable( this->pprogram_, tname, "AB", 0 ) ) )
  {
    CORE_LOG_ERROR( error_str );
    return false;
  }

  array->resize( this->array_size_ );

  // Store information for processing after parsing has succeeded
  OutputBoolArray m;
  m.array_name_ = name;
  m.bool_array_name_ = tname;
  m.bool_array_ = array;

  this->bool_array_data_.push_back( m );

  return true;
}

bool ArrayMathEngine::add_output_double_array( std::string name, std::vector< double >* array )
{
  std::string error_str;
  std::string tname = "__" + name;

  this->post_expression_ += tname + "=to_double_array(" + name + ");";

  // Add the variable to the parser
  if ( !( add_output_variable( this->pprogram_, name, "S", 0 ) ) )
  {
    CORE_LOG_ERROR( error_str );
    return false;
  }
  if ( !( add_output_variable( this->pprogram_, tname, "AB", 0 ) ) )
  {
    CORE_LOG_ERROR( error_str );
    return false;
  }

  array->resize( this->array_size_ );

  // Store information for processing after parsing has succeeded
  OutputDoubleArray m;
  m.array_name_ = name;
  m.double_array_name_ = tname;
  m.double_array_ = array;

  this->double_array_data_.push_back( m );

  return true;
}

// TODO Have Jeroen review this function
bool ArrayMathEngine::add_output_data_block( std::string name, DataBlockHandle input_data_block )
{
  std::string error_str;

  if ( this->array_size_ != input_data_block->get_size() && input_data_block->get_size() != 1 && 
    input_data_block->get_size() != 0 )
  {
    error_str = "The output field '" + name + 
      "' does not have the same number of elements as the other objects.";
    CORE_LOG_ERROR( error_str );
    return false;  
  }
  size_type size = input_data_block->get_size();

  std::string tname = "__" + name;

  if ( size == 0 )
  {
    // TODO How to handle size == 0?
  }

  int flags = 0;
  if ( size > 1 ) 
  {
    flags = SCRIPT_SEQUENTIAL_VAR_E;  
  }

  this->post_expression_ += tname + "=to_data_block(" + name + ");";

  // TODO Why add scalar variable?
  // Add the variable to the parser
  if ( !( add_output_variable( this->pprogram_, name, "S", flags ) ) )
  {
    CORE_LOG_ERROR( error_str );
    return false;
  }
  if ( !( add_output_variable( this->pprogram_, tname, "DATA", flags ) ) )
  {
    CORE_LOG_ERROR( error_str );
    return false;
  }

  // Create new output data block based on input data block
  Core::DataBlockHandle output_data_block( 
    Core::StdDataBlock::New( input_data_block->get_nx(), input_data_block->get_ny(), 
    input_data_block->get_nz(), input_data_block->get_data_type() ) );

  // Store information for processing after parsing has succeeded
  OutputDataBlock m;
  m.array_name_ = name;
  m.data_block_name_ = tname;
  m.data_block_ = output_data_block;

  this->data_block_data_.push_back( m );

  return true;
}

// TODO Have Jeroen review this function
bool ArrayMathEngine::add_output_mask_data_block( std::string name, 
  MaskDataBlockHandle input_mask_data_block )
{
  std::string error_str;

  if ( this->array_size_ != input_mask_data_block->get_size() && 
    input_mask_data_block->get_size() != 1 && input_mask_data_block->get_size() != 0 )
  {
    error_str = "The output field '" + name + 
      "' does not have the same number of elements as the other objects.";
    CORE_LOG_ERROR( error_str );
    return false;  
  }
  size_type size = input_mask_data_block->get_size();

  std::string tname = "__" + name;

  if ( size == 0 )
  {
    // TODO How to handle size == 0?
  }

  int flags = 0;
  if ( size > 1 ) 
  {
    flags = SCRIPT_SEQUENTIAL_VAR_E;  
  }

  this->post_expression_ += tname + "=to_mask_data_block(" + name + ");";

  // TODO Why add scalar variable?
  // Add the variable to the parser
  if ( !( add_output_variable( this->pprogram_, name, "S", flags ) ) )
  {
    CORE_LOG_ERROR( error_str );
    return false;
  }
  if ( !( add_output_variable( this->pprogram_, tname, "MASK", flags ) ) )
  {
    CORE_LOG_ERROR( error_str );
    return false;
  }

  // Create new output data block based on input data block
  Core::MaskDataBlockHandle output_mask_data_block( new Core::MaskDataBlock( 
    Core::StdDataBlock::New( input_mask_data_block->get_nx(), input_mask_data_block->get_ny(), 
    input_mask_data_block->get_nz(), DataType::UCHAR_E ), 
    input_mask_data_block->get_mask_bit() ) );

  // Store information for processing after parsing has succeeded
  OutputMaskDataBlock m;
  m.array_name_ = name;
  m.mask_data_block_name_ = tname;
  m.mask_data_block_ = output_mask_data_block;

  this->mask_data_block_data_.push_back( m );

  return true;
}


bool ArrayMathEngine::add_expressions( std::string& expressions )
{
  this->expression_ += expressions;
  return true;
}

bool ArrayMathEngine::run()
{
  std::string error_str;

  // Link everything together
  std::string full_expression = this->pre_expression_ + ";" + this->expression_ + ";" + 
    this->post_expression_;

  // Parse the full expression
  if ( !( this->parse( this->pprogram_, full_expression, error_str ) ) )
  {
    CORE_LOG_ERROR( error_str );
    return false;
  }

  // Get the catalog with all possible functions
  ParserFunctionCatalogHandle catalog = ArrayMathFunctionCatalog::get_catalog();

  // Validate the expressions
  if ( !( this->validate( this->pprogram_, catalog, error_str ) ) )
  {
    CORE_LOG_ERROR( error_str );
    return false;
  }

  // Optimize the expressions
  if ( !( this->optimize( this->pprogram_, error_str ) ) )
  {
    CORE_LOG_ERROR( error_str );
    return false;
  }

  // DEBUG CALL
#ifdef DEBUG
  this->pprogram_->print();
#endif

  for ( size_t j = 0; j < this->bool_array_data_.size(); j++ )
  {
    if ( this->bool_array_data_[ j ].bool_array_ )
    {
      std::string arrayname = this->bool_array_data_[ j ].bool_array_name_;
      if ( !( this->add_bool_array_sink( this->mprogram_, arrayname, 
        this->bool_array_data_[ j ].bool_array_, error_str ) ) )
      {
        CORE_LOG_ERROR( error_str );
        return false;
      }
    }
  }

  for ( size_t j = 0; j < this->double_array_data_.size(); j++ )
  {
    if ( this->double_array_data_[ j ].double_array_ )
    {
      std::string arrayname = this->double_array_data_[ j ].double_array_name_;
      if ( !( this->add_double_array_sink( this->mprogram_, arrayname,
          this->double_array_data_[ j ].double_array_, error_str ) ) )
      {
        CORE_LOG_ERROR( error_str );
        return false;
      }
    }
  }

  for ( size_t j = 0; j < this->data_block_data_.size(); j++ )
  {
    if ( this->data_block_data_[ j ].data_block_.get() )
    {
      std::string arrayname = this->data_block_data_[ j ].data_block_name_;
      if ( !( this->add_data_block_sink( this->mprogram_, arrayname,
        this->data_block_data_[ j ].data_block_, error_str ) ) )
      {
        CORE_LOG_ERROR( error_str );
        return false;
      }
    }
  }

  for ( size_t j = 0; j < this->mask_data_block_data_.size(); j++ )
  {
    if ( this->mask_data_block_data_[ j ].mask_data_block_.get() )
    {
      std::string arrayname = this->mask_data_block_data_[ j ].mask_data_block_name_;
      if ( !( this->add_mask_data_block_sink( this->mprogram_, arrayname,
        this->mask_data_block_data_[ j ].mask_data_block_, error_str ) ) )
      {
        CORE_LOG_ERROR( error_str );
        return false;
      }
    }
  }

  // Translate the code
  if ( !( this->translate( this->pprogram_, this->mprogram_, error_str ) ) )
  {
    CORE_LOG_ERROR( error_str );
    return false;
  }
  // Set the final array size
  if ( !( this->set_array_size( this->mprogram_, this->array_size_ ) ) )
  {
    CORE_LOG_ERROR( error_str );
    return false;
  }
  // Run the program
  if ( !( ArrayMathInterpreter::run( this->mprogram_, error_str ) ) )
  {
    CORE_LOG_ERROR( error_str );
    return false;
  }
  return true;
}

bool ArrayMathEngine::get_data_block( std::string name, DataBlockHandle& data_block )
{
  for ( size_t j = 0; j < this->data_block_data_.size(); j++ )
  {
    if ( this->data_block_data_[ j ].array_name_ == name )
    {
      data_block = this->data_block_data_[ j ].data_block_;
      return true;
    }
  }

  return false;
}

bool ArrayMathEngine::get_mask_data_block( std::string name, MaskDataBlockHandle& mask_data_block )
{
  for ( size_t j = 0; j < this->mask_data_block_data_.size(); j++ )
  {
    if ( this->mask_data_block_data_[ j ].array_name_ == name )
    {
      mask_data_block = this->mask_data_block_data_[ j ].mask_data_block_;
      return true;
    }
  }

  return false;
}

} // end namespace





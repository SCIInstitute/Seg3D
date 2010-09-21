// STL includes
#include <iostream>

// Core includes
#include <Core/DataBlock/DataBlock.h>
#include <Core/DataBlock/StdDataBlock.h>
#include <Core/Parser/ArrayMathEngine.h>
#include <Core/Parser/ArrayMathFunctionCatalog.h>
#include <Core/Parser/ParserProgram.h>


void print_vector( const std::vector< double >& array )
{
  std::vector< double >::const_iterator iter;  

  for( iter = array.begin(); iter != array.end(); iter++)  
  {  
    std::cout << *iter << std::endl;  
  }  
  std::cout << std::endl;  
}

void test_double_array()
{
  Core::ArrayMathEngine engine;
  
  // Create the DATA object for the function
  std::vector< double > input_array;
  input_array.push_back( 1 );
  input_array.push_back( 2 );
  input_array.push_back( 3 );
  input_array.push_back( 4 );

  std::cout << "Input array:" << std::endl;
  print_vector( input_array );

  if( !( engine.add_input_double_array( "A", &input_array ) ) ) return;

  std::string function = "RESULT = A + 1;"; // Test expression
  if ( !( function.find( "RESULT" ) != std::string::npos ) ) return;
  
  std::vector< double > output_array;
  if( !( engine.add_output_double_array( "RESULT", &output_array ) ) ) return;
  
  if( !( engine.add_expressions( function ) ) ) return;

  // Actual engine call, which does the dynamic compilation, the creation of the
  // code for all the objects, as well as inserting the function and looping 
  // over every data point

  if ( !( engine.run() ) ) return;
  
  std::cout << "Output array:" << std::endl;
  print_vector( output_array );
}

void print_data_block( Core::DataBlockHandle data_block )
{
  size_t nx = data_block->get_nx();
  size_t ny = data_block->get_ny();
  size_t nz = data_block->get_nz();
  for( size_t k = 0; k < nz; k++ )
  {
    for( size_t j = 0; j < ny; j++ )
    {
      for( size_t i = 0; i < nx; i++ )
      {
        double value = data_block->get_data_at( i, j, k );
        std::cout << value << std::endl;  
      }
    }
  }
  std::cout << std::endl;  
}

void test_data_block()
{
  Core::ArrayMathEngine engine;

  // Create the DATA object for the function
  size_t nx, ny, nz;
  nx = ny = nz = 2;
  Core::DataBlockHandle input_data_block( 
    Core::StdDataBlock::New( nx, ny, nz, Core::DataType::DOUBLE_E ) );
  double index = 0;
  for( size_t k = 0; k < nz; k++ )
  {
    for( size_t j = 0; j < ny; j++ )
    {
      for( size_t i = 0; i < nx; i++ )
      {
        input_data_block->set_data_at( i, j, k, index );
        index++;
      }
    }
  }

  std::cout << "Input data block" << std::endl;
  print_data_block( input_data_block );

  if( !( engine.add_input_data_block( "A", input_data_block ) ) ) return;

  std::string function = "RESULT = A * 2;"; // Test expression
  if ( !( function.find( "RESULT" ) != std::string::npos ) ) return;

  if( !( engine.add_output_data_block( "RESULT", input_data_block ) ) ) return;

  if( !( engine.add_expressions( function ) ) ) return;

  // Actual engine call, which does the dynamic compilation, the creation of the
  // code for all the objects, as well as inserting the function and looping 
  // over every data point

  if ( !( engine.run() ) ) return;

  Core::DataBlockHandle output_data_block;
  if( !engine.get_data_block( "RESULT", output_data_block ) ) return;

  std::cout << "Output data block" << std::endl;
  print_data_block( output_data_block );
}

// Test only parsing -- print parse tree
void test_parser()
{
  // Create parser program
  Core::ParserProgramHandle parser_program( new Core::ParserProgram() );

  // Create test expression
  std::string full_expression = "DATA=get_scalar(__DATA);v0=get_scalar(__v0);v=get_scalar(__v);POS=get_node_location(__POS);X=get_node_x(__X);Y=get_node_y(__X);Z=get_node_z(__X);x=get_node_x(__x);y=get_node_y(__x);z=get_node_z(__x);COUNT=get_scalar(__COUNT);RESULT=get_scalar(__RESULT);count=get_scalar(__count);result=get_scalar(__result);INDEX=index();SIZE=4096;;RESULT = sqrt(X*X+Y*Y+Z*Z);;__RESULT=to_fielddata(RESULT);";
  std::string error;

  // Pass program and expression to parser
  Core::Parser parser;
  if( !parser.parse( parser_program, full_expression, error ) ) return;

  // Print parser program
  parser_program->print();
}

int main()
{
  //test_parser();
  //test_double_array();
  test_data_block();

  std::cout << "Hello world" << std::endl;
}
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

// Core includes
#include <Core/Parser/ParserFunctionCatalog.h> 

namespace Core
{

void ParserFunctionCatalog::print()
{
  ParserFunctionList::iterator it, it_end;
  it = this->functions_.begin();
  it_end = this->functions_.end();

  std::cout << "FUNCTION CATALOG:\n";
  while ( it != it_end )
  {
    std::cout << "  " << ( *it ).first << "\n";
    ++it;
  }
}

void ParserFunctionCatalog::add_function( ParserFunction* function )
{
  ParserFunctionCatalog::lock_type lock( this->get_mutex() );
  std::string funid = function->get_function_id();
  this->functions_[ funid ] = function;
}

bool ParserFunctionCatalog::find_function( std::string fid, ParserFunction*& function )
{
  ParserFunctionList::iterator it = this->functions_.find( fid );
  if ( it == this->functions_.end() ) return false;

  function = ( *it ).second;
  return true;
}

ParserFunctionCatalog::ParserFunctionCatalog() 
{
}

}


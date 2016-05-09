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

#ifndef CORE_PARSER_PARSERFWD_H 
#define CORE_PARSER_PARSERFWD_H 

// STL includes
#include <map>

// Boost includes 
#include <boost/shared_ptr.hpp>

namespace Core
{
#ifdef SCI_64BITS
  typedef long long       index_type;
  typedef long long       size_type;
#else
  typedef int             index_type;
  typedef int             size_type;
#endif

  // Forward declarations 

  class DataBlock;
  typedef boost::shared_ptr< DataBlock > DataBlockHandle;

  class MaskDataBlock;
  typedef boost::shared_ptr< MaskDataBlock > MaskDataBlockHandle;

  /// Handles for a few of the classes. These are helper classes that are located elsewhere in 
  /// memory.

  class ArrayMathFunctionCatalog;
  typedef boost::shared_ptr< ArrayMathFunctionCatalog > ArrayMathFunctionCatalogHandle;

  class ArrayMathProgram;
  typedef boost::shared_ptr< ArrayMathProgram > ArrayMathProgramHandle;

  class ArrayMathProgramVariable;
  typedef boost::shared_ptr< ArrayMathProgramVariable > ArrayMathProgramVariableHandle;

  class ParserFunction;

  class ParserFunctionCatalog;
  typedef boost::shared_ptr< ParserFunctionCatalog > ParserFunctionCatalogHandle;

  class ParserNode;
  typedef boost::shared_ptr< ParserNode > ParserNodeHandle;

  class ParserProgram;
  typedef boost::shared_ptr< ParserProgram > ParserProgramHandle;

  class ParserScriptFunction;
  typedef boost::shared_ptr< ParserScriptFunction > ParserScriptFunctionHandle;

  class ParserScriptVariable;
  typedef boost::shared_ptr< ParserScriptVariable > ParserScriptVariableHandle;

  class ParserTree;
  typedef boost::shared_ptr< ParserTree > ParserTreeHandle;

  class ParserVariable;
  typedef boost::shared_ptr< ParserVariable > ParserVariableHandle;

  typedef std::map< std::string, ParserVariableHandle > ParserVariableList;
}

#endif

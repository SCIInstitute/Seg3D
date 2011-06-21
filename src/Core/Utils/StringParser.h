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

#ifndef CORE_CONVERTER_STRINGPARSER_H
#define CORE_CONVERTER_STRINGPARSER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

// STL includes
#include <string>

namespace Core
{

// SCANCOMMAND:
// Scan for the first command in the string str starting at start
// the function will return true if it success and other wise returns
// an error. It returns the next scan point in start

bool ScanCommand( const std::string& str, std::string::size_type& start, std::string& command,
    std::string& error );

// SCANVALUE:
// Scan for the first value in the string str starting at start
// the function will return true if it success and other wise returns
// an error. It returns the next scan point in start

bool ScanValue( const std::string& str, std::string::size_type& start, std::string& value,
    std::string& error );

// SCANKEYVALUEPAIR:
// Scan for the first key value pair in the string str starting at start
// the function will return true if it success and other wise returns
// an error. It returns the next scan point in start

bool ScanKeyValuePair( const std::string& str, std::string::size_type& start, std::string& key,
    std::string& value, std::string& error );

} // end namespace Core

#endif

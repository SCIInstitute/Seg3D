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

#ifndef MAIN_SEG3D_BASE_H
#define MAIN_SEG3D_BASE_H

#include <string>

namespace Seg3D
{

class Seg3DBase
{
  public:
    Seg3DBase();
    virtual ~Seg3DBase();

    bool information_only();
    bool initialize();
    virtual bool run() = 0;
    void close();

  protected:
    bool revision;
    bool version;
    bool help;
    std::string file_to_view;
    bool display_splash_screen;
    std::string python_script;
    bool start_sockets;
    int port_number;
    std::string program_name;

    virtual void warning(std::string& message) = 0;
    void print_usage();
    void check_32_bit();
    void initialize_python();
    void initialize_sockets();
};

} // namespace Seg3D
#endif // MAIN_SEG3D_BASE_H

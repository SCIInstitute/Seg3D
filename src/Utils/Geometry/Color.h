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

#ifndef UTILS_GEOMETRY_COLOR_H
#define UTILS_GEOMETRY_COLOR_H

namespace Utils {

// CLASS Color
// This class defines an rgb color

class Color {

// -- constructor/destructor --
  public:
    Color() :
      r_(0.0), g_(0.0), b_(0.0)
    {}
    
    Color(double r, double g, double b) :
      r_(r), g_(g), b_(b)
    {}
    
    Color(const Color& color) :
      r_(color.r_),
      g_(color.g_),
      b_(color.b_)
    {}

    ~Color()
    {}
    
    Color& operator=(const Color& color)
    {
      r_ = color.r_;
      g_ = color.g_;
      b_ = color.b_;
      
      return (*this);
    }

    inline Color operator*(double alpha) const
    {
      return (Color(r_*alpha,g_*alpha,b_*alpha));
    }

    inline int operator==(const Color& color) const 
    {
      return ((r_==color.r_)&&(g_==color.g_)&&(b_==color.b_));
    }

    inline int operator!=(const Color& color) const 
    {
      return ((r_ != color.r_)||(g_!=color.g_)||(b_!=color.b_));
    }

    inline double r() const {return r_;}
    inline double g() const {return g_;}
    inline double b() const {return b_;}

    inline void r( const double r ) { r_ = r; }
    inline void g( const double g ) { g_ = g; }
    inline void b( const double b ) { b_ = b; }

  private:
    // red, green, blue
    double r_;
    double g_;
    double b_;
};

Color operator*(double alpha, Color color);

} // End namespace Utils

#endif

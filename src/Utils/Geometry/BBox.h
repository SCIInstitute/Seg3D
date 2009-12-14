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

#ifndef UTILS_GEOMETRY_BBOX_H
#define UTILS_GEOMETRY_BBOX_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif 

// STL includes
#include <string>
#include <ostream>

// Utils includes
#include <Utils/Geometry/Point.h>
#include <Utils/Geometry/Vector.h>
#include <Utils/Math/MathFunctions.h>

namespace Utils {

class BBox {
  public:
    enum { INSIDE_E    = 1, 
           INTERSECT_E = 0, 
           OUTSIDE_E   = -1 };

    BBox() 
      : is_valid_(false) 
    {
    }
    
    BBox(const BBox& copy) 
      : cmin_(copy.cmin_), cmax_(copy.cmax_), is_valid_(copy.is_valid_) 
    {
    }
    
    BBox& operator=(const BBox& copy)
    {
      is_valid_ = copy.is_valid_;
      cmin_ = copy.cmin_;
      cmax_ = copy.cmax_;
      return *this;
    }

    BBox(const BBox& b1, const BBox& b2)
      : cmin_(b1.cmin_), cmax_(b1.cmax_), is_valid_(true) 
    {
      extend(b2.cmin_);
      extend(b2.cmax_);
    }

      
    BBox(const Point& p1, const Point& p2)
      : cmin_(p1), cmax_(p1), is_valid_(true) 
    {
      extend(p2);
    }

    BBox(const PointF& p1, const PointF& p2)
      : cmin_(p1), cmax_(p1), is_valid_(true) 
    {
      extend(p2);
    }

    BBox(const Point& p1, const Point& p2, const Point& p3)
      : cmin_(p1), cmax_(p1), is_valid_(true) 
    {
      extend(p2);
      extend(p3);
    }

    BBox(const PointF& p1, const PointF& p2, const PointF& p3)
      : cmin_(p1), cmax_(p1), is_valid_(true) 
    {
      extend(p2);
      extend(p3);
    }
        
    BBox(const std::vector<Point>& points) :
      is_valid_(false)
    {
      for (size_t j=0; j<points.size(); j++)
      {
        extend(points[j]);
      }
    }  

    BBox(const std::vector<PointF>& points) :
      is_valid_(false)
    {
      for (size_t j=0; j<points.size(); j++)
      {
        extend(points[j]);
      }
    }  
          
    bool valid() const 
    {
      return is_valid_; 
    }
    
    void clear() 
    { 
      cmin_ = Point(0.0,0.0,0.0);
      cmax_ = Point(0.0,0.0,0.0);
      is_valid_ = false; 
    }

    void extend(const Point& point)
    {
      if(is_valid_)
      {
        cmin_ = Min(point, cmin_);
        cmax_ = Max(point, cmax_);
      } 
      else 
      {
        cmin_ = point;
        cmax_ = point;
        is_valid_ = true;
      }
    }

    void extend(const PointF& point)
    {
      extend(Point(point));
    }

    // Extend the bounding box on all sides by a margin
    // For example to expand it by a certain epsilon to make
    // sure that a lookup will be inside the bounding box
    void extend(double width)
    {
      if (is_valid_)
      {
        cmin_ -= Vector(width,width,width);
        cmax_ += Vector(width,width,width);
      }
    }

    // Expand the bounding box to include a sphere of radius radius
    // and centered at point point
    inline void extend(const Point& point, double radius)
    {
      Vector rad(radius,radius,radius);
      if(is_valid_)
      {
        cmin_=Min(point-rad, cmin_);
        cmax_=Max(point+rad, cmax_);
      } 
      else 
      {
        cmin_=point-rad;
        cmax_=point+rad;
        is_valid_ = true;
      }
    }

    inline void extend(const PointF& point, float radius)
    {
      extend(Point(point),radius);
    }

    //! Expand the bounding box to include bounding box b
    inline void extend(const BBox& bbox)
    {
      if(bbox.valid())
      {
        extend(bbox.cmin_);
        extend(bbox.cmax_);
      }
    }
    
    //! Expand the bounding box to include a disk centered at cen,
    //! with normal normal, and radius r.
    void extend(const Point& center, const Vector& normal, double radius)
    {
      Vector n(normal); n.normalize();
      double x=Sqrt(1.0-n.x())*radius;
      double y=Sqrt(1.0-n.y())*radius;
      double z=Sqrt(1.0-n.z())*radius;
      extend(center+Vector(x,y,z));
      extend(center-Vector(x,y,z));
    }

    void extend(const PointF& center, const VectorF& normal, float radius)
    {
      extend(Point(center),Vector(normal),radius);
    }

    Point center() const  
    { 
      if (!is_valid_) return (Point(0.0,0.0,0.0));
      return (Point(0.5*(cmin_ + cmax_)));
    }
    
    double longest_edge() const
    {
      if (!is_valid_) return (false);
      Vector diagonal(cmax_-cmin_);
      return Max(diagonal.x(), diagonal.y(), diagonal.z());
    }

    double shortest_edge() const
    {
      if (!is_valid_) return (false);
      Vector diagonal(cmax_-cmin_);
      return Min(diagonal.x(), diagonal.y(), diagonal.z());
    }

    void translate(const Vector &vec)
    {
      cmin_+=vec;
      cmax_+=vec;
    }

    void translate(const VectorF &vec)
    {
      translate(Vector(vec));
    }
    
    void scale(double s, const Point &origin)
    {
      cmin_-=Vector(origin); cmax_-=Vector(origin);
      cmin_*=s;  cmax_*=s;
      cmin_+=origin; cmax_+=origin;
    }

    void scale(float s, const PointF &origin)
    {
      scale(s,Point(origin));
    }
    
    void scale(double scale)
    {
      Point origin = center();
      cmin_-=Vector(origin); cmax_-=Vector(origin);
      cmin_*=scale;  cmax_*=scale;
      cmin_+=origin; cmax_+=origin;
    }

    inline Point min() const
    { 
      return cmin_; 
    }
    
    inline Point max() const
    { 
      return cmax_; 
    }

    inline Vector diagonal() const
    { 
      return cmax_-cmin_; 
    }

    inline bool inside(const Point &p) const 
    {
      return (is_valid_ && p.x() >= cmin_.x() && 
        p.y() >= cmin_.y() && p.z() >= cmin_.z() && 
        p.x() <= cmax_.x() && p.y() <= cmax_.y() && 
        p.z() <= cmax_.z());
    }

    inline bool inside(const PointF &p) const 
    {
      return inside(Point(p));
    }

    inline int intersect(const BBox& b) const
    {
      if ((cmax_.x() < b.cmin_.x()) || (cmin_.x() > b.cmax_.x()) ||
          (cmax_.y() < b.cmin_.y()) || (cmin_.y() > b.cmax_.y()) ||
          (cmax_.z() < b.cmin_.z()) || (cmin_.z() > b.cmax_.z())) 
      {
        return OUTSIDE_E;
      }
      
      if ((cmin_.x() <= b.cmin_.x()) && (cmax_.x() >= b.cmax_.x()) &&
          (cmin_.y() <= b.cmin_.y()) && (cmax_.y() >= b.cmax_.y()) &&
          (cmin_.z() <= b.cmin_.z()) && (cmax_.z() >= b.cmax_.z())) 
      {
        return INSIDE_E;
      }
      
      return INTERSECT_E;    
    }

    inline double x_length() 
    { 
      return (cmax_.x() - cmin_.x()); 
    }
    
    inline double y_length() 
    { 
      return (cmax_.y() - cmin_.y()); 
    }
    
    inline double z_length() 
    { 
      return (cmax_.z() - cmin_.z()); 
    }

    // bbox's that share a face overlap
    bool overlaps(const BBox& bb) const;
    // bbox's that share a face do not overlap_inside
    bool overlaps_inside(const BBox& bb) const;

    friend std::ostream& operator<<(std::ostream& out, const BBox& bbox);

  private:
    Point cmin_;
    Point cmax_;
    bool is_valid_;
};

std::ostream& operator<<(std::ostream& out, const BBox& bbox);

} // End namespace Utils

#endif

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

/*
 *****************************************************************************
 *
 *   View.h
 *
 *   Also see: 
 *
 *   Authors:
 *      Kristen Zygmunt   -- initial attempt      11/10/2009
 *
 *    
 *****************************************************************************
 */

#ifndef APPLICATION_VIEW_VIEW_H
#define APPLICATION_VIEW_VIEW_H 1

//#ifdef (_MSC_VER) && (_MSC_VER >= 1020)
//# pragma once
//#endif

// STL includes
#include <string>
#include <vector>

// Boost includes 
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

// Application includes
#include <Utils/Stubs/Stubs.h>

namespace Seg3D {

// Forward declarations
class View;

// typedefs
typedef boost::shared_ptr<View> ViewHandle;
typedef std::vector<std::string> LayerStackType;

// Class declarations

class View {
  public:
    enum view_type {
      AXIAL_E = 0,
      SAGITTAL_E,
      CORONAL_E,
      VOLUME_E
    };

    View();
    ~View();

    // add a new layer to the view, if make_active is true,
    // this new layer will become the active layer.
    void add_layer(const std::string& layer_name, bool make_active, unsigned int opacity = 100);
    void remove_layer(const std::string& layer_name);
    
    std::string get_active_layer() const { return active_layer_; }
    // TODO: should any validation be done on the provided layer_name?
    void set_active_layer(const std::string& layer_name) { active_layer_ = layer_name; }

    LayerStackType get_layer_order() const { return layer_stack_; }
    // TODO: should any validation be done on the provided layer order?
    void set_layer_order(const LayerStackType& new_layer_order) { layer_stack_ = new_layer_order; }

    // redraw the view
    // TODO: need to define the OpenGLContext, Renderer etc and figure out how to associate it with this view
    void redraw() const;

    view_type get_type() const { return type_; }
    void set_type(view_type new_type) { type_ = new_type; }

    unsigned int get_opacity(const std::string& layer);
    // change the opacity.  Valid values are in the range 0 - 100
    void set_opacity(const std::string& layer, unsigned int new_opacity);

  private:
    view_type type_;
    // TODO: probably not the best type for internal storage if we want to insert layers into the middle
    LayerStackType layer_stack_;
    std::string active_layer_;
    boost::unordered_map<std::string, unsigned int> opacity_;
};

} // end namespace Seg3D

#endif

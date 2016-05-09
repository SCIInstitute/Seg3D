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

#ifndef APPLICATION_LAYER_LAYERFWD_H
#define APPLICATION_LAYER_LAYERFWD_H

#include <utility>
#include <string>
#include <vector>
#include <list>

// Boost includes 
#include <boost/shared_ptr.hpp>

namespace Seg3D
{

// Forward declarations 
class LayerGroup;
typedef boost::shared_ptr< LayerGroup > LayerGroupHandle;
typedef boost::weak_ptr< LayerGroup > LayerGroupWeakHandle;
  
class Layer;
typedef boost::shared_ptr< Layer > LayerHandle;
typedef boost::weak_ptr< Layer > LayerWeakHandle;
typedef std::list< LayerHandle > LayerList;
typedef std::vector< LayerHandle > LayerVector;

class DataLayer;
typedef boost::shared_ptr< DataLayer > DataLayerHandle;
class MaskLayer;
typedef boost::shared_ptr< MaskLayer > MaskLayerHandle;

typedef std::pair< std::string, std::string > LayerIDNamePair;

class LayerSceneItem;
typedef boost::shared_ptr< LayerSceneItem > LayerSceneItemHandle;
typedef std::vector< LayerSceneItemHandle > LayerScene;
typedef boost::shared_ptr< LayerScene > LayerSceneHandle;

} // end namespace Seg3D

#endif

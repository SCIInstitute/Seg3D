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

// File         : IRPruner.h
// Author       : Bradley C. Grimm
// Created      : 2009/05/15 11:52
// Copyright    : (C) 2009 University of Utah
// Description  : Used to prune uninteresting tiles.

#ifndef __IR_PRUNER_HXX__
#define __IR_PRUNER_HXX__

#include <string>

// boost:
#include <boost/filesystem.hpp>

namespace bfs=boost::filesystem;

class IRPruner
{
public:
  typedef std::list<bfs::path> ImageList;
  static void PruneImages(ImageList & image_names,
                          std::vector<base_transform_t::Pointer> &transform,
                          const unsigned int &shrink_factor,
                          const double &pixel_spacing,
                          const bool &use_std_mask,
                          const unsigned int &tile_size,
                          const double &intensity_tolerance,
                          float overlap_percent = 1.0f);
};

#endif



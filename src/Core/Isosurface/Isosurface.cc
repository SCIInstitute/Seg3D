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
#include <fstream>

// Boost includes
#include <boost/array.hpp>
 
// Core includes
#include <Core/DataBlock/StdDataBlock.h>
#include <Core/Isosurface/Isosurface.h>
#include <Core/Utils/StackVector.h>
#include <Core/Utils/Parallel.h>
#include <Core/Utils/Log.h>
#include <Core/Graphics/VertexBufferObject.h>
#include <Core/RenderResources/RenderResources.h>

namespace Core
{

// Marching Cubes tutorial: http://local.wasp.uwa.edu.au/~pbourke/geometry/polygonise/

typedef struct {
  int edges_[15]; // Vertex indices for at most 5 triangles
  int num_triangles_; // Last number in each table entry
} MarchingCubesTableType;

// Precalculated array of 256 possible polygon configurations (2^8 = 256) within the cube
// 256x16 table of integer values, which are used as indices for the array of 12 points of 
// intersection. It defines the right order to connect the intersected edges to form triangles. 
// The process for one cell stops when index of -1 is returned from the table, forming a maximum of 
// 5 triangles. 
const MarchingCubesTableType MARCHING_CUBES_TABLE_C[] = {
  {{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 0},
  {{ 0,  3,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 1},
  {{ 0,  9,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 1},
  {{ 1,  3,  8,  9,  1,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{ 1, 11,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 1},
  {{ 0,  3,  8,  1, 11,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{ 9, 11,  2,  0,  9,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{ 2,  3,  8,  2,  8, 11, 11,  8,  9, -1, -1, -1, -1, -1, -1}, 3},
  {{ 3,  2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 1},
  {{ 0,  2, 10,  8,  0, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{ 1,  0,  9,  2, 10,  3, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{ 1,  2, 10,  1, 10,  9,  9, 10,  8, -1, -1, -1, -1, -1, -1}, 3},
  {{ 3,  1, 11, 10,  3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{ 0,  1, 11,  0, 11,  8,  8, 11, 10, -1, -1, -1, -1, -1, -1}, 3},
  {{ 3,  0,  9,  3,  9, 10, 10,  9, 11, -1, -1, -1, -1, -1, -1}, 3},
  {{ 9, 11,  8, 11, 10,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{ 4,  8,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 1},
  {{ 4,  0,  3,  7,  4,  3, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{ 0,  9,  1,  8,  7,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{ 4,  9,  1,  4,  1,  7,  7,  1,  3, -1, -1, -1, -1, -1, -1}, 3},
  {{ 1, 11,  2,  8,  7,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{ 3,  7,  4,  3,  4,  0,  1, 11,  2, -1, -1, -1, -1, -1, -1}, 3},
  {{ 9, 11,  2,  9,  2,  0,  8,  7,  4, -1, -1, -1, -1, -1, -1}, 3},
  {{ 2,  9, 11,  2,  7,  9,  2,  3,  7,  7,  4,  9, -1, -1, -1}, 4},
  {{ 8,  7,  4,  3,  2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{10,  7,  4, 10,  4,  2,  2,  4,  0, -1, -1, -1, -1, -1, -1}, 3},
  {{ 9,  1,  0,  8,  7,  4,  2, 10,  3, -1, -1, -1, -1, -1, -1}, 3},
  {{ 4, 10,  7,  9, 10,  4,  9,  2, 10,  9,  1,  2, -1, -1, -1}, 4},
  {{ 3,  1, 11,  3, 11, 10,  7,  4,  8, -1, -1, -1, -1, -1, -1}, 3},
  {{ 1, 11, 10,  1, 10,  4,  1,  4,  0,  7,  4, 10, -1, -1, -1}, 4},
  {{ 4,  8,  7,  9, 10,  0,  9, 11, 10, 10,  3,  0, -1, -1, -1}, 4},
  {{ 4, 10,  7,  4,  9, 10,  9, 11, 10, -1, -1, -1, -1, -1, -1}, 3},
  {{ 9,  4,  5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 1},
  {{ 9,  4,  5,  0,  3,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{ 0,  4,  5,  1,  0,  5, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{ 8,  4,  5,  8,  5,  3,  3,  5,  1, -1, -1, -1, -1, -1, -1}, 3},
  {{ 1, 11,  2,  9,  4,  5, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{ 3,  8,  0,  1, 11,  2,  4,  5,  9, -1, -1, -1, -1, -1, -1}, 3},
  {{ 5, 11,  2,  5,  2,  4,  4,  2,  0, -1, -1, -1, -1, -1, -1}, 3},
  {{ 2,  5, 11,  3,  5,  2,  3,  4,  5,  3,  8,  4, -1, -1, -1}, 4},
  {{ 9,  4,  5,  2, 10,  3, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{ 0,  2, 10,  0, 10,  8,  4,  5,  9, -1, -1, -1, -1, -1, -1}, 3},
  {{ 0,  4,  5,  0,  5,  1,  2, 10,  3, -1, -1, -1, -1, -1, -1}, 3},
  {{ 2,  5,  1,  2,  8,  5,  2, 10,  8,  4,  5,  8, -1, -1, -1}, 4},
  {{11, 10,  3, 11,  3,  1,  9,  4,  5, -1, -1, -1, -1, -1, -1}, 3},
  {{ 4,  5,  9,  0,  1,  8,  8,  1, 11,  8, 11, 10, -1, -1, -1}, 4},
  {{ 5,  0,  4,  5, 10,  0,  5, 11, 10, 10,  3,  0, -1, -1, -1}, 4},
  {{ 5,  8,  4,  5, 11,  8, 11, 10,  8, -1, -1, -1, -1, -1, -1}, 3},
  {{ 9,  8,  7,  5,  9,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{ 9,  0,  3,  9,  3,  5,  5,  3,  7, -1, -1, -1, -1, -1, -1}, 3},
  {{ 0,  8,  7,  0,  7,  1,  1,  7,  5, -1, -1, -1, -1, -1, -1}, 3},
  {{ 1,  3,  5,  3,  7,  5, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{ 9,  8,  7,  9,  7,  5, 11,  2,  1, -1, -1, -1, -1, -1, -1}, 3},
  {{11,  2,  1,  9,  0,  5,  5,  0,  3,  5,  3,  7, -1, -1, -1}, 4},
  {{ 8,  2,  0,  8,  5,  2,  8,  7,  5, 11,  2,  5, -1, -1, -1}, 4},
  {{ 2,  5, 11,  2,  3,  5,  3,  7,  5, -1, -1, -1, -1, -1, -1}, 3},
  {{ 7,  5,  9,  7,  9,  8,  3,  2, 10, -1, -1, -1, -1, -1, -1}, 3},
  {{ 9,  7,  5,  9,  2,  7,  9,  0,  2,  2, 10,  7, -1, -1, -1}, 4},
  {{ 2, 10,  3,  0,  8,  1,  1,  8,  7,  1,  7,  5, -1, -1, -1}, 4},
  {{10,  1,  2, 10,  7,  1,  7,  5,  1, -1, -1, -1, -1, -1, -1}, 3},
  {{ 9,  8,  5,  8,  7,  5, 11,  3,  1, 11, 10,  3, -1, -1, -1}, 4},
  {{ 5,  0,  7,  5,  9,  0,  7,  0, 10,  1, 11,  0, 10,  0, 11}, 5},
  {{10,  0, 11, 10,  3,  0, 11,  0,  5,  8,  7,  0,  5,  0,  7}, 5},
  {{10,  5, 11,  7,  5, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{11,  5,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 1},
  {{ 0,  3,  8,  5,  6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{ 9,  1,  0,  5,  6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{ 1,  3,  8,  1,  8,  9,  5,  6, 11, -1, -1, -1, -1, -1, -1}, 3},
  {{ 1,  5,  6,  2,  1,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{ 1,  5,  6,  1,  6,  2,  3,  8,  0, -1, -1, -1, -1, -1, -1}, 3},
  {{ 9,  5,  6,  9,  6,  0,  0,  6,  2, -1, -1, -1, -1, -1, -1}, 3},
  {{ 5,  8,  9,  5,  2,  8,  5,  6,  2,  3,  8,  2, -1, -1, -1}, 4},
  {{ 2, 10,  3, 11,  5,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{10,  8,  0, 10,  0,  2, 11,  5,  6, -1, -1, -1, -1, -1, -1}, 3},
  {{ 0,  9,  1,  2, 10,  3,  5,  6, 11, -1, -1, -1, -1, -1, -1}, 3},
  {{ 5,  6, 11,  1,  2,  9,  9,  2, 10,  9, 10,  8, -1, -1, -1}, 4},
  {{ 6, 10,  3,  6,  3,  5,  5,  3,  1, -1, -1, -1, -1, -1, -1}, 3},
  {{ 0, 10,  8,  0,  5, 10,  0,  1,  5,  5,  6, 10, -1, -1, -1}, 4},
  {{ 3,  6, 10,  0,  6,  3,  0,  5,  6,  0,  9,  5, -1, -1, -1}, 4},
  {{ 6,  9,  5,  6, 10,  9, 10,  8,  9, -1, -1, -1, -1, -1, -1}, 3},
  {{ 5,  6, 11,  4,  8,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{ 4,  0,  3,  4,  3,  7,  6, 11,  5, -1, -1, -1, -1, -1, -1}, 3},
  {{ 1,  0,  9,  5,  6, 11,  8,  7,  4, -1, -1, -1, -1, -1, -1}, 3},
  {{11,  5,  6,  1,  7,  9,  1,  3,  7,  7,  4,  9, -1, -1, -1}, 4},
  {{ 6,  2,  1,  6,  1,  5,  4,  8,  7, -1, -1, -1, -1, -1, -1}, 3},
  {{ 1,  5,  2,  5,  6,  2,  3,  4,  0,  3,  7,  4, -1, -1, -1}, 4},
  {{ 8,  7,  4,  9,  5,  0,  0,  5,  6,  0,  6,  2, -1, -1, -1}, 4},
  {{ 7,  9,  3,  7,  4,  9,  3,  9,  2,  5,  6,  9,  2,  9,  6}, 5},
  {{ 3,  2, 10,  7,  4,  8, 11,  5,  6, -1, -1, -1, -1, -1, -1}, 3},
  {{ 5,  6, 11,  4,  2,  7,  4,  0,  2,  2, 10,  7, -1, -1, -1}, 4},
  {{ 0,  9,  1,  4,  8,  7,  2, 10,  3,  5,  6, 11, -1, -1, -1}, 4},
  {{ 9,  1,  2,  9,  2, 10,  9, 10,  4,  7,  4, 10,  5,  6, 11}, 5},
  {{ 8,  7,  4,  3,  5, 10,  3,  1,  5,  5,  6, 10, -1, -1, -1}, 4},
  {{ 5, 10,  1,  5,  6, 10,  1, 10,  0,  7,  4, 10,  0, 10,  4}, 5},
  {{ 0,  9,  5,  0,  5,  6,  0,  6,  3, 10,  3,  6,  8,  7,  4}, 5},
  {{ 6,  9,  5,  6, 10,  9,  4,  9,  7,  7,  9, 10, -1, -1, -1}, 4},
  {{11,  9,  4,  6, 11,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{ 4,  6, 11,  4, 11,  9,  0,  3,  8, -1, -1, -1, -1, -1, -1}, 3},
  {{11,  1,  0, 11,  0,  6,  6,  0,  4, -1, -1, -1, -1, -1, -1}, 3},
  {{ 8,  1,  3,  8,  6,  1,  8,  4,  6,  6, 11,  1, -1, -1, -1}, 4},
  {{ 1,  9,  4,  1,  4,  2,  2,  4,  6, -1, -1, -1, -1, -1, -1}, 3},
  {{ 3,  8,  0,  1,  9,  2,  2,  9,  4,  2,  4,  6, -1, -1, -1}, 4},
  {{ 0,  4,  2,  4,  6,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{ 8,  2,  3,  8,  4,  2,  4,  6,  2, -1, -1, -1, -1, -1, -1}, 3},
  {{11,  9,  4, 11,  4,  6, 10,  3,  2, -1, -1, -1, -1, -1, -1}, 3},
  {{ 0,  2,  8,  2, 10,  8,  4, 11,  9,  4,  6, 11, -1, -1, -1}, 4},
  {{ 3,  2, 10,  0,  6,  1,  0,  4,  6,  6, 11,  1, -1, -1, -1}, 4},
  {{ 6,  1,  4,  6, 11,  1,  4,  1,  8,  2, 10,  1,  8,  1, 10}, 5},
  {{ 9,  4,  6,  9,  6,  3,  9,  3,  1, 10,  3,  6, -1, -1, -1}, 4},
  {{ 8,  1, 10,  8,  0,  1, 10,  1,  6,  9,  4,  1,  6,  1,  4}, 5},
  {{ 3,  6, 10,  3,  0,  6,  0,  4,  6, -1, -1, -1, -1, -1, -1}, 3},
  {{ 6,  8,  4, 10,  8,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{ 7,  6, 11,  7, 11,  8,  8, 11,  9, -1, -1, -1, -1, -1, -1}, 3},
  {{ 0,  3,  7,  0,  7, 11,  0, 11,  9,  6, 11,  7, -1, -1, -1}, 4},
  {{11,  7,  6,  1,  7, 11,  1,  8,  7,  1,  0,  8, -1, -1, -1}, 4},
  {{11,  7,  6, 11,  1,  7,  1,  3,  7, -1, -1, -1, -1, -1, -1}, 3},
  {{ 1,  6,  2,  1,  8,  6,  1,  9,  8,  8,  7,  6, -1, -1, -1}, 4},
  {{ 2,  9,  6,  2,  1,  9,  6,  9,  7,  0,  3,  9,  7,  9,  3}, 5},
  {{ 7,  0,  8,  7,  6,  0,  6,  2,  0, -1, -1, -1, -1, -1, -1}, 3},
  {{ 7,  2,  3,  6,  2,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{ 2, 10,  3, 11,  8,  6, 11,  9,  8,  8,  7,  6, -1, -1, -1}, 4},
  {{ 2,  7,  0,  2, 10,  7,  0,  7,  9,  6, 11,  7,  9,  7, 11}, 5},
  {{ 1,  0,  8,  1,  8,  7,  1,  7, 11,  6, 11,  7,  2, 10,  3}, 5},
  {{10,  1,  2, 10,  7,  1, 11,  1,  6,  6,  1,  7, -1, -1, -1}, 4},
  {{ 8,  6,  9,  8,  7,  6,  9,  6,  1, 10,  3,  6,  1,  6,  3}, 5},
  {{ 0,  1,  9, 10,  7,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{ 7,  0,  8,  7,  6,  0,  3,  0, 10, 10,  0,  6, -1, -1, -1}, 4},
  {{ 7,  6, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 1},
  {{ 7, 10,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 1},
  {{ 3,  8,  0, 10,  6,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{ 0,  9,  1, 10,  6,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{ 8,  9,  1,  8,  1,  3, 10,  6,  7, -1, -1, -1, -1, -1, -1}, 3},
  {{11,  2,  1,  6,  7, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{ 1, 11,  2,  3,  8,  0,  6,  7, 10, -1, -1, -1, -1, -1, -1}, 3},
  {{ 2,  0,  9,  2,  9, 11,  6,  7, 10, -1, -1, -1, -1, -1, -1}, 3},
  {{ 6,  7, 10,  2,  3, 11, 11,  3,  8, 11,  8,  9, -1, -1, -1}, 4},
  {{ 7,  3,  2,  6,  7,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{ 7,  8,  0,  7,  0,  6,  6,  0,  2, -1, -1, -1, -1, -1, -1}, 3},
  {{ 2,  6,  7,  2,  7,  3,  0,  9,  1, -1, -1, -1, -1, -1, -1}, 3},
  {{ 1,  2,  6,  1,  6,  8,  1,  8,  9,  8,  6,  7, -1, -1, -1}, 4},
  {{11,  6,  7, 11,  7,  1,  1,  7,  3, -1, -1, -1, -1, -1, -1}, 3},
  {{11,  6,  7,  1, 11,  7,  1,  7,  8,  1,  8,  0, -1, -1, -1}, 4},
  {{ 0,  7,  3,  0, 11,  7,  0,  9, 11,  6,  7, 11, -1, -1, -1}, 4},
  {{ 7, 11,  6,  7,  8, 11,  8,  9, 11, -1, -1, -1, -1, -1, -1}, 3},
  {{ 6,  4,  8, 10,  6,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{ 3, 10,  6,  3,  6,  0,  0,  6,  4, -1, -1, -1, -1, -1, -1}, 3},
  {{ 8, 10,  6,  8,  6,  4,  9,  1,  0, -1, -1, -1, -1, -1, -1}, 3},
  {{ 9,  6,  4,  9,  3,  6,  9,  1,  3, 10,  6,  3, -1, -1, -1}, 4},
  {{ 6,  4,  8,  6,  8, 10,  2,  1, 11, -1, -1, -1, -1, -1, -1}, 3},
  {{ 1, 11,  2,  3, 10,  0,  0, 10,  6,  0,  6,  4, -1, -1, -1}, 4},
  {{ 4,  8, 10,  4, 10,  6,  0,  9,  2,  2,  9, 11, -1, -1, -1}, 4},
  {{11,  3,  9, 11,  2,  3,  9,  3,  4, 10,  6,  3,  4,  3,  6}, 5},
  {{ 8,  3,  2,  8,  2,  4,  4,  2,  6, -1, -1, -1, -1, -1, -1}, 3},
  {{ 0,  2,  4,  4,  2,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{ 1,  0,  9,  2,  4,  3,  2,  6,  4,  4,  8,  3, -1, -1, -1}, 4},
  {{ 1,  4,  9,  1,  2,  4,  2,  6,  4, -1, -1, -1, -1, -1, -1}, 3},
  {{ 8,  3,  1,  8,  1,  6,  8,  6,  4,  6,  1, 11, -1, -1, -1}, 4},
  {{11,  0,  1, 11,  6,  0,  6,  4,  0, -1, -1, -1, -1, -1, -1}, 3},
  {{ 4,  3,  6,  4,  8,  3,  6,  3, 11,  0,  9,  3, 11,  3,  9}, 5},
  {{11,  4,  9,  6,  4, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{ 4,  5,  9,  7, 10,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{ 0,  3,  8,  4,  5,  9, 10,  6,  7, -1, -1, -1, -1, -1, -1}, 3},
  {{ 5,  1,  0,  5,  0,  4,  7, 10,  6, -1, -1, -1, -1, -1, -1}, 3},
  {{10,  6,  7,  8,  4,  3,  3,  4,  5,  3,  5,  1, -1, -1, -1}, 4},
  {{ 9,  4,  5, 11,  2,  1,  7, 10,  6, -1, -1, -1, -1, -1, -1}, 3},
  {{ 6,  7, 10,  1, 11,  2,  0,  3,  8,  4,  5,  9, -1, -1, -1}, 4},
  {{ 7, 10,  6,  5, 11,  4,  4, 11,  2,  4,  2,  0, -1, -1, -1}, 4},
  {{ 3,  8,  4,  3,  4,  5,  3,  5,  2, 11,  2,  5, 10,  6,  7}, 5},
  {{ 7,  3,  2,  7,  2,  6,  5,  9,  4, -1, -1, -1, -1, -1, -1}, 3},
  {{ 9,  4,  5,  0,  6,  8,  0,  2,  6,  6,  7,  8, -1, -1, -1}, 4},
  {{ 3,  2,  6,  3,  6,  7,  1,  0,  5,  5,  0,  4, -1, -1, -1}, 4},
  {{ 6,  8,  2,  6,  7,  8,  2,  8,  1,  4,  5,  8,  1,  8,  5}, 5},
  {{ 9,  4,  5, 11,  6,  1,  1,  6,  7,  1,  7,  3, -1, -1, -1}, 4},
  {{ 1, 11,  6,  1,  6,  7,  1,  7,  0,  8,  0,  7,  9,  4,  5}, 5},
  {{ 4, 11,  0,  4,  5, 11,  0, 11,  3,  6,  7, 11,  3, 11,  7}, 5},
  {{ 7, 11,  6,  7,  8, 11,  5, 11,  4,  4, 11,  8, -1, -1, -1}, 4},
  {{ 6,  5,  9,  6,  9, 10, 10,  9,  8, -1, -1, -1, -1, -1, -1}, 3},
  {{ 3, 10,  6,  0,  3,  6,  0,  6,  5,  0,  5,  9, -1, -1, -1}, 4},
  {{ 0,  8, 10,  0, 10,  5,  0,  5,  1,  5, 10,  6, -1, -1, -1}, 4},
  {{ 6,  3, 10,  6,  5,  3,  5,  1,  3, -1, -1, -1, -1, -1, -1}, 3},
  {{ 1, 11,  2,  9, 10,  5,  9,  8, 10, 10,  6,  5, -1, -1, -1}, 4},
  {{ 0,  3, 10,  0, 10,  6,  0,  6,  9,  5,  9,  6,  1, 11,  2}, 5},
  {{10,  5,  8, 10,  6,  5,  8,  5,  0, 11,  2,  5,  0,  5,  2}, 5},
  {{ 6,  3, 10,  6,  5,  3,  2,  3, 11, 11,  3,  5, -1, -1, -1}, 4},
  {{ 5,  9,  8,  5,  8,  2,  5,  2,  6,  3,  2,  8, -1, -1, -1}, 4},
  {{ 9,  6,  5,  9,  0,  6,  0,  2,  6, -1, -1, -1, -1, -1, -1}, 3},
  {{ 1,  8,  5,  1,  0,  8,  5,  8,  6,  3,  2,  8,  6,  8,  2}, 5},
  {{ 1,  6,  5,  2,  6,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{ 1,  6,  3,  1, 11,  6,  3,  6,  8,  5,  9,  6,  8,  6,  9}, 5},
  {{11,  0,  1, 11,  6,  0,  9,  0,  5,  5,  0,  6, -1, -1, -1}, 4},
  {{ 0,  8,  3,  5, 11,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{11,  6,  5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 1},
  {{10, 11,  5,  7, 10,  5, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{10, 11,  5, 10,  5,  7,  8,  0,  3, -1, -1, -1, -1, -1, -1}, 3},
  {{ 5,  7, 10,  5, 10, 11,  1,  0,  9, -1, -1, -1, -1, -1, -1}, 3},
  {{11,  5,  7, 11,  7, 10,  9,  1,  8,  8,  1,  3, -1, -1, -1}, 4},
  {{10,  2,  1, 10,  1,  7,  7,  1,  5, -1, -1, -1, -1, -1, -1}, 3},
  {{ 0,  3,  8,  1,  7,  2,  1,  5,  7,  7, 10,  2, -1, -1, -1}, 4},
  {{ 9,  5,  7,  9,  7,  2,  9,  2,  0,  2,  7, 10, -1, -1, -1}, 4},
  {{ 7,  2,  5,  7, 10,  2,  5,  2,  9,  3,  8,  2,  9,  2,  8}, 5},
  {{ 2, 11,  5,  2,  5,  3,  3,  5,  7, -1, -1, -1, -1, -1, -1}, 3},
  {{ 8,  0,  2,  8,  2,  5,  8,  5,  7, 11,  5,  2, -1, -1, -1}, 4},
  {{ 9,  1,  0,  5,  3, 11,  5,  7,  3,  3,  2, 11, -1, -1, -1}, 4},
  {{ 9,  2,  8,  9,  1,  2,  8,  2,  7, 11,  5,  2,  7,  2,  5}, 5},
  {{ 1,  5,  3,  3,  5,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{ 0,  7,  8,  0,  1,  7,  1,  5,  7, -1, -1, -1, -1, -1, -1}, 3},
  {{ 9,  3,  0,  9,  5,  3,  5,  7,  3, -1, -1, -1, -1, -1, -1}, 3},
  {{ 9,  7,  8,  5,  7,  9, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{ 5,  4,  8,  5,  8, 11, 11,  8, 10, -1, -1, -1, -1, -1, -1}, 3},
  {{ 5,  4,  0,  5,  0, 10,  5, 10, 11, 10,  0,  3, -1, -1, -1}, 4},
  {{ 0,  9,  1,  8, 11,  4,  8, 10, 11, 11,  5,  4, -1, -1, -1}, 4},
  {{11,  4, 10, 11,  5,  4, 10,  4,  3,  9,  1,  4,  3,  4,  1}, 5},
  {{ 2,  1,  5,  2,  5,  8,  2,  8, 10,  4,  8,  5, -1, -1, -1}, 4},
  {{ 0, 10,  4,  0,  3, 10,  4, 10,  5,  2,  1, 10,  5, 10,  1}, 5},
  {{ 0,  5,  2,  0,  9,  5,  2,  5, 10,  4,  8,  5, 10,  5,  8}, 5},
  {{ 9,  5,  4,  2,  3, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{ 2, 11,  5,  3,  2,  5,  3,  5,  4,  3,  4,  8, -1, -1, -1}, 4},
  {{ 5,  2, 11,  5,  4,  2,  4,  0,  2, -1, -1, -1, -1, -1, -1}, 3},
  {{ 3,  2, 11,  3, 11,  5,  3,  5,  8,  4,  8,  5,  0,  9,  1}, 5},
  {{ 5,  2, 11,  5,  4,  2,  1,  2,  9,  9,  2,  4, -1, -1, -1}, 4},
  {{ 8,  5,  4,  8,  3,  5,  3,  1,  5, -1, -1, -1, -1, -1, -1}, 3},
  {{ 0,  5,  4,  1,  5,  0, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{ 8,  5,  4,  8,  3,  5,  9,  5,  0,  0,  5,  3, -1, -1, -1}, 4},
  {{ 9,  5,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 1},
  {{ 4,  7, 10,  4, 10,  9,  9, 10, 11, -1, -1, -1, -1, -1, -1}, 3},
  {{ 0,  3,  8,  4,  7,  9,  9,  7, 10,  9, 10, 11, -1, -1, -1}, 4},
  {{ 1, 10, 11,  1,  4, 10,  1,  0,  4,  7, 10,  4, -1, -1, -1}, 4},
  {{ 3,  4,  1,  3,  8,  4,  1,  4, 11,  7, 10,  4, 11,  4, 10}, 5},
  {{ 4,  7, 10,  9,  4, 10,  9, 10,  2,  9,  2,  1, -1, -1, -1}, 4},
  {{ 9,  4,  7,  9,  7, 10,  9, 10,  1,  2,  1, 10,  0,  3,  8}, 5},
  {{10,  4,  7, 10,  2,  4,  2,  0,  4, -1, -1, -1, -1, -1, -1}, 3},
  {{10,  4,  7, 10,  2,  4,  8,  4,  3,  3,  4,  2, -1, -1, -1}, 4},
  {{ 2, 11,  9,  2,  9,  7,  2,  7,  3,  7,  9,  4, -1, -1, -1}, 4},
  {{ 9,  7, 11,  9,  4,  7, 11,  7,  2,  8,  0,  7,  2,  7,  0}, 5},
  {{ 3, 11,  7,  3,  2, 11,  7, 11,  4,  1,  0, 11,  4, 11,  0}, 5},
  {{ 1,  2, 11,  8,  4,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{ 4,  1,  9,  4,  7,  1,  7,  3,  1, -1, -1, -1, -1, -1, -1}, 3},
  {{ 4,  1,  9,  4,  7,  1,  0,  1,  8,  8,  1,  7, -1, -1, -1}, 4},
  {{ 4,  3,  0,  7,  3,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{ 4,  7,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 1},
  {{ 9,  8, 11, 11,  8, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{ 3,  9,  0,  3, 10,  9, 10, 11,  9, -1, -1, -1, -1, -1, -1}, 3},
  {{ 0, 11,  1,  0,  8, 11,  8, 10, 11, -1, -1, -1, -1, -1, -1}, 3},
  {{ 3, 11,  1, 10, 11,  3, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{ 1, 10,  2,  1,  9, 10,  9,  8, 10, -1, -1, -1, -1, -1, -1}, 3},
  {{ 3,  9,  0,  3, 10,  9,  1,  9,  2,  2,  9, 10, -1, -1, -1}, 4},
  {{ 0, 10,  2,  8, 10,  0, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{ 3, 10,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 1},
  {{ 2,  8,  3,  2, 11,  8, 11,  9,  8, -1, -1, -1, -1, -1, -1}, 3},
  {{ 9,  2, 11,  0,  2,  9, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{ 2,  8,  3,  2, 11,  8,  0,  8,  1,  1,  8, 11, -1, -1, -1}, 4},
  {{ 1,  2, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 1},
  {{ 1,  8,  3,  9,  8,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 2},
  {{ 0,  1,  9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 1},
  {{ 0,  8,  3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 1},
  {{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 0}};

typedef struct {
  int points_[12]; // Vertex indices for at most 4 triangles
  int num_triangles_; // Last number in each table entry
} CappingTableType;

// Precalculated array of 16 possible polygon configurations (2^4 = 16) within the cell.
// 16x13 table of integer values, which are used as indices for the array of 8 points (4 vertices, 4 edges) of 
// intersection. It defines the right order to connect the intersected edges to form triangles. 
// The process for one cell stops when index of -1 is returned from the table, forming a maximum of 
// 4 triangles. 
// For example: {{tri1.p1, tri1.p2, tri1.p3, tri2.p1, tri2.p2, tri2.p3, tri3.p1, tri3.p2, tri3.p3, 
// tri4.p1, tri4,p2, tri4.p3}, num_tri}
const CappingTableType CAPPING_TABLE_C[] = {
  {{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 0},  // 0
  {{0, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 1},   // 1
  {{4, 1, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 1},   // 2
  {{0, 1, 7, 7, 1, 5, -1, -1, -1, -1, -1, -1}, 2},    // 3
  {{6, 5, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 1},   // 4
  {{0, 4, 7, 7, 4, 5, 7, 5, 6, 6, 5, 2}, 4},        // 5
  {{4, 1, 6, 6, 1, 2, -1, -1, -1, -1, -1, -1}, 2},    // 6
  {{0, 6, 7, 0, 1, 6, 6, 1, 2, -1, -1, -1}, 3},     // 7
  {{7, 6, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 1},   // 8 
  {{0, 4, 3, 3, 4, 6, -1, -1, -1, -1, -1, -1}, 2},    // 9
  {{7, 6, 3, 7, 4, 6, 4, 5, 6, 4, 1, 5}, 4},        // 10
  {{0, 6, 3, 0, 5, 6, 0, 1, 5, -1, -1, -1}, 3},     // 11
  {{7, 5, 3, 3, 5, 2, -1, -1, -1, -1, -1, -1}, 2},    // 12
  {{0, 5, 3, 0, 4, 5, 3, 5, 2, -1, -1, -1}, 3},     // 13
  {{7, 4, 3, 3, 4, 2, 4, 1, 2, -1, -1, -1}, 3},     // 14
  {{0, 1, 3, 3, 1, 2, -1, -1, -1, -1, -1, -1}, 2}};   // 15

class VertexBufferBatch
{
public:
  VertexAttribArrayBufferHandle vertex_buffer_;
  VertexAttribArrayBufferHandle normal_buffer_;
  VertexAttribArrayBufferHandle value_buffer_;
  ElementArrayBufferHandle faces_buffer_;
};
typedef boost::shared_ptr< VertexBufferBatch > VertexBufferBatchHandle;

  const std::string Isosurface::EXPORT_FORMATS_C( "VTK (*.vtk);;ASCII (*.fac *.pts *.val);;STL (*.stl)" );

class IsosurfacePrivate 
{

public:
  void downsample_setup( int num_threads, double quality_factor );

  // PARALLEL_DOWNSAMPLE:
  // Downsample mask prior to computing the isosurface in order to reduce the mesh to speed up
  // rendering.
  void parallel_downsample_mask( int thread, int num_threads, boost::barrier& barrier, 
    double quality_factor );

  // Copy values to members just to simplify and shorten code.  Must be called after downsample
  // and before face computation.
  void compute_setup();

  // SETUP:
  // Setup the algorithm and the buffers for face computation
  void compute_faces_setup( int num_threads );

  // PARALLEL_COMPUTE_FACES:
  // Parallelized isosurface computation algorithm 
  void parallel_compute_faces( int thread, int num_threads, boost::barrier& barrier );

  void translate_cap_coords( int cap_num, float i, float j, float& x, float& y, float& z );

  size_t get_data_index( float x, float y, float z );

  // COMPUTE_CAP_FACES:
  // Compute the "cap" faces at the boundary of the mask volume to handle the case where the 
  // mask goes all the way to the boundary.  Otherwise, we end up with holes in the 
  // isosurface at the boundary.  The cap faces are computed as separate geometry so that they can 
  // be turned on/off independently from the rest of the isosurface.
  void compute_cap_faces();

  // PARALLEL_COMPUTE_NORMALS:
  // Parallelized isosurface normal computation algorithm 
  void parallel_compute_normals( int thread, int num_threads,  boost::barrier& barrier );

  // UPLOAD_TO_VERTEX_BUFFER:
  void upload_to_vertex_buffer();

  void reset();
  

  // Pointer to public Isosurface -- needed to give access to public signals
  Isosurface* isosurface_;

  // Downsample params
  MaskVolumeHandle downsample_mask_volume_;
  int neighborhood_size_;
  size_t zsize_;
  size_t total_neighborhoods_;

  // Input to isosurface computation, not downsampled
  MaskVolumeHandle orig_mask_volume_; 
  // Mask volume to be used for isosurface computation.  May point to original volume or 
  // downsampled volume.
  MaskVolumeHandle compute_mask_volume_; 
  unsigned char mask_value_; // Same for original volume and downsampled volume

  // Output mesh
  std::vector< PointF > points_; 
  std::vector< VectorF > normals_; 
  std::vector< unsigned int > faces_; // unsigned int because GL expects this
  std::vector< float > values_; // Should be in range [0, 1]
  float area_; // Surface area of the isosurface

  // Single colormap shared by all isosurfaces
  ColorMapHandle color_map_;

  // Algorithm data & buffers
  unsigned char* data_; // Mask data is stored in bit-plane (8 masks per data block)
  size_t nx_, ny_, nz_; // Mask dimensions of original or downsampled volume depending on quality
  size_t elem_nx_, elem_ny_, elem_nz_; // Number of (marching) cubes

  std::vector<unsigned char> type_buffer_; 
  std::vector<std::vector<unsigned int> > edge_buffer_;  

  std::vector<unsigned int> min_point_index_;
  std::vector<unsigned int> max_point_index_;
  std::vector<unsigned int> min_face_index_;
  std::vector<unsigned int> max_face_index_;

  std::vector< std::pair<unsigned int, unsigned int> > part_points_;
  std::vector< std::pair<unsigned int, unsigned int> > part_faces_;
  std::vector< std::vector< unsigned int > > part_indices_;

  std::vector< std::vector< PointF > > new_points_; 
  std::vector< std::vector< StackVector< size_t, 3 > > > new_elems_;
  std::vector< float > new_elem_areas_;

  std::vector< size_t > front_offset_;
  std::vector< size_t > back_offset_;
  size_t global_point_cnt_;

  unsigned int prev_point_min_;
  unsigned int prev_point_max_;

  std::vector< VertexBufferBatchHandle > vbo_batches_;
  bool vbo_available_;
  bool surface_changed_;
  bool values_changed_;

  bool need_abort_;
  boost::function< bool () > check_abort_;

  const static double COMPUTE_PERCENT_PROGRESS_C;
  const static double NORMAL_PERCENT_PROGRESS_C;
  const static double PARTITION_PERCENT_PROGRESS_C;
};

// Initialize static variables
const double IsosurfacePrivate::COMPUTE_PERCENT_PROGRESS_C = 0.8;
const double IsosurfacePrivate::NORMAL_PERCENT_PROGRESS_C = 0.05;
const double IsosurfacePrivate::PARTITION_PERCENT_PROGRESS_C = 0.15; 

void IsosurfacePrivate::downsample_setup( int num_threads, double quality_factor )
{
  this->nx_ = this->orig_mask_volume_->get_mask_data_block()->get_nx();
  this->ny_ = this->orig_mask_volume_->get_mask_data_block()->get_ny();
  this->nz_ = this->orig_mask_volume_->get_mask_data_block()->get_nz();

  // Mask data is stored in bit-plane (8 masks per data block)
  this->data_ = this->orig_mask_volume_->get_mask_data_block()->get_mask_data();

  // Bit where mask bit is stored
  this->mask_value_ = this->orig_mask_volume_->get_mask_data_block()->get_mask_value();

  // Create downsampled mask to store results 
  this->neighborhood_size_ = static_cast< int >( 1.0 / quality_factor );
  size_t downsampled_nx = this->nx_ / this->neighborhood_size_;
  size_t downsampled_ny = this->ny_ / this->neighborhood_size_;
  size_t downsampled_nz = this->nz_ / this->neighborhood_size_;

  // Normally MaskDataBlocks should be registered with the MathDataBlockManager, but in this
  // case we are only using this as a temporary object and do not want to share the mask with
  // other masks since we would then have to carefully lock/unlock it during use.
  MaskDataBlockHandle mask_data_block( new MaskDataBlock( 
    StdDataBlock::New( downsampled_nx, downsampled_ny, downsampled_nz, DataType::UCHAR_E ), 
    this->orig_mask_volume_->get_mask_data_block()->get_mask_bit() ) );

  // Downsampled mask needs to be scaled up to fill the same geometric space as the original
  // mask.
  GridTransform grid_transform = this->orig_mask_volume_->get_grid_transform();
  double transform_scale = static_cast< double >( this->neighborhood_size_ );
  grid_transform.post_scale( Vector( transform_scale, transform_scale, transform_scale ) );

  this->downsample_mask_volume_ = MaskVolumeHandle( new MaskVolume( grid_transform, 
    mask_data_block ) );

  // Point to downsampled mask rather than original mask
  this->compute_mask_volume_ = this->downsample_mask_volume_;

  // For parallelization, divide mask into slabs along z.  No synchronization is needed.
  this->zsize_ = static_cast< size_t >( this->nz_ / num_threads );
  // Make sure this zsize will cover all the data
  if ( this->zsize_ * num_threads < this->nz_ ) 
  {
    this->zsize_++;
  }
  int remainder = this->zsize_ % this->neighborhood_size_;
  // Round up to the next neighborhood increment -- leaves least work for last thread
  if( remainder != 0 )
  {
    this->zsize_ += ( this->neighborhood_size_ - remainder );
  }

  size_t x_neighborhoods = this->nx_ / this->neighborhood_size_;
  size_t y_neighborhoods = this->ny_ / this->neighborhood_size_;
  size_t z_neighborhoods = this->zsize_ / this->neighborhood_size_;
  this->total_neighborhoods_ = x_neighborhoods * y_neighborhoods * z_neighborhoods;
}

/*
Allow downsampling by only half, quarter, and eighth.  When downsampling by half, a 2x2x2 
neighborhood of nodes is downsampled to a single node.  If at least one neighborhood node is "on", 
result is "on".  This method was chosen to prevent holes in the downsampled data.
*/
void IsosurfacePrivate::parallel_downsample_mask( int thread, int num_threads, 
  boost::barrier& barrier, double quality_factor )
{
  // Only need to setup once 
  if( thread == 0 )
  {
    this->downsample_setup( num_threads, quality_factor );
  }

  // All threads must wait for setup to complete
  barrier.wait();

  // Different thread process slabs along the z axis.  Each slab contains one or more 
  // neighborhoods to be downsampled.
  // [nzstart, nzend) 
  size_t nzstart = thread * this->zsize_;
  size_t nzend = ( thread + 1 ) * this->zsize_;
  if ( nzend > this->nz_ ) 
  {
    nzend = this->nz_;
  }

  unsigned char* downsampled_data = 
    this->downsample_mask_volume_->get_mask_data_block()->get_mask_data();

  size_t z_offset = this->nx_ * this->ny_;
  unsigned char not_mask_value = ~( this->mask_value_ );

  size_t target_index = thread * this->total_neighborhoods_;
  size_t x_start_end = this->nx_ - this->neighborhood_size_ + 1;
  size_t y_start_end = this->ny_ - this->neighborhood_size_ + 1;
  size_t z_start_end = nzend - this->neighborhood_size_ + 1;

  // Loop over neighborhoods, chop off border values
  for ( size_t z_start = nzstart; z_start < z_start_end; z_start += this->neighborhood_size_ ) 
  {
    for ( size_t y_start = 0; y_start < y_start_end; y_start += this->neighborhood_size_ ) 
    {
      for ( size_t x_start = 0; x_start < x_start_end; x_start += this->neighborhood_size_ ) 
      {
        // Clear entry initially
        downsampled_data[ target_index ] &= not_mask_value;
        
        bool stop = false;
        size_t x_end = x_start + this->neighborhood_size_;
        size_t y_end = y_start + this->neighborhood_size_;
        size_t z_end = z_start + this->neighborhood_size_;

        // Loop over neighbors based on corner index
        for( size_t z = z_start; z < z_end && !stop; z++ )
        {
          for( size_t y = y_start; y < y_end && !stop; y++ )
          {
            for( size_t x = x_start; x < x_end && !stop; x++ )
            {
              size_t index = ( ( z_offset ) * z ) + ( this->nx_ * y ) + x;  

              // If at least one neighborhood node is "on", result is "on"
              if ( this->data_[ index ] & this->mask_value_ ) // Node "on"    
              {
                // Turn on mask value
                downsampled_data[ target_index ] |= this->mask_value_;
                // Short-circuit
                stop = true;
              }
            }
          }
        }
        target_index++;
      }
    }
  }
}

void IsosurfacePrivate::compute_setup()
{
  this->nx_ = this->compute_mask_volume_->get_mask_data_block()->get_nx();
  this->ny_ = this->compute_mask_volume_->get_mask_data_block()->get_ny();
  this->nz_ = this->compute_mask_volume_->get_mask_data_block()->get_nz();

  // Mask data is stored in bit-plane (8 masks per data block)
  this->data_ = this->compute_mask_volume_->get_mask_data_block()->get_mask_data();

  // Bit where mask bit is stored
  this->mask_value_ = this->compute_mask_volume_->get_mask_data_block()->get_mask_value();
}

void IsosurfacePrivate::compute_faces_setup( int num_threads )
{
  // Number of elements (cubes) in each dimension
  this->elem_nx_ = this->nx_ - 1;
  this->elem_ny_ = this->ny_ - 1;
  this->elem_nz_ = this->nz_ - 1;

  // Stores index into polygon configuration table for each element (cube)?
  // Why +1?  Maybe just padding for safety?
  this->type_buffer_.resize( ( this->nx_ + 1 ) * ( this->ny_ + 1 ) );
  // For each element (cube), holds edge ID (12 edges) back_buffer_x, back_buffer_y, 
  // front_buffer_x, front_buffer_y, and side_buffer 
  this->edge_buffer_.resize( 5 ); 
  for ( size_t q = 0; q < 5; q++ )
  {
    this->edge_buffer_[ q ].resize( ( this->nx_ + 1 ) * ( this->ny_ + 1 ) );
  }

  // Interpolated edge points for isosurface per thread
  this->new_points_.resize( num_threads );

  // Vector of face indices per triangle, per thread
  this->new_elems_.resize( num_threads );
  // Surface areas of generated triangles per thread
  this->new_elem_areas_.resize( num_threads, 0 );
  // Offset allows zero-based indexing over multiple slices
  this->front_offset_.resize( num_threads, 0 );
  this->back_offset_.resize( num_threads, 0 ); 

  // Total number of isosurface points
  this->global_point_cnt_ = 0;

  this->min_point_index_.resize( this->elem_nz_ );
  this->max_point_index_.resize( this->elem_nz_ );
  this->min_face_index_.resize( this->elem_nz_ );
  this->max_face_index_.resize( this->elem_nz_ );

  this->prev_point_min_ = 0;
  this->prev_point_max_ = 0;
  
  this->need_abort_ = false;

}

/*
Basic ideas:
- Move through volume two slices at a time along z axis.  Back and front refer to these two slices.
- Points are shared by multiple triangles in the isosurface.  We don't want to store a copy of a
  point for each triangle.  In order to avoid duplicates, we go through edges in one direction at a 
  time, looking at edges that need to be split.  This way we encounter each edge only once.
- Sort edges into 5 configurations
  - back_buffer_x - Edges along the x direction on the back buffer (slice) 
  - back_buffer_y - Edges along the y direction on the back buffer (slice)
  - front_buffer_x - Edges along the x direction on the front buffer (slice)
  - front_buffer_y - Edges along the y direction on the front buffer (slice)
  - side_buffer - Edges along the sides between the back and front buffers (slices)
- These are tables of split edges with indices into a points vector of actual points.
- Number points as you encounter split edges.
- After edge tables are built, go back to type list, use configurations to lookup into the tables.
- Tables are built in parallel using relative indices per thread.
- Parallel point lists are merged together at the end.
- At end, swap front and back data (front is now back).
- One advantage of this approach is that we don't need complex and confusing linked lists; we can
  use tables that directly correspond to elements.
- Point of confusion: sometimes "element" is synonymous with "cube" and sometimes it refers
  to a triangle.
*/
void IsosurfacePrivate::parallel_compute_faces( int thread, int num_threads, 
  boost::barrier& barrier )
{
  // Setup the algorithm and the buffers
  if ( thread == 0 ) // Only need to setup once 
  {
    this->compute_faces_setup( num_threads ); 
  }

  // An object of class barrier is a synchronization primitive used to cause a set of threads 
  // to wait until they each perform a certain function or each reach a particular point in their 
  // execution.  This mechanism proves useful in situations where processing progresses in 
  // stages and completion of the current stage by all threads is the entry criteria for the next 
  // stage.
  barrier.wait();

  // In order to parallelize the algorithm, each thread processes a different horizontal strip
  // of data
  // Determine the element (cube) range for each thread in the y dimension
  size_t ysize = static_cast< size_t >( this->ny_ / num_threads );
  if ( ysize * num_threads < this->ny_ ) 
  {
    ysize++;
  }

  size_t nystart = thread * ysize;
  size_t nyend = ( thread + 1 ) * ysize;
  if ( nyend > this->ny_ ) 
  {
    nyend = this->ny_;
  }

  size_t elem_ysize = static_cast< size_t >( this->elem_ny_ / num_threads );
  if ( elem_ysize * num_threads < this->elem_ny_ ) 
  {
    elem_ysize++;
  }

  size_t elem_nystart = thread * elem_ysize;
  size_t elem_nyend = ( thread + 1 ) * elem_ysize;
  if ( elem_nyend > this->elem_ny_ ) 
  {
    elem_nyend = this->elem_ny_;
  }

  // Each thread generates new points for the isosurface
  std::vector< PointF >& points = this->new_points_[ thread ];
  // StackVector is a SCIRun class.  
  // StackVector implements a subclass of the std::vector class, except that
  // the vector is statically allocated on the stack for performance.
  // elems = triangles
  std::vector< StackVector < size_t, 3 > >& elements = this->new_elems_[ thread ];

  // mark the point counter with the process number
  unsigned int point_cnt = 0;

  // Vector of pointers into edge_buffer_ for each of the 12 edges.  edge_buffer_ has indices into
  // vector of actual points.
  std::vector< unsigned int* > edge_table( 12 );

  barrier.wait();

  // See function description
  int back_buffer_x = 0;
  int back_buffer_y = 1;
  int front_buffer_x = 2;
  int front_buffer_y = 3;
  int side_buffer = 4;

  StackVector< size_t, 3 > elems( 3 );    

  // Get mask transform from MaskVolume 
  GridTransform grid_transform = this->compute_mask_volume_->get_grid_transform();

  // Loop over all the slices
  for ( size_t z = 0;  z < this->elem_nz_; z++ ) 
  {
    // Process two adjacent slices at a time (back and front)
    // Get pointer to beginning of each slice in the data
    unsigned char* data1 = this->data_ + z * ( this->nx_ * this->ny_ );
    unsigned char* data2 = this->data_ + ( z + 1 ) * ( this->nx_ * this->ny_ );

    points.clear();
    point_cnt = thread<<24; // upper 8 bits are used to store thread id -- efficiency trick

    // References to back/front/side tables
    std::vector< unsigned int >& back_edge_x = this->edge_buffer_[ back_buffer_x ];
    std::vector< unsigned int >& back_edge_y = this->edge_buffer_[ back_buffer_y ];

    std::vector< unsigned int >& front_edge_x = this->edge_buffer_[ front_buffer_x ];
    std::vector< unsigned int >& front_edge_y = this->edge_buffer_[ front_buffer_y ];

    std::vector< unsigned int >& side_edge = this->edge_buffer_[ side_buffer ];

    // Use relative offsets to find edges
    edge_table[ 0 ] = &( this->edge_buffer_[ back_buffer_x ][ 0 ] );
    edge_table[ 1 ] = &( this->edge_buffer_[ back_buffer_y ][ 1 ] );
    edge_table[ 2 ] = &( this->edge_buffer_[ back_buffer_x ][ this->nx_ ] );
    edge_table[ 3 ] = &( this->edge_buffer_[ back_buffer_y ][ 0 ] );

    edge_table[ 4 ] = &( this->edge_buffer_[ front_buffer_x ][ 0 ] );
    edge_table[ 5 ] = &( this->edge_buffer_[ front_buffer_y ][ 1 ] );
    edge_table[ 6 ] = &( this->edge_buffer_[ front_buffer_x ][ this->nx_ ] );
    edge_table[ 7 ] = &( this->edge_buffer_[ front_buffer_y ][ 0 ] );

    edge_table[ 8 ] = &( this->edge_buffer_[ side_buffer ][ 0 ] );
    edge_table[ 9 ] = &( this->edge_buffer_[ side_buffer ][ 1 ] );
    edge_table[ 10 ] = &( this->edge_buffer_[ side_buffer ][ this->nx_ ] );
    edge_table[ 11 ] = &( this->edge_buffer_[ side_buffer ][ this->nx_ + 1 ] );

    // Step 1: determine the type of marching cube pattern (triangles) that needs
    // to go in each element (cube) and the intersecting points on each edge

    // Loop over horizontal strip of elements (cubes)
    for ( size_t y = nystart; y < nyend; y++ )
    {
      for ( size_t x = 0; x < this->nx_; x++ )
      {
        // There are dim - 1 elements (cubes)
        if ( x < this->elem_nx_ && y < this->elem_ny_ )
        {
          // type = index into polygonal configuration table
          unsigned char type = 0;
          size_t q = y * this->nx_ + x; // Index into data
          // An 8 bit index is formed where each bit corresponds to a vertex 
          // Bit on if vertex is inside surface, off otherwise
          if ( data1[ q ] & this->mask_value_ )         type |= 0x1;
          if ( data1[ q + 1 ] & this->mask_value_ )       type |= 0x2;
          if ( data1[ q + this->nx_ + 1 ] & this->mask_value_ ) type |= 0x4;
          if ( data1[ q + this->nx_ ] & this->mask_value_ )   type |= 0x8;

          if ( data2[ q ] & this->mask_value_ )         type |= 0x10;
          if ( data2[ q + 1 ] & this->mask_value_ )       type |= 0x20;
          if ( data2[ q + this->nx_ + 1 ] & this->mask_value_ ) type |= 0x40;
          if ( data2[ q + this->nx_ ] & this->mask_value_ )   type |= 0x80;

          this->type_buffer_[ q ] = type;

          // All points are inside or outside the cube -- does not contribute to the 
          // isosurface 
          if (type == 0x00 || type == 0xFF ) 
          {
            continue;
          }

          // Since mask values are either on or off, no need to interpolate 
          // between vertices along edges.  Always put point in center of edge.
          const float INTERP_EDGE_OFFSET_C = 0.5f;

          if ( z == 0 )
          {
            // top border and center ones
            if ( ( ( type>>0 ) ^ ( type>>1 ) ) & 0x01 ) 
            {
              PointF edge_point = PointF( static_cast< float >( x ) + 
                INTERP_EDGE_OFFSET_C,
                static_cast< float >( y ),
                static_cast< float >( z ) );

              // Transform point by mask transform
              edge_point = grid_transform.project( edge_point );

              points.push_back( edge_point );
              back_edge_x[ q ] = point_cnt;
              point_cnt++;
            }

            // bottom border one
            if ( ( y == this->elem_ny_ - 1 ) && ( ( ( type>>2 ) ^ ( type>>3 ) ) & 0x01 ) )
            {
              PointF edge_point = PointF( static_cast< float >( x ) + 
                INTERP_EDGE_OFFSET_C,
                static_cast< float>( y + 1 ),
                static_cast< float >( z ) );

              // Transform point by mask transform
              edge_point = grid_transform.project( edge_point );

              points.push_back( edge_point );
              back_edge_x[ q + this->nx_ ] = point_cnt;
              point_cnt++;
            }

            // left border and center ones
            if ( ( ( type>>0 ) ^ ( type>>3 ) ) & 0x01 )
            {
              PointF edge_point = PointF( static_cast< float >( x ),
                static_cast< float >( y ) + INTERP_EDGE_OFFSET_C,
                static_cast< float >( z ) );

              // Transform point by mask transform
              edge_point = grid_transform.project( edge_point );

              points.push_back( edge_point );
              back_edge_y[ q ] = point_cnt;
              point_cnt++;
            }

            // right one
            if ( ( x == this->elem_nx_ - 1 ) && ( ( ( type>>1 ) ^ ( type>>2 ) ) & 0x01 ) )
            {
              PointF edge_point = PointF( static_cast< float >( x + 1 ), 
                static_cast< float >( y ) + INTERP_EDGE_OFFSET_C,
                static_cast< float >( z ) );

              // Transform point by mask transform
              edge_point = grid_transform.project( edge_point );

              points.push_back( edge_point );
              back_edge_y[ q + 1 ] = point_cnt;
              point_cnt++;
            }
          }

          // top border and center ones
          if ( ( ( type>>4 ) ^ ( type>>5 ) ) & 0x01 )
          {
            PointF edge_point = PointF( static_cast< float >( x ) + INTERP_EDGE_OFFSET_C,
              static_cast< float >( y ),
              static_cast< float >( z + 1 ) );

            // Transform point by mask transform
            edge_point = grid_transform.project( edge_point );

            points.push_back( edge_point );
            front_edge_x[ q ] = point_cnt;
            point_cnt++;
          }

          // bottom border one

          if ( ( y == this->elem_ny_ - 1 ) && ( ( ( type>>6 ) ^ ( type>>7 ) ) & 0x01 ) )
          {
            PointF edge_point = PointF( static_cast< float >( x ) + INTERP_EDGE_OFFSET_C,
              static_cast< float >( y + 1 ),
              static_cast< float >( z + 1 ) );

            // Transform point by mask transform
            edge_point = grid_transform.project( edge_point );

            points.push_back( edge_point );
            front_edge_x[ q + this->nx_ ] = point_cnt;
            point_cnt++;
          }

          // left border and center ones
          if ( ( ( type>>4 ) ^ ( type>>7 ) ) & 0x01 )
          {
            PointF edge_point = PointF( static_cast< float >( x ),
              static_cast< float >( y ) + INTERP_EDGE_OFFSET_C,
              static_cast< float >( z + 1 ) );

            // Transform point by mask transform
            edge_point = grid_transform.project( edge_point );

            points.push_back( edge_point );
            front_edge_y[ q ] = point_cnt;
            point_cnt++;
          }

          // bottom one
          if ( ( x== this->elem_nx_ - 1 ) && ( ( ( type>>5 ) ^ ( type>>6 ) ) & 0x01 ) )
          {
            PointF edge_point = PointF( static_cast< float >( x + 1 ), 
              static_cast< float >( y ) + INTERP_EDGE_OFFSET_C,
              static_cast< float >( z + 1 ) );

            // Transform point by mask transform
            edge_point = grid_transform.project( edge_point );

            points.push_back( edge_point );
            front_edge_y[ q + 1 ] = point_cnt;
            point_cnt++;
          }

          // side edges

          if ( ( ( type>>0 ) ^ ( type >> 4 ) ) & 0x01 )
          {
            PointF edge_point = PointF( static_cast< float >( x ), 
              static_cast< float >( y ), 
              static_cast< float >( z ) + INTERP_EDGE_OFFSET_C );

            // Transform point by mask transform
            edge_point = grid_transform.project( edge_point );

            points.push_back( edge_point );
            side_edge[ q ] = point_cnt;
            point_cnt++;              
          }    


          if ( ( x == this->elem_nx_ - 1 ) && ( ( ( type>>1 ) ^ ( type>>5 ) ) & 0x01 ) )
          {
            PointF edge_point = PointF( static_cast< float >( x + 1 ), 
              static_cast< float >( y ),
              static_cast< float >( z ) + INTERP_EDGE_OFFSET_C );

            // Transform point by mask transform
            edge_point = grid_transform.project( edge_point );

            points.push_back( edge_point );
            side_edge[ q + 1 ] = point_cnt;
            point_cnt++;              
          }

          if ( ( y == this->elem_ny_ - 1 ) && ( ( ( type>>3 ) ^ ( type>>7 ) ) & 0x01 ) )
          {
            PointF edge_point = PointF( static_cast< float >( x ), 
              static_cast< float >( y + 1 ), 
              static_cast< float >( z ) + INTERP_EDGE_OFFSET_C );

            // Transform point by mask transform
            edge_point = grid_transform.project( edge_point );

            points.push_back( edge_point );
            side_edge[ q + this->nx_ ] = point_cnt;
            point_cnt++;              
          }

          if ( ( ( y == this->elem_ny_ - 1 ) && ( x == this->elem_nx_ - 1 ) ) && 
            ( ( ( type>>2 ) ^ ( type>>6 ) ) & 0x01 ) )
          {
            PointF edge_point = PointF( static_cast< float >( x + 1 ), 
              static_cast< float >( y + 1 ), 
              static_cast< float >( z ) + INTERP_EDGE_OFFSET_C );

            // Transform point by mask transform
            edge_point = grid_transform.project( edge_point );

            points.push_back( edge_point );
            side_edge[ q + this->nx_ + 1 ] = point_cnt;
            point_cnt++;              
          }
        }
      }
    }

    barrier.wait();
    elements.clear();

    // Combine points from all threads
    if ( thread == 0 )
    {   
      size_t local_size = 0;
      for ( int p = 0; p < num_threads; p++ )
      {            
        this->front_offset_[ p ] = this->global_point_cnt_ + local_size;
        if ( z == 0 )
        {
          this->back_offset_[ p ] = this->front_offset_[ p ];
        }
        local_size += this->new_points_[ p ].size();
        std::vector< PointF >& points = this->new_points_[ p ];
        for ( size_t q = 0; q < points.size(); q++ )
        {
          this->points_.push_back( points[ q ] ); 
        }
      }
      this->global_point_cnt_ += local_size;
    
      this->min_point_index_[ z ] = this->prev_point_min_;
      this->max_point_index_[ z ] = static_cast<unsigned int>( this->points_.size() );
      this->prev_point_min_ = this->prev_point_max_;
      this->prev_point_max_ = this->max_point_index_[ z ];
    }

    barrier.wait();

    // Build triangles
    for ( size_t y = elem_nystart; y < elem_nyend; y++ )
    {
      for (size_t x=0;x<elem_nx_;x++)
      {
        size_t elem_offset = y * this->nx_ + x;
        unsigned char type = this->type_buffer_[ elem_offset ];

        // All points are inside or outside the cube -- does not contribute to the 
        // isosurface 
        if ( type == 0 || type == 0xFF ) 
        {
          continue;
        }

        // Get the edges from the marching cube table 
        const MarchingCubesTableType& table = MARCHING_CUBES_TABLE_C[ type ];

        for ( int k = 0; k < table.num_triangles_; k++ )
        {
          // Get the edge index (0-11 for 12 edges)
          int i1 = table.edges_[ 3 * k ];
          int i2 = table.edges_[ 3 * k + 1 ];
          int i3 = table.edges_[ 3 * k + 2 ];

          unsigned int p1 = edge_table[ i1 ][ elem_offset ];
          unsigned int p2 = edge_table[ i2 ][ elem_offset ];
          unsigned int p3 = edge_table[ i3 ][ elem_offset ];

          if ( i1 < 4 ) 
          {
            elems[ 0 ] = ( p1 & 0x00FFFFFF ) + this->back_offset_[ p1>>24 ];
          }
          else
          {
            elems[ 0 ] = ( p1 & 0x00FFFFFF ) + this->front_offset_[ p1>>24 ];            
          }

          if ( i2 < 4 ) 
          {
            elems[ 1 ] = ( p2 & 0x00FFFFFF ) + this->back_offset_[ p2>>24 ];
          }
          else
          {
            elems[ 1 ] = ( p2 & 0x00FFFFFF ) + this->front_offset_[ p2>>24 ];            
          }

          if ( i3 < 4 ) 
          {
            elems[ 2 ] = ( p3 & 0x00FFFFFF ) + this->back_offset_[ p3>>24 ];
          }
          else
          {
            elems[ 2 ] = ( p3 & 0x00FFFFFF ) + this->front_offset_[ p3>>24 ];            
          }
          elements.push_back( elems );
          // Add the area of the triangle to the total
          this->new_elem_areas_[ thread ] += 0.5f * 
            Cross( this->points_[ elems[ 1 ] ] - this->points_[ elems[ 0 ] ], 
            this->points_[ elems[ 2 ] ] - this->points_[ elems[ 0 ] ] ).length();
        }
      }
    }

    std::swap( back_buffer_x, front_buffer_x );
    std::swap( back_buffer_y, front_buffer_y );
    barrier.wait();

    if ( thread == 0 )
    {
      this->min_face_index_[ z ] = static_cast<unsigned int>( this->faces_.size() );

      for ( int w = 0;  w < num_threads; w++ )
      {
        std::vector< StackVector< size_t, 3 > >& pelements = this->new_elems_[ w ];
        for ( size_t p = 0; p < pelements.size(); p++ )
        {
          StackVector< size_t, 3 >& el = pelements[ p ];
          this->faces_.push_back( static_cast< unsigned int >( el[ 0 ] ) );
          this->faces_.push_back( static_cast< unsigned int >( el[ 1 ] ) );
          this->faces_.push_back( static_cast< unsigned int >( el[ 2 ] ) );
        }
      }
      this->back_offset_ = this->front_offset_;

      this->max_face_index_[ z ] = static_cast<unsigned int>( this->faces_.size() );
    }

    if ( thread == 0 )
    {
      if ( this->check_abort_() ) 
      {
        this->need_abort_ = true;
      }
    }

    barrier.wait();   

    if ( this->need_abort_ ) 
    {
      return;
    }
    
    // Update progress based on number of z slices processed
    double compute_progress = 
      static_cast< double >( z + 1 ) / static_cast< double >( this->elem_nz_ );
    double total_progress = compute_progress * COMPUTE_PERCENT_PROGRESS_C;
    this->isosurface_->update_progress_signal_( total_progress );
  }   

  barrier.wait();

  // Add up surface areas computed in all threads
  if ( thread == 0 )
  {
    for ( int p = 0; p < num_threads; ++p )
    {
      this->area_ += this->new_elem_areas_[ p ];
    }
  }
  
  barrier.wait();
}

//  Translates border face coords (i, j) to volume coords (x, y, z).  


void IsosurfacePrivate::translate_cap_coords( int cap_num, float i, float j, 
  float& x, float& y, float& z )
{
  switch( cap_num )
  {
    case 0: // x, y, z = 0 (front)
      x = i;
      y = j;
      z = 0;
      break;
    case 1: // x, z, y = 0  (top) 
      x = i;
      y = 0;
      z = j;
      break;
    case 2: // y, z, x = 0 (left)
      x = 0;
      y = i;
      z = j;
      break;
    case 3: // x, y, z = nz - 1 (back)
      x = i;
      y = j;
      z = static_cast< float >( this->nz_ - 1 );
      break;
    case 4: // x, z, y = ny - 1 (bottom)
      x = i;
      y = static_cast< float >( this->ny_ - 1 );
      z = j;
      break;
    case 5: // y, z, x = nx - 1 (right)
      x = static_cast< float >( this->nx_ - 1 );
      y = i;
      z = j;
      break;
    default:
      x = y = z = 0;
      break;
  }
}

size_t IsosurfacePrivate::get_data_index( float x, float y, float z )
{
  size_t data_index = ( static_cast< size_t >( z ) * this->nx_ * this->ny_ ) + 
    ( static_cast< size_t >( y ) * this->nx_ ) + static_cast< size_t >( x );
  return data_index;
}

/*
Basic ideas:
- Marching cubes:
  - Each "cube" has a configuration of on/off nodes that can be represented as an 8-bit bitmask.
    The bitmask can be used as an index into a 256 (2^8) entry lookup table of predefined, canonical
    facet combinations.
  - A naive implementation of marching cubes would just process each cube 
    independently and add all the faces (points and indices) for that cube.  The problem with this 
    approach is that points end up in the points list multiple times (more GPU memory) and edges get 
    evaluated multiple times (once for each cube they belong to, up to 4).  To avoid this we process edges 
    one at a time by running through them horizontally and vertically.  For each edge that is split (one vertex is "on" and the other is "off"),
    we add a point to the points list.  At this point we have a list of points as well as a canonical facet
    combination for each cube.  We want to build triangles.  What we need is a way to map the canonical edge indices in the facet 
    combination to indices of actual points in the points list.  So while we are examining edges we 
    build a translation table to keep track of what canonical index a split edge point belongs to for 
    each cube adjacent to the edge.  For each cube we can look up the point index for each canonical index in that 
    cube's facet combination.  This 2D array looks like 
    cubes[cube_index][canonical_edge_index] = point_index.  
  Size: cubes[num cubes][12 (edges per cube)]
  We're translating this data:
    cube type -> facet combo -> canonical edge index -> actual point index -> point
  - Normally in marching cubes the edge points are linearly interpolated between the vertex values, but for
  for binary images we always place the edge points in the middle of the edge.
- Isosurface capping
  - Generating isosurface geometry for the 6 caps of the volume is a 2D operation since each
    of the 6 faces is a 2D image.  Therefore, instead of calling the elements "cubes," we call them 
    "cells."
  - We need a different facet combo type table for caps.
  - For caps, both nodes and edge center-points can potentially be added to the list of points.  This is 
    because there would be edge points between the caps and the imaginary outside padding, but
    these points get clamped to the nodes on the border.  In fact, all border nodes that are "on" are
    included as points because they are all adjacent to the imaginary padding that is "off."  This
  means that the canonical point indices must include both nodes and edge points, amounting to 
  8 canonical indices for a cell (4 nodes, 4 edge points).
*/
// Naive implementation -- needs to be optimized!
void IsosurfacePrivate::compute_cap_faces()
{
  // TODO Figure out how to handle x/y/z indices for caps so that code doesn't have to
  // be duplicated for each cap
    
  // Solution A: have function that translates (i, j) to (x, y, z).  Try this first.
  // Solution B (faster): Have (i, j, k) Vector offsets based on cap.  Add at end of loops.

  // Process 6 caps independently.  At most this will duplicate volume edge nodes twice.
  
  size_t nx = this->nx_; // Store local copy just to make code more concise
  size_t ny = this->ny_;
  size_t nz = this->nz_;
  std::vector< std::pair< size_t, size_t > > cap_dimensions;
  cap_dimensions.push_back( std::make_pair( nx, ny ) ); // front and back caps
  cap_dimensions.push_back( std::make_pair( nx, nz ) ); // top and bottom caps
  cap_dimensions.push_back( std::make_pair( ny, nz ) ); // left and right side caps
  PointF elem_vertices[ 3 ]; // Temporary storage for triangle vertices

  // For each of 6 caps
  for( int cap_num = 0; cap_num < 6; cap_num++ )
  {
    unsigned int min_point_index = static_cast< unsigned int >( this->points_.size() );
    unsigned int min_face_index = static_cast< unsigned int >( this->faces_.size() );

    // Each 
    // STEP 1: Find cell types

    // Calculate ni, nj for this face
    size_t ni = cap_dimensions[ cap_num % 3 ].first;
    size_t nj = cap_dimensions[ cap_num % 3 ].second;

    // Since each cell has 4 nodes, we only need a char to represent the type.  Only using bits
    // 0 - 3.
    // Create an array of type per index
    size_t num_cells = ( ni - 1 )* ( nj - 1 );
    std::vector< unsigned char > cell_types( num_cells );
    // Loop through all 2D cells to find each cell type
    size_t cell_index = 0;
    bool some_nodes_on = false;
    for( float j = 0; j < nj - 1; j++ )
    {
      for( float i = 0; i < ni - 1; i++ )
      {
        // Build type
        // type = index into polygonal configuration table
        unsigned char cell_type = 0;

        // A 4 bit index is formed where each bit corresponds to a vertex 
        // Bit on if vertex is inside surface, off otherwise
        float x, y, z;
        this->translate_cap_coords( cap_num, i, j, x, y, z ); 
        size_t data_index = this->get_data_index( x, y, z );
        if ( this->data_[ data_index ] & this->mask_value_ )  cell_type |= 0x1;
        
        this->translate_cap_coords( cap_num, i + 1, j, x, y, z ); 
        data_index = this->get_data_index( x, y, z );
        if ( this->data_[ data_index ] & this->mask_value_ )  cell_type |= 0x2;

        this->translate_cap_coords( cap_num, i + 1, j + 1, x, y, z ); 
        data_index = this->get_data_index( x, y, z );
        if ( this->data_[ data_index ] & this->mask_value_ )  cell_type |= 0x4;

        this->translate_cap_coords( cap_num, i, j + 1, x, y, z ); 
        data_index = this->get_data_index( x, y, z );
        if ( this->data_[ data_index ] & this->mask_value_ )  cell_type |= 0x8;

        cell_types[ cell_index ] = cell_type;
        cell_index++; 

        if( cell_type != 0 )
        {
          some_nodes_on = true;
        }
      }
    }

    // If no border nodes for this cap are on, skip this cap
    if( !some_nodes_on )
    {
      continue;
    }

    // STEP 2: Add nodes to points list and translation table

    // Create translation table (2D matrix storing indices into actual points vector)
    // size_t point_trans_table[num cells = (nx - 1) * (ny - 1)][8 canonical indices (4 nodes, 4 edge points)]
    std::vector< boost::array< unsigned int, 8 > > point_trans_table( num_cells );

    // Get mask transform from MaskVolume 
    GridTransform grid_transform = this->compute_mask_volume_->get_grid_transform();

    // All border nodes that are "on" are included as points because they are all adjacent to 
    // the imaginary padding that is "off." 
    // Loop through all nodes
    
    for( float j = 0; j < nj; j++ )
    {
      for( float i = 0; i < ni; i++ )
      {
        // Translate (i, j) to (x, y, z) for this cap
        float x, y, z;
        this->translate_cap_coords( cap_num, i, j, x, y, z ); 

        // Look up mask value at (x, y, z)
        size_t data_index = this->get_data_index( x, y, z );
          
        // If mask value on
        if ( this->data_[ data_index ] & this->mask_value_ )
        {
          // Transform point by mask transform
          PointF node_point = grid_transform.project( PointF( x, y, z ) );
          // Add node to the points list.
          this->points_.push_back( node_point );
          unsigned int point_index = 
            static_cast< unsigned int >( this->points_.size() - 1 );

          // Add relevant canonical coordinates to translation table for adjacent cells.
          // Find indices and canonical coordinates of 1-4 adjacent cells

          // Lower right cell index
          if( i < ni - 1 && j < nj - 1 )
          {
            size_t cell_index = static_cast< size_t >( ( j * ( ni - 1 ) ) + i ); 
            size_t canonical_coordinate = 0; 
            point_trans_table[ cell_index ][ canonical_coordinate ] = point_index;
          }
          
          // Lower left cell index
          if( i > 0 && j < nj - 1 )
          {
            size_t cell_index = static_cast< size_t >( ( j * ( ni - 1 ) ) + i - 1 ); 
            size_t canonical_coordinate = 1; 
            point_trans_table[ cell_index ][ canonical_coordinate ] = point_index;
          }
          
          // Upper left cell index
          if( i > 0 && j > 0 )
          {
            size_t cell_index = 
              static_cast< size_t >( ( ( j - 1 ) * ( ni - 1 ) ) + i - 1 ); 
            size_t canonical_coordinate = 2; 
            point_trans_table[ cell_index ][ canonical_coordinate ] = point_index;
          }
          
          // Upper right cell index
          if( i < ni - 1 && j > 0 )
          {
            size_t cell_index = static_cast< size_t >( ( ( j - 1 ) * ( ni - 1 ) ) + i ); 
            size_t canonical_coordinate = 3; 
            point_trans_table[ cell_index ][ canonical_coordinate ] = point_index;
          }
        }
      }
    }

    // STEP 3: Find edge nodes, add to points list and translation table

    // Check vertical edges
    for( float j = 0; j < nj - 1; j++ )
    {
      for( float i = 0; i < ni; i++ )
      {
        // Find endpoint nodes
        float start_x, start_y, start_z;
        this->translate_cap_coords( cap_num, i, j, start_x, start_y, start_z ); 
        float end_x, end_y, end_z;
        this->translate_cap_coords( cap_num, i, j + 1, end_x, end_y, end_z ); 

        size_t start_data_index = this->get_data_index( start_x, start_y, start_z );
        size_t end_data_index = this->get_data_index( end_x, end_y, end_z );
          
        // If edge is "split" (one endpoint node is on and the other is off)
        unsigned char start_bit = this->data_[ start_data_index ] & this->mask_value_;
        unsigned char end_bit = this->data_[ end_data_index ] & this->mask_value_;
        if ( start_bit != end_bit )
        {
          // Find edge point
          float edge_x, edge_y, edge_z;
          this->translate_cap_coords( cap_num, i, j + 0.5f, 
            edge_x, edge_y, edge_z ); 

          // Transform point by mask transform
          PointF edge_point = grid_transform.project( PointF( edge_x, edge_y, edge_z) );
          // Add edge to the points list.
          this->points_.push_back( edge_point );
          unsigned int point_index = 
            static_cast< unsigned int >( this->points_.size() - 1 );

          // Add the relevant canonical coordinates to the translation table for adjacent 1-2 cells.

          // Left cell index
          if( i > 0 )
          {
            size_t cell_index = static_cast< size_t >( ( j * ( ni - 1 ) ) + i - 1 ); 
            size_t canonical_coordinate = 5; 
            point_trans_table[ cell_index ][ canonical_coordinate ] = point_index;
          }

          // Right cell index
          if( i < ni - 1 )
          {
            size_t cell_index = static_cast< size_t >( ( j * ( ni - 1 ) ) + i ); 
            size_t canonical_coordinate = 7; 
            point_trans_table[ cell_index ][ canonical_coordinate ] = point_index;
          }
        }
      }
    }

    // Check horizontal edges
    for( float j = 0; j < nj; j++ )
    {
      for( float i = 0; i < ni - 1; i++ )
      {
        // Find endpoint nodes
        float start_x, start_y, start_z;
        this->translate_cap_coords( cap_num, i, j, start_x, start_y, start_z ); 
        float end_x, end_y, end_z;
        this->translate_cap_coords( cap_num, i + 1, j, end_x, end_y, end_z ); 

        size_t start_data_index = this->get_data_index( start_x, start_y, start_z );
        size_t end_data_index = this->get_data_index( end_x, end_y, end_z );

        // If edge is "split" (one endpoint node is on and the other is off)
        unsigned char start_bit = this->data_[ start_data_index ] & this->mask_value_;
        unsigned char end_bit = this->data_[ end_data_index ] & this->mask_value_;
        if ( start_bit != end_bit )
        {
          // Find edge point
          float edge_x, edge_y, edge_z;
          this->translate_cap_coords( cap_num, i + 0.5f, j, 
            edge_x, edge_y, edge_z ); 

          // Transform point by mask transform
          PointF edge_point = grid_transform.project( PointF( edge_x, edge_y, edge_z) );
          // Add edge to the points list.
          this->points_.push_back( edge_point );
          unsigned int point_index = 
            static_cast< unsigned int >( this->points_.size() - 1 );

          // Add the relevant canonical coordinates to the translation table for adjacent 1-2 cells.

          // Upper cell index
          if( j > 0 )
          {
            size_t cell_index = static_cast< size_t >( ( ( j - 1 ) * ( ni - 1 ) ) + i ); 
            size_t canonical_coordinate = 6; 
            point_trans_table[ cell_index ][ canonical_coordinate ] = point_index;
          }

          // Lower cell index
          if( j < nj - 1 )
          {
            size_t cell_index = static_cast< size_t >( ( j * ( ni - 1 ) ) + i ); 
            size_t canonical_coordinate = 4; 
            point_trans_table[ cell_index ][ canonical_coordinate ] = point_index;
          }
        }
      }
    }
    
    // STEP 4: Create face geometry (points + triangle indices)

    // Add points list to existing vertex list
    // For each cell
    for( size_t cell_index = 0; cell_index < num_cells; cell_index++ )
    {
      // Lookup cell type (already stored in a 1D vector)
      unsigned char cell_type = cell_types[ cell_index ];
      
      // Couldn't I just look up type on the fly here?  Or does that make parallelization harder?
      // Look up the facet combo for this type
      const CappingTableType& tesselation = CAPPING_TABLE_C[ cell_type ];

      // For each triangle in the tesselation for this type
      for( int triangle_index = 0; triangle_index < tesselation.num_triangles_; triangle_index++ )
      {
        // For each vertex in the triangle
        for( int triangle_point_index = 0; triangle_point_index < 3; triangle_point_index++ )
        {
          // Find canonical index for the point
          int canonical_index = 
            tesselation.points_[ 3 * triangle_index + triangle_point_index ];
        
          // Look up the point index in the translation table for this cell 
          unsigned int point_index = point_trans_table[ cell_index ][ canonical_index ];
          // Store the point coordinates in the temporary variable
          elem_vertices[ triangle_point_index ] = this->points_[ point_index ];
          // Add point index to the faces list
          this->faces_.push_back( point_index );
        }
        // Compute the area of  the triangle and add it to the total area
        this->area_ += 0.5f * Cross( elem_vertices[ 1 ] - elem_vertices[ 0 ], 
          elem_vertices[ 2 ] - elem_vertices[ 0 ] ).length();
      }
    }

    // Create a rendering "patch" for each cap 
    unsigned int max_point_index = static_cast< unsigned int >( this->points_.size() );
    unsigned int max_face_index = static_cast< unsigned int >( this->faces_.size() );
    if( min_point_index != max_point_index && min_face_index != max_face_index )
    {
      this->min_point_index_.push_back( min_point_index );
      this->max_point_index_.push_back( max_point_index );

      this->min_face_index_.push_back( min_face_index );
      this->max_face_index_.push_back( max_face_index );
    }
  }
}

void IsosurfacePrivate::parallel_compute_normals( int thread, int num_threads, 
  boost::barrier& barrier )
{
  size_t num_vertices = this->points_.size();

  if ( thread == 0 ) // Only need to setup once 
  { 
    // Reset the normals vector
    this->normals_.clear();
    this->normals_.resize( num_vertices, VectorF( 0, 0, 0 ) );
  }

  // All threads have to wait until setup is done before proceeding
  barrier.wait();

  // In order to parallelize the algorithm, each thread processes a different range of vertex 
  // indices [vertex_index_start, vertex_index_end)
  size_t vertex_range_size = static_cast< size_t >( num_vertices / num_threads );
  if ( vertex_range_size * num_threads < num_vertices ) 
  {
    vertex_range_size++;
  }

  size_t vertex_index_start = thread * vertex_range_size;
  size_t vertex_index_end = ( thread + 1 ) * vertex_range_size;
  if ( vertex_index_end > num_vertices ) 
  {
    vertex_index_end = num_vertices;
  }

  // For each face
  for( size_t i = 0; i + 2 < this->faces_.size(); i += 3 )
  {
    size_t vertex_index1 = this->faces_[ i ];
    size_t vertex_index2 = this->faces_[ i + 1 ];
    size_t vertex_index3 = this->faces_[ i + 2 ];

    // If this face has at least one vertex in our range
    if( ( vertex_index_start <= vertex_index1 && vertex_index1 < vertex_index_end ) ||
      ( vertex_index_start <= vertex_index2 && vertex_index2 < vertex_index_end ) ||
      ( vertex_index_start <= vertex_index3 && vertex_index3 < vertex_index_end ) )
    { 
      // Get vertices of face
      PointF p1 = this->points_[ vertex_index1 ];
      PointF p2 = this->points_[ vertex_index2 ];
      PointF p3 = this->points_[ vertex_index3 ];

      // Calculate cross product of edges
      VectorF v0 = p3 - p2;
      VectorF v1 = p1 - p2;
      VectorF n = Cross( v0, v1 );

      // Add to normal for each vertex in our range
      if( ( vertex_index_start <= vertex_index1 && vertex_index1 < vertex_index_end ) )
      {
        this->normals_[ vertex_index1 ] += n;
      }
      if( ( vertex_index_start <= vertex_index2 && vertex_index2 < vertex_index_end ) )
      {
        this->normals_[ vertex_index2 ] += n;
      }
      if( ( vertex_index_start <= vertex_index3 && vertex_index3 < vertex_index_end ) )
      {
        this->normals_[ vertex_index3 ] += n;
      }
    }
  }

  // For each vertex in our range
  for( size_t i = vertex_index_start; i < vertex_index_end; i++ )
  {
    // Normalize normal
    this->normals_[ i ].normalize();
  }
}

void IsosurfacePrivate::upload_to_vertex_buffer()
{
  if ( !this->surface_changed_ && !this->values_changed_ )
  {
    return;
  }

  size_t num_of_parts = this->part_points_.size();
  bool has_values = this->values_.size() == this->points_.size();

  // Estimate the size of video memory required to upload the isosurface
  ptrdiff_t total_size = 0;
  for ( size_t i = 0; i < num_of_parts; ++i )
  {
    unsigned int num_pts = this->part_points_[ i ].second - this->part_points_[ i ].first;
    ptrdiff_t vertex_size = num_pts * sizeof( PointF );
    ptrdiff_t normal_size = num_pts * sizeof( VectorF );
    ptrdiff_t value_size = has_values ? num_pts * sizeof( float ) : 0;
    unsigned int num_face_indices = this->part_faces_[ i ].second - this->part_faces_[ i ].first;
    ptrdiff_t face_size = num_face_indices * sizeof( unsigned int );
    ptrdiff_t batch_size = vertex_size + normal_size + value_size + face_size;
    CORE_LOG_MESSAGE( "Isosurface Batch " + ExportToString( i ) + ": " +
             ExportToString( num_pts ) + " vertices, " +
             ExportToString( num_face_indices / 3 ) + " triangles. Total memory: " + 
             ExportToString( batch_size ) );
    total_size += batch_size;
  }
  CORE_LOG_MESSAGE( "Total memory required for the isosurface: " +
           ExportToString( total_size ) );
  
  if ( total_size + ( 20 << 20 ) > static_cast< ptrdiff_t >( 
    RenderResources::Instance()->get_vram_size() ) )
  {
    CORE_LOG_WARNING( "Could not fit the isosurface in GPU memory" );
    this->vbo_batches_.clear();
    this->surface_changed_ = false;
    this->values_changed_ = false;
    this->vbo_available_ = false;
    return;
  }
  
  RenderResources::lock_type rr_lock( RenderResources::GetMutex() );
  this->vbo_batches_.resize( num_of_parts );
  for ( size_t i = 0; i < num_of_parts; ++i )
  {
    this->vbo_batches_[ i ].reset( new VertexBufferBatch );
    this->vbo_batches_[ i ]->vertex_buffer_.reset( new Core::VertexAttribArrayBuffer );
    this->vbo_batches_[ i ]->normal_buffer_.reset( new Core::VertexAttribArrayBuffer );
    this->vbo_batches_[ i ]->faces_buffer_.reset( new Core::ElementArrayBuffer );
    this->vbo_batches_[ i ]->vertex_buffer_->set_array( 
      VertexAttribArrayType::VERTEX_E, 3, GL_FLOAT, 0, 0 );
    this->vbo_batches_[ i ]->normal_buffer_->set_array( 
      VertexAttribArrayType::NORMAL_E, GL_FLOAT, 0, 0 );

    unsigned int num_pts = this->part_points_[ i ].second - this->part_points_[ i ].first;
    ptrdiff_t vertex_size = num_pts * sizeof( PointF );
    ptrdiff_t normal_size = num_pts * sizeof( VectorF );
    unsigned int num_face_indices = this->part_faces_[ i ].second - this->part_faces_[ i ].first;
    ptrdiff_t face_size = num_face_indices * sizeof( unsigned int );

    this->vbo_batches_[ i ]->vertex_buffer_->set_buffer_data( vertex_size, 
      &this->points_[ this->part_points_[ i ].first ], GL_STATIC_DRAW );
    this->vbo_batches_[ i ]->normal_buffer_->set_buffer_data( normal_size, 
      &this->normals_[ this->part_points_[ i ].first ], GL_STATIC_DRAW );
    this->vbo_batches_[ i ]->faces_buffer_->set_buffer_data( face_size, 
      &this->part_indices_[ i ][ 0 ], GL_STATIC_DRAW );
    if ( has_values )
    {
      this->vbo_batches_[ i ]->value_buffer_.reset( new Core::VertexAttribArrayBuffer );
      this->vbo_batches_[ i ]->value_buffer_->set_generic_array( 1, 1, GL_FLOAT, 
        GL_FALSE, 0, 0 );
      this->vbo_batches_[ i ]->value_buffer_->set_buffer_data( num_pts * sizeof( float ),
        &this->values_[ this->part_points_[ i ].first ], GL_STATIC_DRAW );
    }
  }
  
  this->surface_changed_ = false;
  this->values_changed_ = false;
  this->vbo_available_ = true;
}

void IsosurfacePrivate::reset()
{
  this->points_.clear();
  this->normals_.clear();
  this->faces_.clear();
  this->values_.clear();
}

Isosurface::Isosurface( const MaskVolumeHandle& mask_volume ) :
  private_( new IsosurfacePrivate )
{
  this->private_->isosurface_ = this;
  this->private_->orig_mask_volume_ = mask_volume;
  this->private_->compute_mask_volume_ = mask_volume;
  this->private_->surface_changed_ = false;
  this->private_->values_changed_ = false;
  this->private_->vbo_available_ = false;

  // Test code -- set default colormap
  //this->private_->color_map_ = ColorMapHandle( new ColorMap() );
}

void Isosurface::compute( double quality_factor, bool capping_enabled, 
  boost::function< bool () > check_abort )
{
  lock_type lock( this->get_mutex() );

  this->private_->points_.clear();
  this->private_->normals_.clear();
  this->private_->faces_.clear();
  this->private_->values_.clear();
  this->private_->area_ = 0;
  this->private_->values_changed_ = false;
  this->private_->check_abort_ = check_abort;

  {
    Core::MaskVolume::shared_lock_type vol_lock( this->private_->orig_mask_volume_->get_mutex() );

    // Initially assume we're computing the isosurface for the original volume (not downsampled)
    this->private_->compute_mask_volume_ = this->private_->orig_mask_volume_;

    // Downsample mask if needed
    if( quality_factor != 1.0 )
    {
      assert( quality_factor == 0.5 || quality_factor == 0.25 || quality_factor == 0.125 );
      Parallel parallel_downsample( boost::bind( &IsosurfacePrivate::parallel_downsample_mask, 
        this->private_, _1, _2, _3, quality_factor ) );
      parallel_downsample.run();
    }

    if ( check_abort() )
    {
      // leave it in a decent state
      this->private_->reset();
      return;
    }

    // Copy values to members just to simplify and shorten code.
    this->private_->compute_setup();

    // Compute isosurface without caps
    Parallel parallel_faces( boost::bind( &IsosurfacePrivate::parallel_compute_faces, 
      this->private_, _1, _2, _3 ) );
    parallel_faces.run();

    if ( check_abort() )
    {
      // leave it in a decent state
      this->private_->reset();
      return;
    }

    // Compute isosurface caps
    if( capping_enabled )
    {
      this->private_->compute_cap_faces();
    }
  }

  // Check for empty isosurface
  if( this->private_->points_.size() == 0 ) 
  {
    return;
  }

  // Test code -- assign values to vertices in range [0, 1].  
  /*size_t num_points = this->private_->points_.size();
  for( size_t i = 0; i < num_points; i++ )
  {
    float val = static_cast< float >( i ) / static_cast< float >( num_points );
    this->private_->values_.push_back( val );
  }*/

  Parallel parallel_normals( boost::bind( &IsosurfacePrivate::parallel_compute_normals, 
    this->private_, _1, _2, _3 ) );
  parallel_normals.run();

  if ( check_abort() )
  {
    // leave it in a decent state
    this->private_->reset();
    return;
  }

  this->update_progress_signal_( IsosurfacePrivate::COMPUTE_PERCENT_PROGRESS_C + 
    IsosurfacePrivate::NORMAL_PERCENT_PROGRESS_C );

  this->private_->type_buffer_.clear();
  this->private_->edge_buffer_.clear();
  this->private_->new_points_.clear();
  this->private_->new_elems_.clear();
  this->private_->new_elem_areas_.clear();
  this->private_->front_offset_.clear();
  this->private_->back_offset_.clear();
  
  this->private_->part_points_.clear();
  this->private_->part_faces_.clear();
  this->private_->part_indices_.clear();
  
  unsigned int num_faces = 0;
  unsigned int min_point_index = 0;
  unsigned int min_face_index = 0;
  
  for ( size_t j = 0; j < this->private_->min_point_index_.size(); j++ )
  {
    if ( num_faces == 0 )
    {
      min_point_index = this->private_->min_point_index_[ j ];
      min_face_index = this->private_->min_face_index_[ j ];
    }
    
    num_faces += this->private_->max_face_index_[ j ] - this->private_->min_face_index_[ j ];
    
    if ( num_faces > 0 && (num_faces > 1000000 || j == this->private_->min_point_index_.size() - 1 ) )
    {
      this->private_->part_points_.push_back( std::make_pair(
        min_point_index, this->private_->max_point_index_[ j ] ) );
      this->private_->part_faces_.push_back( std::make_pair(
        min_face_index, this->private_->max_face_index_[ j ] ) );
      num_faces = 0;
    }

    if ( check_abort() )
    {
      // leave it in a decent state
      this->private_->reset();
      return;
    }

    // Update progress
    double partition_progress = 
      static_cast< double >( j + 1 ) / static_cast< double >( this->private_->min_point_index_.size() ); 
    double total_progress = IsosurfacePrivate::COMPUTE_PERCENT_PROGRESS_C + 
      IsosurfacePrivate::NORMAL_PERCENT_PROGRESS_C +
      ( partition_progress * IsosurfacePrivate::PARTITION_PERCENT_PROGRESS_C );
    this->update_progress_signal_( total_progress );
  }
  size_t num_batches = this->private_->part_points_.size();
  for ( size_t i = 0; i < num_batches; ++i )
  {
    unsigned int num_face_indices = this->private_->part_faces_[ i ].second - 
      this->private_->part_faces_[ i ].first;
    std::vector< unsigned int > local_indices( num_face_indices );
    for ( size_t j = 0; j < num_face_indices; ++j )
    {
      size_t pt_global_idx = this->private_->part_faces_[ i ].first + j;
      local_indices[ j ] = this->private_->faces_[ pt_global_idx ] - 
        this->private_->part_points_[ i ].first;
      assert( local_indices[ j ] >= 0 && 
           local_indices[ j ] < ( this->private_->part_points_[ i ].second - 
                     this->private_->part_points_[ i ].first ) );
    }
    this->private_->part_indices_.push_back( local_indices );
  }
  
  this->private_->min_point_index_.clear();
  this->private_->max_point_index_.clear();
  this->private_->min_face_index_.clear();
  this->private_->max_face_index_.clear();

  this->private_->surface_changed_ = true;

  if ( check_abort() )
  {
    // leave it in a decent state
    this->private_->reset();
    return;
  }

  this->update_progress_signal_( 1.0 );

  // Test code
  // this->export_legacy_isosurface( "", "test_isosurface" );
}

const std::vector< PointF >& Isosurface::get_points() const
{
  return this->private_->points_;
}

const std::vector< unsigned int >& Isosurface::get_faces() const
{
  return this->private_->faces_;
}

const std::vector< VectorF >& Isosurface::get_normals() const
{ 
  return this->private_->normals_;
}

const std::vector< float >& Isosurface::get_values() const
{
  return this->private_->values_;
}

bool Isosurface::set_values( const std::vector< float >& values )
{
  if( !( values.size() == this->private_->points_.size() || values.size() == 0 ) )
  {
    return false;
  }
  this->private_->values_ = values; 
  this->private_->values_changed_ = true;
  return true;
}


void Isosurface::set_color_map( ColorMapHandle color_map )
{
  lock_type lock( this->get_mutex() );
  this->private_->color_map_ = color_map;
}

Core::ColorMapHandle Isosurface::get_color_map() const
{
  lock_type lock( this->get_mutex() );
  return this->private_->color_map_;
}

void Isosurface::redraw( bool use_colormap )
{
  lock_type lock( this->get_mutex() );

  // Check for empty isosurface
  if( this->private_->points_.size() == 0 ) 
  {
    return;
  }
  
  this->private_->upload_to_vertex_buffer();
  size_t num_batches = this->private_->part_points_.size();
  bool has_values = this->private_->values_.size() == this->private_->points_.size();
  
  // Error checking
  if( use_colormap ) 
  {
    if( !has_values )
    {
      CORE_LOG_WARNING( "Isosurface colormap enabled, but no per-vertex values assigned." ); 
    }
    if( this->private_->color_map_.get() == 0 )
    {
      CORE_LOG_WARNING( "Isosurface colormap enabled, but no colormap assigned." ); 
    }
  }

  // Use the uploaded VBO for rendering if it's available
  if ( this->private_->vbo_available_ )
  {
    for ( size_t i = 0; i < num_batches; ++i )
    {
      this->private_->vbo_batches_[ i ]->vertex_buffer_->enable_arrays();
      this->private_->vbo_batches_[ i ]->normal_buffer_->enable_arrays();
      if ( has_values && use_colormap )
      {
        this->private_->vbo_batches_[ i ]->value_buffer_->enable_arrays();
      }
      this->private_->vbo_batches_[ i ]->faces_buffer_->draw_elements( GL_TRIANGLES, 
        static_cast< GLsizei >( ( this->private_->part_faces_[ i ].second - 
        this->private_->part_faces_[ i ].first ) ), GL_UNSIGNED_INT );
      this->private_->vbo_batches_[ i ]->vertex_buffer_->disable_arrays();
      this->private_->vbo_batches_[ i ]->normal_buffer_->disable_arrays();
      if ( has_values && use_colormap )
      {
        this->private_->vbo_batches_[ i ]->value_buffer_->disable_arrays();
      }
    }
    return;
  }
  
  RenderResources::lock_type rr_lock( RenderResources::GetMutex() );

  // The isosurface couldn't fit in GPU memory, render it piece by piece.
  VertexAttribArrayBufferHandle vertex_buffer( new VertexAttribArrayBuffer );
  vertex_buffer->set_array( VertexAttribArrayType::VERTEX_E, 3, GL_FLOAT, 0, 0 );
  VertexAttribArrayBufferHandle normal_buffer( new VertexAttribArrayBuffer );
  normal_buffer->set_array( VertexAttribArrayType::NORMAL_E, GL_FLOAT, 0, 0 );
  VertexAttribArrayBufferHandle value_buffer;
  if ( has_values && use_colormap )
  {
    value_buffer.reset( new VertexAttribArrayBuffer );
    value_buffer->set_generic_array( 1, 1, GL_FLOAT, GL_FALSE, 0, 0 );
  }
  ElementArrayBufferHandle face_buffer( new ElementArrayBuffer );
  for ( size_t i = 0; i < num_batches; ++i )
  {
    unsigned int num_pts = this->private_->part_points_[ i ].second - this->private_->part_points_[ i ].first;
    ptrdiff_t vertex_size = num_pts * sizeof( PointF );
    ptrdiff_t normal_size = num_pts * sizeof( VectorF );
    ptrdiff_t value_size = has_values && use_colormap ? num_pts * sizeof( float ) : 0;
    unsigned int num_face_indices = this->private_->part_faces_[ i ].second - this->private_->part_faces_[ i ].first;
    ptrdiff_t face_size = num_face_indices * sizeof( unsigned int );
    
    vertex_buffer->set_buffer_data( vertex_size, 0, GL_STREAM_DRAW );
    void* buffer = vertex_buffer->map_buffer( GL_WRITE_ONLY );
    if ( buffer == 0 )
    {
      CORE_LOG_ERROR( "Failed to map OpenGL buffer, isosurface rendering will"
        " be incomplete!" );
      return;
    }
    memcpy( buffer, &this->private_->points_[ this->private_->part_points_[ i ].first ], vertex_size );
    vertex_buffer->unmap_buffer();
    
    normal_buffer->set_buffer_data( normal_size, 0, GL_STREAM_DRAW );
    buffer = normal_buffer->map_buffer( GL_WRITE_ONLY );
    if ( buffer == 0 )
    {
      CORE_LOG_ERROR( "Failed to map OpenGL buffer, isosurface rendering will"
        " be incomplete!" );
      return;
    }
    memcpy( buffer, &this->private_->normals_[ this->private_->part_points_[ i ].first ], normal_size );
    normal_buffer->unmap_buffer();
    
    if ( has_values && use_colormap )
    {
      value_buffer->set_buffer_data( value_size, 0, GL_STREAM_DRAW );
      buffer = value_buffer->map_buffer( GL_WRITE_ONLY );
      if ( buffer == 0 )
      {
        CORE_LOG_ERROR( "Failed to map OpenGL buffer, isosurface rendering will"
          " be incomplete!" );
        return;
      }
      memcpy( buffer, &this->private_->values_[ this->private_->part_points_[ i ].first ], value_size );
      value_buffer->unmap_buffer();
    }

    face_buffer->set_buffer_data( face_size, 0, GL_STREAM_DRAW );
    buffer = face_buffer->map_buffer( GL_WRITE_ONLY );
    if ( buffer == 0 )
    {
      CORE_LOG_ERROR( "Failed to map OpenGL buffer, isosurface rendering will"
        " be incomplete!" );
      return;
    }
    memcpy( buffer, &this->private_->part_indices_[ i ][ 0 ], face_size );
    face_buffer->unmap_buffer();

    vertex_buffer->enable_arrays();
    normal_buffer->enable_arrays();
    if ( has_values && use_colormap )
    {
      value_buffer->enable_arrays();
    }
    face_buffer->draw_elements( GL_TRIANGLES, 
      static_cast< GLsizei >( ( this->private_->part_faces_[ i ].second - 
      this->private_->part_faces_[ i ].first ) ), GL_UNSIGNED_INT );
    vertex_buffer->disable_arrays();
    normal_buffer->disable_arrays();
    if ( has_values && use_colormap )
    {
      value_buffer->disable_arrays();
    }
  } 
}

bool Isosurface::export_legacy_isosurface( const boost::filesystem::path& path,
                                           const std::string& file_prefix )
{
  lock_type lock( this->get_mutex() );

  // Write points to .pts file
  boost::filesystem::path points_path = path / ( file_prefix + ".pts" );
  std::ofstream pts_file( points_path.string().c_str() );
  if( !pts_file.is_open() ) 
  {
    return false;
  }

  for( size_t i = 0; i < this->private_->points_.size(); i++ )
  {
    PointF pt = this->private_->points_[ i ];
    pts_file << pt.x() << " " << pt.y() << " " << pt.z() << std::endl; 
  }
  pts_file.close();

  // Write faces to .fac file
  boost::filesystem::path faces_path = path / ( file_prefix + ".fac" );
  std::ofstream fac_file( faces_path.string().c_str() );
  if( !fac_file.is_open() ) 
  {
    return false;
  }

  for( size_t i = 0; i + 2 < this->private_->faces_.size(); i += 3 )
  {
    fac_file << this->private_->faces_[ i ] << " " << this->private_->faces_[ i + 1 ] << " " 
      << this->private_->faces_[ i + 2 ] << std::endl; 
  }
  fac_file.close();

  // Write values to .val file
  if( this->private_->values_.size() > 0 )
  {
    boost::filesystem::path values_path = path / ( file_prefix + ".val" );
    std::ofstream val_file( values_path.string().c_str() );
    if( !val_file.is_open() ) 
    {
      return false;
    }

    for( size_t i = 0; i < this->private_->values_.size(); i++ )
    {
      val_file << this->private_->values_[ i ] << std::endl; 
    }
    val_file.close();
  }

  return true;
}


bool Isosurface::export_vtk_isosurface( const boost::filesystem::path& filename )
{
  lock_type lock( this->get_mutex() );

  std::ofstream vtk_file( filename.string().c_str() );
  if( !vtk_file.is_open() ) 
  {
    return false;
  }

  // Legacy VTK file format (http://vtk.org/VTK/img/file-formats.pdf)
  //
  // write header
  vtk_file << "# vtk DataFile Version 3.0\n";
  vtk_file << "vtk output\n";

  vtk_file << "ASCII\n";
  vtk_file << "DATASET POLYDATA\n";
  vtk_file << "POINTS " << this->private_->points_.size() << " float\n";

  for( size_t i = 0; i < this->private_->points_.size(); i++ )
  {
    PointF pt = this->private_->points_[ i ];
    vtk_file << pt.x() << " " << pt.y() << " " << pt.z() << std::endl; 
  }

  unsigned int num_triangles = this->private_->faces_.size() / 3;
  unsigned int triangle_list_size = num_triangles * 4;

  vtk_file << "\nPOLYGONS " << num_triangles << " " << triangle_list_size << std::endl;

  for( size_t i = 0; i + 2 < this->private_->faces_.size(); i += 3 )
  {
    vtk_file << "3 " << this->private_->faces_[ i ] << " " << this->private_->faces_[ i + 1 ] << " " 
      << this->private_->faces_[ i + 2 ] << std::endl; 
  }

  vtk_file.close();

  return true;
}

// ASCII STL format (http://en.wikipedia.org/wiki/STL_(file_format))
bool Isosurface::export_stl_isosurface( const boost::filesystem::path& filename, const std::string& name )
{
  lock_type lock( this->get_mutex() );
  std::ofstream stl_file( filename.string().c_str() );
  if( ! stl_file.is_open() ) 
  {
    return false;
  }

  const std::string delim(" ");
  const std::string indent_level1("  ");
  const std::string indent_level2("    ");
  const std::string indent_level3("      ");
  stl_file << "solid " << name << std::endl;

  for( size_t i = 0; i + 2 < this->private_->faces_.size(); i += 3 )
  {
    size_t vertex_index1 = this->private_->faces_[ i ];
    size_t vertex_index2 = this->private_->faces_[ i + 1 ];
    size_t vertex_index3 = this->private_->faces_[ i + 2 ];

    // Get vertices of face
    PointF p1 = this->private_->points_[ vertex_index1 ];
    PointF p2 = this->private_->points_[ vertex_index2 ];
    PointF p3 = this->private_->points_[ vertex_index3 ];

    // compute face normal:
    //   U = p2 - p1
    //   V = p3 - p1
    //   Ni = UyVz - UzVy
    //   Nj = UzVx - UxVz
    //   Nk = UxVy - UyVx

    VectorF U = p2 - p1;
    VectorF V = p3 - p1;

    double Ni = U.y() * V.z() - U.z() * V.y();
    double Nj = U.z() * V.x() - U.x() * V.z();
    double Nk = U.x() * V.y() - U.y() * V.x();
    
    stl_file << indent_level1 << "facet normal " << std::fixed << Ni << delim << Nj << delim << Nk << std::endl;
    stl_file << indent_level2 << "outer loop" << std::endl;
    stl_file << indent_level3 << "vertex " << std::fixed << p1.x() << delim << p1.y() << delim << p1.z() << std::endl;
    stl_file << indent_level3 << "vertex " << std::fixed << p2.x() << delim << p2.y() << delim << p2.z() << std::endl;
    stl_file << indent_level3 << "vertex " << std::fixed << p3.x() << delim << p3.y() << delim << p3.z() << std::endl;
    stl_file << indent_level2 << "endloop" << std::endl;    
    stl_file << indent_level1 << "endfacet" << std::endl;
  }
  stl_file << "endsolid" << std::endl;

  stl_file.close();
  
  return true;
}

float Isosurface::surface_area() const
{
  lock_type lock( this->get_mutex() );
  return this->private_->area_;
}

} // end namespace Core

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

// Core includes
#include <Core/Isosurface/Isosurface.h>
#include <Core/Utils/StackVector.h>
#include <Core/Utils/Parallel.h>
#include <Core/Utils/Log.h>
#include <Core/Graphics/VertexBufferObject.h>

namespace Core
{

// Marching Cubes tutorial: http://local.wasp.uwa.edu.au/~pbourke/geometry/polygonise/

typedef struct {
  int edges_[15]; // Vertex indices for at most 5 triangles
  int num_triangles_; 
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

class IsosurfacePrivate
{

public:
  // Setup the algorithm and the buffers
  void setup( int num_threads );
  // Parallelized isosurface computation algorithm 
  void parallel_compute_faces( int thread, int num_threads, boost::barrier& barrier );
  void parallel_compute_normals( int thread, int num_threads,  boost::barrier& barrier );

  void upload_to_vertex_buffer();

  // Pointer to public Isosurface -- needed to give access to public signals
  Isosurface* isosurface_;

  // Input to isosurface computation
  MaskVolumeHandle mask_volume_; 

  // Output mesh
  std::vector< PointF > points_; 
  std::vector< VectorF > normals_; 
  std::vector< unsigned int > faces_; // unsigned int because GL expects this

  // Algorithm data & buffers
  unsigned char* data_; // Mask data is stored in bit-plane (8 masks per data block)
  size_t nx_, ny_, nz_; // Mask dimensions
  size_t elem_nx_, elem_ny_, elem_nz_; // Number of (marching) cubes

  std::vector<unsigned char> type_buffer_; 
  std::vector<std::vector<unsigned int> > edge_buffer_;  

  std::vector<std::vector< PointF > > new_points_; 
  std::vector<std::vector< StackVector< size_t, 3 > > > new_elems_;

  std::vector< size_t > front_offset_;
  std::vector< size_t > back_offset_;
  size_t global_point_cnt_;

  VertexAttribArrayBufferHandle vertex_buffer_;
  VertexAttribArrayBufferHandle normal_buffer_;
  ElementArrayBufferHandle faces_buffer_;
  bool surface_changed_;
  bool gl_initialized_;
};

void IsosurfacePrivate::setup( int num_threads )
{
  this->nx_ = this->mask_volume_->get_mask_data_block()->get_nx();
  this->ny_ = this->mask_volume_->get_mask_data_block()->get_ny();
  this->nz_ = this->mask_volume_->get_mask_data_block()->get_nz();
  
  // Number of elements (cubes) in each dimension
  this->elem_nx_ = this->nx_ - 1;
  this->elem_ny_ = this->ny_ - 1;
  this->elem_nz_ = this->nz_ - 1;

  // Mask data is stored in bit-plane (8 masks per data block)
  this->data_ = this->mask_volume_->get_mask_data_block()->get_mask_data();

  // Stores index into polgyon configuration table for each element (cube)?
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
  // Offset allows zero-based indexing over multiple slices
  this->front_offset_.resize( num_threads, 0 );
  this->back_offset_.resize( num_threads, 0 ); 

  // Total number of isosurface points
  this->global_point_cnt_ = 0;
}

/*
Basic ideas:
- Move through volume two slices at a time.  Back and front refer to these two slices.
- Points are shared by multiple triangles in the isosurface.  We don't want to store a copy of a
  point for each triangle.  In order to avoid duplicates, we go through edges in one direction at a 
  time, looking at edges that need to be split.  This way we encounter each edge only once.
- Sort edges into 5 configurations
  - back_buffer_x - Edges along the x direction on the back buffer (slice) 
  - back_buffer_y - Edges along the y direction on the back buffer (slice)
  - front_buffer_x - Edges along the x direction on the front buffer (slice)
  - front_buffer_y - Edges along the y direction on the front buffer (slice)
  - side_buffer - Edges along the sides between the back and front buffers (slices)
- Number points as you encounter split edges.
- These are tables of split edges with indices into a points vector of actual points.
- After edge tables are built, go back to type list, use configurations to lookup into the tables.
- Tables are built in parallel using relative indices per thread.
- Parallel point lists are merged together at the end.
- At end, swap front and back data (front is now back).
- One advantage of this approach is that we don't need complex and confusing linked lists; we can
  use tables that directly correspond to elements.
- Point of confusion: sometimes "element" is are synonymous with "cube" and sometimes it refers
  to a triangle.
*/
void IsosurfacePrivate::parallel_compute_faces( int thread, int num_threads, 
  boost::barrier& barrier )
{
  // Setup the algorithm and the buffers
  if ( thread == 0 ) // Only need to setup once 
  {
    this->setup( num_threads ); 
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

  // Bit where mask bit is stored
  unsigned char mask_value = this->mask_volume_->get_mask_data_block()->get_mask_value(); 

  // Get mask transform from MaskVolume 
  GridTransform grid_transform = this->mask_volume_->get_grid_transform();

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
          if ( data1[ q ] & mask_value )          type |= 0x1;
          if ( data1[ q + 1 ] & mask_value )        type |= 0x2;
          if ( data1[ q + this->nx_ + 1 ] & mask_value )  type |= 0x4;
          if ( data1[ q + this->nx_ ] & mask_value )    type |= 0x8;

          if ( data2[ q ] & mask_value )          type |= 0x10;
          if ( data2[ q + 1 ] & mask_value )        type |= 0x20;
          if ( data2[ q + this->nx_ + 1 ] & mask_value )  type |= 0x40;
          if ( data2[ q + this->nx_ ] & mask_value )    type |= 0x80;

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
    }

    barrier.wait();

    // Build triangles
    for ( size_t y = elem_nystart; y < elem_nyend; y++ )
    {
      for (size_t x=0;x<elem_nx_;x++)
      {
        int elem_offset = y * this->nx_ + x;
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
        }
      }
    }

    std::swap( back_buffer_x, front_buffer_x );
    std::swap( back_buffer_y, front_buffer_y );
    barrier.wait();

    if ( thread == 0 )
    {
      for ( int w = 0;  w < num_threads; w++ )
      {
        std::vector< StackVector< size_t, 3 > >& pelements = this->new_elems_[ w ];
        for ( size_t p = 0; p < pelements.size(); p++ )
        {
          StackVector< size_t, 3 >& el = pelements[ p ];
          this->faces_.push_back( el[ 0 ] );
          this->faces_.push_back( el[ 1 ] );
          this->faces_.push_back( el[ 2 ] );
        }
      }
      this->back_offset_ = this->front_offset_;
    }

    barrier.wait();   

    // Update progress based on number of z slices processed
    this->isosurface_->update_progress_signal_( 
      static_cast< double >( z + 1 ) / static_cast< double >( this->elem_nz_ ) );
  }   

  barrier.wait();
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
  if ( !this->surface_changed_ )
  {
    return;
  }

  if ( !this->gl_initialized_ )
  {
    this->vertex_buffer_.reset( new Core::VertexAttribArrayBuffer );
    this->normal_buffer_.reset( new Core::VertexAttribArrayBuffer );
    this->faces_buffer_.reset( new Core::ElementArrayBuffer );
    this->vertex_buffer_->set_array( VertexAttribArrayType::VERTEX_E, 3, GL_FLOAT, 0, 0 );
    this->normal_buffer_->set_array( VertexAttribArrayType::NORMAL_E, GL_FLOAT, 0, 0 );
    this->gl_initialized_ = true;
  }

  ptrdiff_t vertex_size = this->points_.size() * sizeof( PointF );
  ptrdiff_t normal_size = this->normals_.size() * sizeof( VectorF );
  ptrdiff_t num_of_faces = this->faces_.size() / 3;
  ptrdiff_t face_size = this->faces_.size() * sizeof( unsigned int );
  ptrdiff_t total_size = vertex_size + normal_size + face_size;
  CORE_LOG_MESSAGE( "Uploading vertex data to GPU: " + 
    ExportToString( this->points_.size() ) + " vertices, " +
    ExportToString( num_of_faces ) + " triangles. Total memory: " + 
    ExportToString( total_size ) );
  if ( total_size > ( 50 << 20 ) )
  {
    CORE_LOG_WARNING( "Isosurface data takes more than 10MB!" );
  }
  this->vertex_buffer_->set_buffer_data( vertex_size, &this->points_[ 0 ], GL_STATIC_DRAW );
  this->normal_buffer_->set_buffer_data( normal_size, &this->normals_[ 0 ], GL_STATIC_DRAW );
  this->faces_buffer_->set_buffer_data( face_size, &this->faces_[ 0 ], GL_STATIC_DRAW );
  this->surface_changed_ = false;
}

Isosurface::Isosurface( const MaskVolumeHandle& mask_volume ) :
  private_( new IsosurfacePrivate )
{
  this->private_->isosurface_ = this;
  this->private_->mask_volume_ = mask_volume;
  this->private_->gl_initialized_ = false;
  this->private_->surface_changed_ = false;
}

void Isosurface::compute()
{
  lock_type lock( this->get_mutex() );

  this->private_->points_.clear();
  this->private_->normals_.clear();
  this->private_->faces_.clear();

  {
    Core::MaskVolume::shared_lock_type vol_lock( this->private_->mask_volume_->get_mutex() );

    Parallel parallel_faces( boost::bind( &IsosurfacePrivate::parallel_compute_faces, 
      this->private_, _1, _2, _3 ) );
    parallel_faces.run();
  }

  Parallel parallel_normals( boost::bind( &IsosurfacePrivate::parallel_compute_normals, 
    this->private_, _1, _2, _3 ) );
  parallel_normals.run();

  this->private_->surface_changed_ = true;
}

const std::vector< PointF >& Isosurface::get_points() const
{
  return this->private_->points_;
}

const std::vector< VectorF >& Isosurface::get_normals() const
{ 
  return this->private_->normals_;
}

const std::vector< unsigned int >& Isosurface::get_faces() const
{
  return this->private_->faces_;
}

void Isosurface::redraw()
{
  lock_type lock( this->get_mutex() );
  this->private_->upload_to_vertex_buffer();
  if ( this->private_->gl_initialized_ )
  {
    this->private_->vertex_buffer_->enable_arrays();
    this->private_->normal_buffer_->enable_arrays();
    this->private_->faces_buffer_->draw_elements( GL_TRIANGLES, 
      static_cast< GLsizei >( this->private_->faces_.size() ), GL_UNSIGNED_INT );
    this->private_->normal_buffer_->disable_arrays();
    this->private_->vertex_buffer_->disable_arrays();
  }
}


} // end namespace Core
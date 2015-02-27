/*
 For more information, please see: http://software.sci.utah.edu
 
 The MIT License
 
 Copyright (c) 2013 Scientific Computing and Imaging Institute,
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

// File         : the_grid_transform.cxx
// Author       : Pavel A. Koshevoy
// Created      : Thu Nov 30 13:37:18 MST 2006
// Copyright    : (C) 2004-2008 University of Utah
// Description  : A discontinuous transform -- a uniform grid of vertices is
//                mapped to an image. At each vertex, in addition to image
//                space coordinates, a second set of coordinates is stored.
//                This is similar to texture mapped OpenGL triangle meshes,
//                where the texture coordinates correspond to the image space
//                vertex coordinates.

// local includes:
#include <Core/ITKCommon/Transform/the_grid_transform.hxx>

// system includes:
#include <algorithm>
#include <vector>
#include <math.h>


//----------------------------------------------------------------
// GRID_DENSITY
//
static const int GRID_DENSITY = 1;

//----------------------------------------------------------------
// EPSILON
// 
static const double EPSILON = 1e-6;


//----------------------------------------------------------------
// WARPING
//
#ifdef DEBUG_WARPING
bool WARPING = false;
#endif

//----------------------------------------------------------------
// triangle_t::triangle_t
//
triangle_t::triangle_t()
{
  vertex_[0] = ~0;
  vertex_[1] = ~0;
  vertex_[2] = ~0;
}

//----------------------------------------------------------------
// triangle_intersect
// 
inline static bool
triangle_intersect(const double * pwb,
                   const double * pwc,
                   const double & pt_x,
                   const double & pt_y,
                   double * barycentric_coords)
{
  double wa = barycentric_coords[0];
  double wb = barycentric_coords[1];
  double wc = barycentric_coords[2];

  wb = pwb[0] * pt_x + pwb[1] * pt_y + pwb[2];
  wc = pwc[0] * pt_x + pwc[1] * pt_y + pwc[2];

  // corner cases -- clamp when within tolerance:
  if (wb < 0 && wb > -EPSILON) wb = 0;
  if (wb > 1 && (wb - 1) < EPSILON) wb = 1;
  if (wc < 0 && wc > -EPSILON) wc = 0;
  if (wc > 1 && (wc - 1) < EPSILON) wc = 1;
  
  wa = 1.0 - (wb + wc);
  if (wa < 0 && wa > -EPSILON) wa = 0;
  if (wa > 1 && (wa - 1) < EPSILON) wa = 1;
  
  return (wa >= 0.0 && wb >= 0.0 && wc >= 0.0);
}

//----------------------------------------------------------------
// triangle_t::intersect
//
bool
triangle_t::xy_intersect(const vertex_t * v_arr,
                         const pnt2d_t & xy,
                         pnt2d_t & uv) const
{
  double barycentric_coords[3] = {};
  if (! triangle_intersect(xy_pwb, xy_pwc, xy[0], xy[1], barycentric_coords))
  {
    return false;
  }

  double wa = barycentric_coords[0];
  double wb = barycentric_coords[1];
  double wc = barycentric_coords[2];
  
  const pnt2d_t & A = v_arr[vertex_[0]].uv_;
  const pnt2d_t & B = v_arr[vertex_[1]].uv_;
  const pnt2d_t & C = v_arr[vertex_[2]].uv_;
  
  uv[0] = A[0] * wa + B[0] * wb + C[0] * wc;
  uv[1] = A[1] * wa + B[1] * wb + C[1] * wc;
  
#ifdef DEBUG_WARPING
  static const double THRESHOLD = 5e-2;
  if (WARPING && (fabs(wa) < THRESHOLD ||
                  fabs(wb) < THRESHOLD ||
                  fabs(wc) < THRESHOLD))
  {
    uv[0] = 0.0;
    uv[1] = 0.0;
  }
#endif // DEBUG_WARPING
  
  return true;
}

//----------------------------------------------------------------
// triangle_t::uv_intersect
// 
bool
triangle_t::uv_intersect(const vertex_t * v_arr,
                         const pnt2d_t & uv,
                         pnt2d_t & xy) const
{  
  double barycentric_coords[3] = {};
  if (!triangle_intersect(uv_pwb, uv_pwc, uv[0], uv[1], barycentric_coords))
  {
//#if 0
//    // for debugging:
//    static int count = 0;
//    count++;
//    printf("%3i. %+e %+e %+e,   %e\n", count, wa, wb, wc,
//           fabs(wa) + fabs(wb) + fabs(wc));
//    triangle_intersect(uv_pwb, uv_pwc, uv[0], uv[1], barycentric_coords);
//#endif
    
    return false;
  }

  double wa = barycentric_coords[0];
  double wb = barycentric_coords[1];
  double wc = barycentric_coords[2];
  
  const pnt2d_t & A = v_arr[vertex_[0]].xy_;
  const pnt2d_t & B = v_arr[vertex_[1]].xy_;
  const pnt2d_t & C = v_arr[vertex_[2]].xy_;
  
  xy[0] = A[0] * wa + B[0] * wb + C[0] * wc;
  xy[1] = A[1] * wa + B[1] * wb + C[1] * wc;
  
  return true;
}


//----------------------------------------------------------------
// the_acceleration_grid_t::the_acceleration_grid_t
// 
the_acceleration_grid_t::the_acceleration_grid_t():
  rows_(0),
  cols_(0)
{
  // reset the grid bounding box:
  xy_min_[0] = std::numeric_limits<double>::max();
  xy_min_[1] = xy_min_[0];
  xy_ext_[0] = 0.0;
  xy_ext_[1] = 0.0;
}

//----------------------------------------------------------------
// the_acceleration_grid_t::xy_cell
//
unsigned int
the_acceleration_grid_t::xy_cell(const pnt2d_t & xy) const
{
  // find where in the grid the point lands:
  double a = (xy[0] - xy_min_[0]) / xy_ext_[0];
  double b = (xy[1] - xy_min_[1]) / xy_ext_[1];
  if (a >= 0.0 && a <= 1.0 && b >= 0.0 && b <= 1.0)
  {
    double c = std::min(double(cols_ - 1), a * double(cols_));
    double r = std::min(double(rows_ - 1), b * double(rows_));

    unsigned int col = (unsigned int)(floor(c));
    unsigned int row = (unsigned int)(floor(r));

    return row * cols_ + col;
  }

  return ~0;
}

//----------------------------------------------------------------
// the_acceleration_grid_t::xy_triangle
//
unsigned int
the_acceleration_grid_t::xy_triangle(const pnt2d_t & xy, pnt2d_t & uv) const
{
  unsigned int cell_id = xy_cell(xy);

  if (cell_id != (unsigned int)(~0))
  {
    // shortcuts:
    const std::list<unsigned int> & cell = xy_[cell_id];
    const vertex_t * v_arr = &(mesh_[0]);

    // check each candidate triangle for intersection:
    for (std::list<unsigned int>::const_iterator iter = cell.begin();
         iter != cell.end(); ++iter)
    {
      const triangle_t & tri = tri_[*iter];
      if (tri.xy_intersect(v_arr, xy, uv))
      {
        return *iter;
      }
    }
  }

  return ~0;
}

//----------------------------------------------------------------
// the_acceleration_grid_t::uv_cell
//
unsigned int
the_acceleration_grid_t::uv_cell(const pnt2d_t & uv) const
{
  // find where in the grid the point lands:
  double a = uv[0];
  double b = uv[1];
  if (a >= 0.0 && a <= 1.0 && b >= 0.0 && b <= 1.0)
  {
    double c = std::min(double(cols_ - 1), a * double(cols_));
    double r = std::min(double(rows_ - 1), b * double(rows_));

    unsigned int col = (unsigned int)(floor(c));
    unsigned int row = (unsigned int)(floor(r));

    return row * cols_ + col;
  }

  return ~0;
}

//----------------------------------------------------------------
// the_acceleration_grid_t::uv_triangle
//
unsigned int
the_acceleration_grid_t::uv_triangle(const pnt2d_t & uv, pnt2d_t & xy) const
{
  unsigned int cell_id = uv_cell(uv);

  if (cell_id != (unsigned int)(~0))
  {
    // shortcuts:
    const std::list<unsigned int> & cell = uv_[cell_id];
    const vertex_t * v_arr = &(mesh_[0]);

    // check each candidate triangle for intersection:
    for (std::list<unsigned int>::const_iterator iter = cell.begin();
         iter != cell.end(); ++iter)
    {
      const triangle_t & tri = tri_[*iter];
      if (tri.uv_intersect(v_arr, uv, xy))
      {
        return *iter;
      }
    }
  }

  return ~0;
}

//----------------------------------------------------------------
// the_acceleration_grid_t::update
//
void
the_acceleration_grid_t::update(const vec2d_t * xy_shift)
{
  unsigned int num_verts = mesh_.size();
  vertex_t * v_arr = &(mesh_[0]);
  for (unsigned int i = 0; i < num_verts; i++)
  {
    // cerr << v_arr[i].xy_ << " -> ";
    v_arr[i].xy_ += xy_shift[i];
    // cerr << v_arr[i].xy_ << endl;
  }

  rebuild();
}

//----------------------------------------------------------------
// the_acceleration_grid_t::shift
// 
void
the_acceleration_grid_t::shift(const vec2d_t & xy_shift)
{
  unsigned int num_verts = mesh_.size();
  vertex_t * v_arr = &(mesh_[0]);
  for (unsigned int i = 0; i < num_verts; i++)
  {
    // cerr << v_arr[i].xy_ << " -> ";
    v_arr[i].xy_ += xy_shift;
    // cerr << v_arr[i].xy_ << endl;
  }

  rebuild();
}

//----------------------------------------------------------------
// the_acceleration_grid_t::resize
// 
void
the_acceleration_grid_t::resize(unsigned int rows,
                                unsigned int cols)
{
  // reset the grid bounding box:
  xy_min_[0] = std::numeric_limits<double>::max();
  xy_min_[1] = xy_min_[0];
  xy_ext_[0] = 0.0;
  xy_ext_[1] = 0.0;
  
  rows_ = rows;
  cols_ = cols;
  xy_.resize(rows_ * cols_);
  uv_.resize(rows_ * cols_);
}

//----------------------------------------------------------------
// the_acceleration_grid_t::rebuild
//
void
the_acceleration_grid_t::rebuild()
{
  // reset the grid bounding box and destroy the the old grid:
  xy_min_[0] = std::numeric_limits<double>::max();
  xy_min_[1] = xy_min_[0];
  
  pnt2d_t xy_max;
  xy_max[0] = -xy_min_[0];
  xy_max[1] = -xy_min_[1];

  // find the new grid bounding box:
  unsigned int num_verts = mesh_.size();
  vertex_t * v_arr = &(mesh_[0]);
  for (unsigned int i = 0; i < num_verts; i++)
  {
    update_bbox(xy_min_, xy_max, v_arr[i].xy_);
  }
  xy_ext_ = xy_max - xy_min_;

#if 0
  cerr << "xy_min: " << xy_min_ << endl
       << "xy_max: " << xy_max << endl
       << "xy_ext: " << xy_ext_ << endl;
#endif
  
  // reset to empty grid:
  xy_.assign(xy_.size(), std::list<unsigned int>());
  uv_.assign(uv_.size(), std::list<unsigned int>());
  
  // add triangles to the grid:
  unsigned int num_triangles = tri_.size();
  for (unsigned int i = 0; i < num_triangles; i++)
  {
    update_grid(i);
  }

#if 0
  int count = 0;
  for (unsigned int row = 0; row < rows_; row++)
  {
    for (unsigned int col = 0; col < cols_; col++)
    {
      count += xy_[row * cols_ + col].size();
      cout << setw(3) << xy_[row * cols_ + col].size();
    }
    cout << endl;
  }

  assert(count > 0);
#endif
}

//----------------------------------------------------------------
// intersect_lines_2d
//
// Find the intersection parameters (u, v)  of 2 parametric lines
//   A = a + b * u
//   B = c + d * v
//
static bool
intersect_lines_2d(// start point A:
                   double ax,
                   double ay,

                   // direction vector A:
                   double bx,
                   double by,

                   // start point B:
                   double cx,
                   double cy,

                   // direction vector B:
                   double dx,
                   double dy,

                   // results:
                   double & u,
                   double & v)
{
  // solve the linear system Ax = b:
  //
  // [bx  -dx] [u] = [cx - ax]
  // [by  -dy] [v]   [cy - ay]
  //

  double det_A = dx * by - bx * dy;
  if (det_A == 0.0) return false;

  double det_A_inv = 1.0 / det_A;
  double A_inv[][2] = {
    { -dy * det_A_inv, dx * det_A_inv },
    { -by * det_A_inv, bx * det_A_inv }
  };

  double b[] = { cx - ax, cy - ay };
  u = A_inv[0][0] * b[0] + A_inv[0][1] * b[1];
  v = A_inv[1][0] * b[0] + A_inv[1][1] * b[1];

  return true;
}

//----------------------------------------------------------------
// intersect_bbox_triangle
//
static bool
intersect_bbox_triangle(const pnt2d_t & min,
                        const pnt2d_t & max,
                        const pnt2d_t & A,
                        const pnt2d_t & B,
                        const pnt2d_t & C)
{
  double lines_a[][4] = {
    // horizontal:
    { min[0], min[1], max[0] - min[0], 0.0 },
    { min[0], max[1], max[0] - min[0], 0.0 },
    // vertical:
    { min[0], min[1], 0.0, max[1] - min[1] },
    { max[0], min[1], 0.0, max[1] - min[1] }
  };

  double lines_b[][4] = {
    { A[0], A[1], B[0] - A[0], B[1] - A[1] },
    { A[0], A[1], C[0] - A[0], C[1] - A[1] },
    { B[0], B[1], C[0] - B[0], C[1] - B[1] }
  };

  double u;
  double v;
  for (unsigned int i = 0; i < 4; i++)
  {
    for (unsigned int j = 0; j < 3; j++)
    {
      if (intersect_lines_2d(lines_a[i][0],
                             lines_a[i][1],
                             lines_a[i][2],
                             lines_a[i][3],

                             lines_b[j][0],
                             lines_b[j][1],
                             lines_b[j][2],
                             lines_b[j][3],

                             u,
                             v))
      {
        if (u >= 0.0 && u <= 1.0 && v >= 0.0 && v <= 1.0)
        {
          return true;
        }
      }
    }
  }

  return false;
}

//----------------------------------------------------------------
// update_grid
// 
static void
update_grid(// the acceleration grid:
            std::list<unsigned int> * grid,
            const unsigned int rows,
            const unsigned int cols,
            
            // acceleration grid bounding box:
            const pnt2d_t & grid_min,
            const vec2d_t & grid_ext,
            
            // the triangle being added to the grid:
            const pnt2d_t & A,
            const pnt2d_t & B,
            const pnt2d_t & C,
            const unsigned int tri_id,
            
            // calculate fast barycentric coordinate calculation coefficients:
            double * pwb,
            double * pwc)
{
  // precompute the barycentric calculation coefficients:
  {
    vec2d_t b = B - A;
    vec2d_t c = C - A;
    double bycx_bxcy = b[1] * c[0] - b[0] * c[1];

    pwb[0] = -c[1] / bycx_bxcy;
    pwb[1] = c[0] / bycx_bxcy;
    pwb[2] = (c[1] * A[0] - c[0] * A[1]) / bycx_bxcy;

    pwc[0] = b[1] / bycx_bxcy;
    pwc[1] = -b[0] / bycx_bxcy;
    pwc[2] = (b[0] * A[1] - b[1] * A[0]) / bycx_bxcy;
  }

  pnt2d_t min = A;
  pnt2d_t max = min;
  update_bbox(min, max, B);
  update_bbox(min, max, C);

  const double & gw = grid_ext[0];
  double a[] = {
    (min[0] - grid_min[0]) / gw,
    (max[0] - grid_min[0]) / gw
  };

  unsigned int c[] = {
    std::min(cols - 1, (unsigned int)(floor(a[0] * double(cols)))),
    std::min(cols - 1, (unsigned int)(floor(a[1] * double(cols))))
  };

  const double & gh = grid_ext[1];
  double b[] = {
    (min[1] - grid_min[1]) / gh,
    (max[1] - grid_min[1]) / gh
  };

  unsigned int r[] = {
    std::min(rows - 1, (unsigned int)(floor(b[0] * double(rows)))),
    std::min(rows - 1, (unsigned int)(floor(b[1] * double(rows))))
  };
  
  // temporary barycentric coordinates of point inside the triangle:
  double barycentric_coords[3] = {};
  
  for (unsigned int row = r[0]; row <= r[1]; row++)
  {
    for (unsigned int col = c[0]; col <= c[1]; col++)
    {
      unsigned int i = row * cols + col;
      std::list<unsigned int> & cell = grid[i];
      
      // calculate the bounding box of this grid cell:
      pnt2d_t min(grid_min);
      min[0] += gw * double(col) / double(cols);
      min[1] += gh * double(row) / double(rows);
      
      pnt2d_t max(grid_min);
      max[0] += gw * double(col + 1) / double(cols);
      max[1] += gh * double(row + 1) / double(rows);
      
      if (inside_bbox(min, max, A) ||
          inside_bbox(min, max, B) ||
          inside_bbox(min, max, C) ||
          triangle_intersect(pwb, pwc, min[0], min[1], barycentric_coords) ||
          triangle_intersect(pwb, pwc, max[0], min[1], barycentric_coords) ||
          triangle_intersect(pwb, pwc, max[0], max[1], barycentric_coords) ||
          triangle_intersect(pwb, pwc, min[0], max[1], barycentric_coords) ||
          intersect_bbox_triangle(min, max, A, B, C))
      {
        cell.push_back(tri_id);
      }
    }
  }
}

//----------------------------------------------------------------
// the_acceleration_grid_t::update_grid
//
void
the_acceleration_grid_t::update_grid(unsigned int t_idx)
{
  // shortcuts:
  triangle_t & tri = tri_[t_idx];
  const vertex_t * v_arr = &(mesh_[0]);
  const vertex_t & v0 = v_arr[tri.vertex_[0]];
  const vertex_t & v1 = v_arr[tri.vertex_[1]];
  const vertex_t & v2 = v_arr[tri.vertex_[2]];

  // update the xy-grid:
  ::update_grid(&(xy_[0]),
                rows_,
                cols_,

                // xy-grid bbox:
                xy_min_,
                xy_ext_,
                
                // xy-triangle:
                v0.xy_,
                v1.xy_,
                v2.xy_,
                
                t_idx,
                tri.xy_pwb,
                tri.xy_pwc);
  
  // update the uv-grid:
  ::update_grid(&(uv_[0]),
                rows_,
                cols_,
                
                // uv-grid bbox:
                pnt2d(0, 0),
                vec2d(1, 1),
                
                // xy-triangle:
                v0.uv_,
                v1.uv_,
                v2.uv_,
                
                t_idx,
                tri.uv_pwb,
                tri.uv_pwc);
}


//----------------------------------------------------------------
// the_base_triangle_transform_t::transform
//
bool
the_base_triangle_transform_t::transform(const pnt2d_t & xy,
                                         pnt2d_t & uv) const
{
  unsigned int t_id = grid_.xy_triangle(xy, uv);

  if (t_id == (unsigned int)(~0))
  {
    uv[0] = std::numeric_limits<double>::quiet_NaN();
    uv[1] = uv[0];
  }

  return t_id != (unsigned int)(~0);
}

//----------------------------------------------------------------
// the_base_triangle_transform_t::transform_inv
//
bool
the_base_triangle_transform_t::transform_inv(const pnt2d_t & uv,
                                             pnt2d_t & xy) const
{
  unsigned int t_id = grid_.uv_triangle(uv, xy);

  if (t_id == (unsigned int)(~0))
  {
#if 0
    // for debugging:
    grid_.uv_triangle(uv, xy);
#endif
    
    xy[0] = std::numeric_limits<double>::quiet_NaN();
    xy[1] = xy[0];
  }

  return t_id != (unsigned int)(~0);
}

//----------------------------------------------------------------
// the_base_triangle_transform_t::jacobian
//
bool
the_base_triangle_transform_t::jacobian(const pnt2d_t & P,
                                        unsigned int * idx,
                                        double * jac) const
{
  pnt2d_t uv;
  unsigned int t_id = grid_.xy_triangle(P, uv);
  if (t_id == (unsigned int)(~0)) return false;

  const triangle_t & tri = grid_.tri_[t_id];
  idx[0] = tri.vertex_[0];
  idx[1] = tri.vertex_[1];
  idx[2] = tri.vertex_[2];

  const vertex_t & A = grid_.mesh_[idx[0]];
  const vertex_t & B = grid_.mesh_[idx[1]];
  const vertex_t & C = grid_.mesh_[idx[2]];

  // calculate partial derivatrives of wB and wC with respect to A, B, C:
  double dw[2][6];
  {
    const double & Ax = A.xy_[0];
    const double & Ay = A.xy_[1];
    const double & Bx = B.xy_[0];
    const double & By = B.xy_[1];
    const double & Cx = C.xy_[0];
    const double & Cy = C.xy_[1];
    const double & Px = P[0];
    const double & Py = P[1];

    double bx = Bx - Ax;
    double cx = Cx - Ax;
    double px = Px - Ax;

    double by = By - Ay;
    double cy = Cy - Ay;
    double py = Py - Ay;

    double pycx_pxcy = py * cx - px * cy;
    double pxby_pybx = px * by - py * bx;
    double bycx_bxcy = by * cx - bx * cy;
    double bycx_bxcy_2 = bycx_bxcy * bycx_bxcy;

    // dwB/dAx, dwB/dAy:
    dw[0][0] = (Cy - Py) / bycx_bxcy - (Cy - By) / bycx_bxcy_2;
    dw[0][1] = (Px - Cx) / bycx_bxcy - (Bx - Cx) / bycx_bxcy_2;

    // dwB/dBx, dwB/dBy:
    dw[0][2] = cy * pycx_pxcy / bycx_bxcy_2;
    dw[0][3] = -cx * pycx_pxcy / bycx_bxcy_2;

    // dwB/dCx, dwB/dCy:
    dw[0][4] = py / bycx_bxcy - by * pycx_pxcy / bycx_bxcy_2;
    dw[0][5] = bx * pycx_pxcy / bycx_bxcy_2 - px / bycx_bxcy;

    // dwC/dAx, dwC/dAy:
    dw[1][0] = (Py - By) / bycx_bxcy - (Cy - By) * pxby_pybx / bycx_bxcy_2;
    dw[1][1] = (Bx - Px) / bycx_bxcy - (Bx - Cx) * pxby_pybx / bycx_bxcy_2;

    // dwC/dBx, dwC/dBy:
    dw[1][2] = cy * pxby_pybx / bycx_bxcy_2 - py / bycx_bxcy;
    dw[1][3] = px / bycx_bxcy - cx * pxby_pybx / bycx_bxcy_2;

    // dwC/dCx, dwC/dCy:
    dw[1][4] = -by * pxby_pybx / bycx_bxcy_2;
    dw[1][5] = bx * pxby_pybx / bycx_bxcy_2;
  }

  double bu = B.uv_[0] - A.uv_[0];
  double bv = B.uv_[1] - A.uv_[1];
  double cu = C.uv_[0] - A.uv_[0];
  double cv = C.uv_[1] - A.uv_[1];

  // shortcut:
  double * du = &(jac[0]);
  double * dv = &(jac[6]);

  for (unsigned int i = 0; i < 6; i++)
  {
    du[i] = bu * dw[0][i] + cu * dw[1][i];
    dv[i] = bv * dw[0][i] + cv * dw[1][i];
  }

  return true;
}


//----------------------------------------------------------------
// the_grid_transform_t::the_grid_transform_t
//
the_grid_transform_t::the_grid_transform_t():
  rows_(0),
  cols_(0)
{}

//----------------------------------------------------------------
// the_grid_transform_t::is_ready
// 
bool
the_grid_transform_t::is_ready() const
{
  unsigned int n = rows_ * cols_;
  return (n != 0) && (grid_.tri_.size() == 2 * n);
}

//----------------------------------------------------------------
// the_grid_transform_t::transform_inv
//
bool
the_grid_transform_t::transform_inv(const pnt2d_t & uv, pnt2d_t & xy) const
{
#if 0
  const double & c = uv[0];
  int c0 = int(floor(c * double(cols_)));
  int c1 = c0 + 1;
  if (c0 < 0 || c0 > int(cols_)) return false;
  if (c0 == int(cols_))
  {
    c0--;
    c1--;
  }
  
  const double & r = uv[1];
  int r0 = int(floor(r * double(rows_)));
  int r1 = r0 + 1;
  if (r0 < 0 || r0 > int(rows_)) return false;
  if (r0 == int(rows_))
  {
    r0--;
    r1--;
  }
  
  double w[4];
  {
    double wx[] = {
      double(c1) - c * double(cols_),
      c * double(cols_) - double(c0)
    };

    double wy[] = {
      double(r1) - r * double(rows_),
      r * double(rows_) - double(r0)
    };

    w[0] = wx[0] * wy[0];
    w[1] = wx[1] * wy[0];
    w[2] = wx[1] * wy[1];
    w[3] = wx[0] * wy[1];
  }

  xy[0] =
    vertex(r0, c0).xy_[0] * w[0] +
    vertex(r0, c1).xy_[0] * w[1] +
    vertex(r1, c1).xy_[0] * w[2] +
    vertex(r1, c0).xy_[0] * w[3];

  xy[1] =
    vertex(r0, c0).xy_[1] * w[0] +
    vertex(r0, c1).xy_[1] * w[1] +
    vertex(r1, c1).xy_[1] * w[2] +
    vertex(r1, c0).xy_[1] * w[3];

  return true;
  
#else
  
  return the_base_triangle_transform_t::transform_inv(uv, xy);
#endif
}

//----------------------------------------------------------------
// the_grid_transform_t::setup
//
void
the_grid_transform_t::setup(unsigned int rows,
                            unsigned int cols,
                            const pnt2d_t & tile_min,
                            const pnt2d_t & tile_max,
                            const std::vector<pnt2d_t> & xy)
{
  rows_ = rows;
  cols_ = cols;

  tile_min_ = tile_min;
  tile_ext_ = tile_max - tile_min;

  grid_.mesh_.resize((rows_ + 1) * (cols_ + 1));
  grid_.resize(rows_ * GRID_DENSITY,
               cols_ * GRID_DENSITY);

  // temporaries:
  pnt2d_t uv;

  for (unsigned int row = 0; row <= rows_; row++)
  {
    uv[1] = double(row) / double(rows_);
    for (unsigned int col = 0; col <= cols_; col++)
    {
      uv[0] = double(col) / double(cols_);

      vertex_t & vx = vertex(row, col);
      vx.uv_ = uv;
      vx.xy_ = xy[row * (cols_ + 1) + col];
    }
  }

  setup_mesh();
}

//----------------------------------------------------------------
// the_grid_transform_t::setup_mesh
//
void
the_grid_transform_t::setup_mesh()
{
  grid_.tri_.resize(rows_ * cols_ * 2);

  // temporaries:
  unsigned int v_idx[4];
  unsigned int t_idx[2];

  for (unsigned int row = 0; row < rows_; row++)
  {
    for (unsigned int col = 0; col < cols_; col++)
    {
      // vertex indices:
      v_idx[0] = (cols_ + 1) * row + col;
      v_idx[1] = v_idx[0] + (cols_ + 1);
      v_idx[2] = v_idx[1] + 1;
      v_idx[3] = v_idx[0] + 1;

      // triangle indices:
      t_idx[0] = (cols_ * row + col) * 2;
      t_idx[1] = t_idx[0] + 1;

      // setup triangle A:
      triangle_t & tri_a = grid_.tri_[t_idx[0]];
      tri_a.vertex_[0] = v_idx[0];
      tri_a.vertex_[1] = v_idx[1];
      tri_a.vertex_[2] = v_idx[2];

      // setup triangle B:
      triangle_t & tri_b = grid_.tri_[t_idx[1]];
      tri_b.vertex_[0] = v_idx[0];
      tri_b.vertex_[1] = v_idx[2];
      tri_b.vertex_[2] = v_idx[3];
    }
  }
  
  // initialize the acceleration grid:
  grid_.rebuild();
}


//----------------------------------------------------------------
// the_mesh_transform_t::is_ready
// 
bool
the_mesh_transform_t::is_ready() const
{
  bool ready = (grid_.tri_.size() > 1);
  return ready;
}

//----------------------------------------------------------------
// the_mesh_transform_t::setup
//
bool
the_mesh_transform_t::setup(const pnt2d_t & tile_min,
                            const pnt2d_t & tile_max,
                            const std::vector<pnt2d_t> & uv,
                            const std::vector<pnt2d_t> & xy,
                            unsigned int accel_grid_rows,
                            unsigned int accel_grid_cols)
{
  const std::size_t num_pts = uv.size();
  
  tile_min_ = tile_min;
  tile_ext_ = tile_max - tile_min;

  grid_.mesh_.resize(num_pts);
  grid_.resize(accel_grid_rows,
               accel_grid_cols);

  for (unsigned int i = 0; i < num_pts; i++)
  {
    vertex_t & vx = grid_.mesh_[i];
    vx.uv_ = uv[i];
    vx.xy_ = xy[i];
  }

  return setup_mesh();
}

//----------------------------------------------------------------
// the_mesh_transform_t::insert_point
// 
bool
the_mesh_transform_t::insert_point(const pnt2d_t & uv,
                                   const pnt2d_t & xy,
                                   const bool delay_setup)
{
  vertex_t vx;
  vx.uv_ = uv;
  vx.xy_ = xy;
  
  // Check for duplicate entries.
  std::vector<vertex_t>::iterator iter = grid_.mesh_.begin();
  for ( ; iter != grid_.mesh_.end(); ++iter )
  {
    if ( vx.uv_ == (*iter).uv_ && vx.xy_ == (*iter).xy_ )
      break;
  }

  // We can't allow duplicates, as it breaks the triangulation algorithm.
  if ( iter != grid_.mesh_.end() )
    return false;
  
  grid_.mesh_.push_back(vx);
  
  // At times we want to add a bunch of points before performing
  // Delauney.  By waiting we can save cycles.
  return ( delay_setup ) ? false : setup_mesh();
}

//----------------------------------------------------------------
// the_mesh_transform_t::insert_point
// 
bool
the_mesh_transform_t::insert_point(const pnt2d_t & uv)
{
  pnt2d_t xy;
  if (!transform_inv(uv, xy))
  {
    return false;
  }
  
  return insert_point(uv, xy);
}


//----------------------------------------------------------------
// CircumCircle
// 
// Credit to Paul Bourke (pbourke@swin.edu.au)
// for the original Fortran 77 Program :))
// 
// Check out http://local.wasp.uwa.edu.au/~pbourke/papers/triangulate/index.html
// You can use this code however you like providing the above credits
// remain in tact.
// 
// Return true if a point (xp, yp) is inside the circumcircle
// made up of the points (x1, y1), (x2, y2), (x3, y3)
// 
// The circumcircle centre is returned in (xc,yc) and the radius r
// 
// NOTE: A point on the edge is inside the circumcircle
// 
static bool
CircumCircle(double xp, double yp,
             double x1, double y1,
             double x2, double y2,
             double x3, double y3,
             double * xc, double * yc,
             double * rsqr)
{
  double fabsy1y2 = fabs(y1 - y2);
  double fabsy2y3 = fabs(y2 - y3);

  // Check for coincident points
  if (fabsy1y2 < EPSILON && fabsy2y3 < EPSILON)
  {
    return (false);
  }

  // temporaries:
  double m1, m2, mx1, mx2, my1, my2;
  
  if (fabsy1y2 < EPSILON)
  {
    m2 = -(x3 - x2) / (y3 - y2);
    mx2 = (x2 + x3) / 2.0;
    my2 = (y2 + y3) / 2.0;
    *xc = (x2 + x1) / 2.0;
    *yc = m2 * (*xc - mx2) + my2;
  }
  else if (fabsy2y3 < EPSILON)
  {
    m1 = -(x2 - x1) / (y2 - y1);
    mx1 = (x1 + x2) / 2.0;
    my1 = (y1 + y2) / 2.0;
    *xc = (x3 + x2) / 2.0;
    *yc = m1 * (*xc - mx1) + my1;
  }
  else
  {
    m1 = - (x2 - x1) / (y2 - y1);
    m2 = - (x3 - x2) / (y3 - y2);
    mx1 = (x1 + x2) / 2.0;
    mx2 = (x2 + x3) / 2.0;
    my1 = (y1 + y2) / 2.0;
    my2 = (y2 + y3) / 2.0;
    *xc = (m1 * mx1 - m2 * mx2 + my2 - my1) / (m1 - m2);
    
    if (fabsy1y2 > fabsy2y3)
    {
      *yc = m1 * (*xc - mx1) + my1;
    }
    else
    {
      *yc = m2 * (*xc - mx2) + my2;
    }
  }

  double dx = x2 - *xc;
  double dy = y2 - *yc;
  *rsqr = dx * dx + dy * dy;
  
  dx = xp - *xc;
  dy = yp - *yc;
  
  double drsqr = dx*dx + dy*dy;
  return (drsqr <= *rsqr);
}

//----------------------------------------------------------------
// TVertex
// 
typedef struct
{
  double x, y;
  unsigned int idx;
} TVertex;

//----------------------------------------------------------------
// TTriangle
// 
typedef struct
{
  int p1, p2, p3;
} TTriangle;

//----------------------------------------------------------------
// TEdge
// 
typedef struct
{
  int p1, p2;
} TEdge;

//----------------------------------------------------------------
// vertex_sorter_t
// 
static bool
TVertexCompare(const TVertex & a, const TVertex & b)
{
  // sort the vertices on the x-coordinate, in ascending order:
  return a.x < b.x;
}

//----------------------------------------------------------------
// Triangulate
// 
// Credit to Paul Bourke (pbourke@swin.edu.au)
// for the original Fortran 77 Program :))
// 
// Check out http://local.wasp.uwa.edu.au/~pbourke/papers/triangulate/index.html
// You can use this code however you like providing the above credits
// remain in tact.
// 
// Takes as input a number of vertices in vertex array
// Passes back a list of triangular faces
// 
// The triangles are arranged in a consistent clockwise order.
// The triangle array should be allocated to hold 2 * (nv + 3) - 2 triangles
// 
// The vertex array must be big enough to hold 3 extra vertices
// (used internally for the supertiangle)
// 
// The vertex array must be sorted in increasing x values
// 
static int
Triangulate(int num_vertices,
            TVertex * vertex,
            TTriangle * tri,
            const int max_triangles,
            int & num_triangles)
{
  // Allocate memory for the completeness list, flag for each triangle
  std::vector<int> complete;
  complete.assign(max_triangles, false);
  
  // Allocate memory for the edge list
  int emax = 200;
  std::vector<TEdge> edges(emax);
  
  // Set up the supertriangle
  //
  // This is a triangle which encompasses all the sample points.
  // The supertriangle coordinates are added to the end of the
  // vertex list. The supertriangle is the first triangle in
  // the triangle list.
  //
  {
    // Given a bounding box of all the vertices we can
    // construct a bounding circle.  Given the bounding circle we can
    // construct a bounding triangle.

    // find the bounding box:
    double xmin = vertex[0].x;
    double ymin = vertex[0].y;
    double xmax = xmin;
    double ymax = ymin;
    
    for (int i = 1; i < num_vertices; i++)
    {
      if (vertex[i].x < xmin) xmin = vertex[i].x;
      if (vertex[i].x > xmax) xmax = vertex[i].x;
      if (vertex[i].y < ymin) ymin = vertex[i].y;
      if (vertex[i].y > ymax) ymax = vertex[i].y;
    }
    
    // find the (inflated) bounding circle:
    double dx = xmax - xmin;
    double dy = ymax - ymin;
    
    double xmid = (xmax + xmin) / 2.0;
    double ymid = (ymax + ymin) / 2.0;
    
    double inflate = 1.1;
    double r = (sqrt(dx * dx + dy * dy) / 2.0) * inflate;
    
    // find the bounding triangle with clockwise winding:
    static const double sqrt3 = sqrt(3.0);
    
    vertex[num_vertices + 0].x = xmid - r * sqrt3;
    vertex[num_vertices + 0].y = ymid - r;
    vertex[num_vertices + 0].idx = num_vertices;
    
    vertex[num_vertices + 1].x = xmid;
    vertex[num_vertices + 1].y = ymid + r * 2.0;
    vertex[num_vertices + 1].idx = num_vertices + 1;
    
    vertex[num_vertices + 2].x = xmid + r * sqrt3;
    vertex[num_vertices + 2].y = ymid - r;
    vertex[num_vertices + 2].idx = num_vertices + 2;
    
    tri[0].p1 = num_vertices;
    tri[0].p2 = num_vertices + 1;
    tri[0].p3 = num_vertices + 2;
    
    complete[0] = false;
    num_triangles = 1;
  }
  
  // temporaries:
  double xp = 0;
  double yp = 0;
  double x1 = 0;
  double y1 = 0;
  double x2 = 0;
  double y2 = 0;
  double x3 = 0;
  double y3 = 0;
  double xc = 0;
  double yc = 0;
  double r = 0;
  
  // Include each point one at a time into the existing mesh
  int nedge = 0;
  
  for (int i = 0; i < num_vertices; i++)
  {
    xp = vertex[i].x;
    yp = vertex[i].y;
    nedge = 0;

    // Set up the edge buffer.
    // If the point (xp,yp) lies inside the circumcircle then the
    // three edges of that triangle are added to the edge buffer
    // and that triangle is removed.
    //
    for (int j = 0; j < num_triangles; j++)
    {
      if (complete[j])
      {
        continue;
      }
      
      x1 = vertex[tri[j].p1].x;
      y1 = vertex[tri[j].p1].y;
      x2 = vertex[tri[j].p2].x;
      y2 = vertex[tri[j].p2].y;
      x3 = vertex[tri[j].p3].x;
      y3 = vertex[tri[j].p3].y;
      
      const bool inside = CircumCircle(xp, yp,
                                       x1, y1,
                                       x2, y2,
                                       x3, y3,
                                       &xc, &yc,
                                       &r);
      
      if (xc < xp && ((xp - xc) * (xp - xc)) > r)
      {
        complete[j] = true;
      }
      
      if (inside)
      {
        // Check that we haven't exceeded the edge list size
        if (nedge + 3 >= emax)
        {
          emax += 100;
          edges.resize(emax);
        }
        
        edges[nedge + 0].p1 = tri[j].p1;
        edges[nedge + 0].p2 = tri[j].p2;
        edges[nedge + 1].p1 = tri[j].p2;
        edges[nedge + 1].p2 = tri[j].p3;
        edges[nedge + 2].p1 = tri[j].p3;
        edges[nedge + 2].p2 = tri[j].p1;
        
        nedge += 3;
        tri[j] = tri[num_triangles - 1];
        complete[j] = complete[num_triangles - 1];
        
        num_triangles--;
        j--;
      }
    }

    // Tag multiple edges
    // 
    // NOTE: if all triangles are specified anticlockwise then all
    // interior edges are opposite pointing in direction.
    // 
    for (int j = 0; j < nedge - 1; j++)
    {
      for (int k = j + 1; k < nedge; k++)
      {
        if ((edges[j].p1 == edges[k].p2) && (edges[j].p2 == edges[k].p1))
        {
          edges[j].p1 = -1;
          edges[j].p2 = -1;
          edges[k].p1 = -1;
          edges[k].p2 = -1;
        }
        
        // Shouldn't need the following, see note above
        if ((edges[j].p1 == edges[k].p1) && (edges[j].p2 == edges[k].p2))
        {
          edges[j].p1 = -1;
          edges[j].p2 = -1;
          edges[k].p1 = -1;
          edges[k].p2 = -1;
        }
      }
    }

    // Form new triangles for the current point
    // Skipping over any tagged edges.
    // All edges are arranged in clockwise order.
    // 
    for (int j = 0; j < nedge; j++)
    {
      if (edges[j].p1 < 0 || edges[j].p2 < 0)
      {
        continue;
      }
      
      if (num_triangles >= max_triangles)
      {
        return 4;
      }
      
      tri[num_triangles].p1 = edges[j].p1;
      tri[num_triangles].p2 = edges[j].p2;
      tri[num_triangles].p3 = i;
      complete[num_triangles] = false;
      num_triangles++;
    }
  }
  
  // Remove triangles with supertriangle vertices
  // (triangles which have a vertex number greater than num_vertices)
  // 
  for (int i = 0; i < num_triangles; i++)
  {
    if (tri[i].p1 >= num_vertices ||
        tri[i].p2 >= num_vertices ||
        tri[i].p3 >= num_vertices)
    {
      tri[i] = tri[num_triangles - 1];
      num_triangles--;
      i--;
    }
  }
  
  return 0;
}


//----------------------------------------------------------------
// the_mesh_transform_t::setup_mesh
//
bool
the_mesh_transform_t::setup_mesh()
{
  std::size_t num_vertices = grid_.mesh_.size();
  if (num_vertices == 0)
  {
    return false;
  }
  
  // shortcut:
  const vertex_t * verts = &(grid_.mesh_[0]);
  
  // added 3 extra points for the super-triangle (bounding triangle):
  std::vector<TVertex> vertex_vec(num_vertices + 3);
  for (std::size_t i = 0; i < num_vertices; i++)
  {
    TVertex & vertex = vertex_vec[i];
    
    // triangulation happens in the uv-space:
    vertex.x = verts[i].uv_[0];
    vertex.y = verts[i].uv_[1];
    
    // keep track of the original vertex index:
    vertex.idx = i;
  }
  
  // sort the vertices on the x-coordinate, in ascending order:
  std::sort(vertex_vec.begin(),
            vertex_vec.begin() + num_vertices,
            &TVertexCompare);
  
  // In 2-D the number of triangles is calculated as
  // Nt = 2 * Nv - 2 - Nb, where Nv is the number of vertices
  // and Nb is the number of boundary vertices (vertices on the convex hull).
  // 
  // Since we don't know how many of the vertices are boundary vertices
  // we allocate a triangle buffer larger than necessary
  // also accounting for the supertriangle:
  // 
  const int max_triangles = 2 * (num_vertices + 3) - 2;
  std::vector<TTriangle> triangle_vec(max_triangles);
  
  // shortcuts:
  TVertex * vertices = &(vertex_vec[0]);
  TTriangle * triangles = &(triangle_vec[0]);

  int num_triangles = 0;
  int error = ::Triangulate(num_vertices,
                            vertices,
                            triangles,
                            max_triangles,
                            num_triangles);
  if (error != 0)
  {
    // this shouldn't happen:
    return false;
  }

  if ( num_triangles == 0 )
  {
    return true;
  }
  
  // setup the mesh triangles:
  grid_.tri_.resize(num_triangles);
  triangle_t * tris = &(grid_.tri_[0]);
  
  for (int i = 0; i < num_triangles; i++)
  {
    triangle_t & tri = tris[i];
    
    // flip triangle winding to counterclockwise:
    tri.vertex_[0] = (vertices[triangles[i].p1]).idx;
    tri.vertex_[2] = (vertices[triangles[i].p2]).idx;
    tri.vertex_[1] = (vertices[triangles[i].p3]).idx;
  }
  
  // initialize the acceleration grid:
  grid_.rebuild();
  
  return true;
}

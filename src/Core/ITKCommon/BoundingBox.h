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

#ifndef CORE_ITKCOMMON_BOUNDINGBOX_H
#define CORE_ITKCOMMON_BOUNDINGBOX_H

#include <itkPoint.h>
#include <itkVector.h>
#include <itkImageFileReader.h>

#include <Core/ITKCommon/itkTypes.h>

// TODO: temporary
#include <Core/IRTools_tmp/the_text.hxx>
#include <Core/IRTools_tmp/the_terminator.hxx>

//----------------------------------------------------------------
// is_empty_bbox
// 
// Test whether a bounding box is empty (min > max)
// 
extern bool
is_empty_bbox(const pnt2d_t & min,
              const pnt2d_t & max);

//----------------------------------------------------------------
// is_singular_bbox
// 
// Test whether a bounding box is singular (min == max)
// 
extern bool
is_singular_bbox(const pnt2d_t & min,
                 const pnt2d_t & max);

//----------------------------------------------------------------
// clamp_bbox
// 
// Restrict a bounding box to be within given limits.
// 
extern void
clamp_bbox(const pnt2d_t & confines_min,
           const pnt2d_t & confines_max,
           pnt2d_t & min,
           pnt2d_t & max);

//----------------------------------------------------------------
// update_bbox
// 
// Expand the bounding box to include a given point.
// 
inline static void
update_bbox(pnt2d_t & min, pnt2d_t & max, const pnt2d_t & pt)
{
  if (min[0] > pt[0]) min[0] = pt[0];
  if (min[1] > pt[1]) min[1] = pt[1];
  if (max[0] < pt[0]) max[0] = pt[0];
  if (max[1] < pt[1]) max[1] = pt[1];
}


//----------------------------------------------------------------
// bbox_overlap
// 
// Test whether two bounding boxes overlap.
// 
inline bool
bbox_overlap(const pnt2d_t & min_box1, 
             const pnt2d_t & max_box1, 
             const pnt2d_t & min_box2,
             const pnt2d_t & max_box2)
{
  return
  max_box1[0] > min_box2[0] && 
  min_box1[0] < max_box2[0] &&
  max_box1[1] > min_box2[1] && 
  min_box1[1] < max_box2[1];
}

//----------------------------------------------------------------
// inside_bbox
// 
// Test whether a given point is inside the bounding box.
// 
inline bool
inside_bbox(const pnt2d_t & min, const pnt2d_t & max, const pnt2d_t & pt)
{
  return
  min[0] <= pt[0] &&
  pt[0] <= max[0] &&
  min[1] <= pt[1] &&
  pt[1] <= max[1];
}

//----------------------------------------------------------------
// calc_image_bbox
// 
// Calculate the bounding box for a given image,
// expressed in image space.
// 
template <typename T>
void
calc_image_bbox(const T * image, pnt2d_t & bbox_min, pnt2d_t & bbox_max)
{
  typename T::SizeType sz = image->GetLargestPossibleRegion().GetSize();
  typename T::SpacingType sp = image->GetSpacing();
  bbox_min = image->GetOrigin();
  bbox_max[0] = bbox_min[0] + sp[0] * double(sz[0]);
  bbox_max[1] = bbox_min[1] + sp[1] * double(sz[1]);
}

//----------------------------------------------------------------
// calc_image_bboxes
// 
// Calculate the bounding boxes for a set of given images,
// expressed in image space.
// 
template <class image_pointer_t>
void
calc_image_bboxes(const std::vector<image_pointer_t> & image,
                  
                  // image space bounding boxes (for feathering):
                  std::vector<pnt2d_t> & image_min,
                  std::vector<pnt2d_t> & image_max)
{
  typedef typename image_pointer_t::ObjectType::Pointer::ObjectType T;
  typedef typename T::RegionType::SizeType imagesz_t;
  typedef typename T::SpacingType spacing_t;
  
  const unsigned int num_images = image.size();
  image_min.resize(num_images);
  image_max.resize(num_images);
  
  // calculate the bounding boxes:
  for (unsigned int i = 0; i < num_images; i++)
  {
    // initialize an empty bounding box:
    image_min[i][0] = std::numeric_limits<double>::max();
    image_min[i][1] = image_min[i][0];
    image_max[i][0] = -image_min[i][0];
    image_max[i][1] = -image_min[i][0];
    
    // it happens:
    if (image[i].GetPointer() == NULL) continue;
    
    // bounding box in the image space:
    calc_image_bbox<T>(image[i], image_min[i], image_max[i]);
  }
}

//----------------------------------------------------------------
// calc_image_bboxes_load_images
// 
// Calculate the bounding boxes for a set of images,
// expressed in image space.  These must be loaded, and are loaded
// one by one to save memory on large images.
// 
template <class image_pointer_t>
void
calc_image_bboxes_load_images(const std::list<the_text_t> & in,
                              
                              // image space bounding boxes (for feathering):
                              std::vector<pnt2d_t> & image_min,
                              std::vector<pnt2d_t> & image_max,
                              
                              const unsigned int shrink_factor,
                              const double pixel_spacing)
{
  typedef typename image_pointer_t::ObjectType::Pointer::ObjectType T;
  typedef typename T::RegionType::SizeType imagesz_t;
  typedef typename T::SpacingType spacing_t;
  
  const unsigned int num_images = in.size();
  image_min.resize(num_images);
  image_max.resize(num_images);
  
  // calculate the bounding boxes:
  unsigned int i = 0;
  for (std::list<the_text_t>::const_iterator iter = in.begin();
       iter != in.end(); ++iter, i++)
  {
    // initialize an empty bounding box:
    image_min[i][0] = std::numeric_limits<double>::max();
    image_min[i][1] = image_min[i][0];
    image_max[i][0] = -image_min[i][0];
    image_max[i][1] = -image_min[i][0];
    
    // Read in just the OutputInformation for speed.
    typedef typename itk::ImageFileReader<T> reader_t;
    typename reader_t::Pointer reader = reader_t::New();
    
    reader->SetFileName((*iter));
    std::cout << "loading region from " << (*iter) << std::endl;
    
    WRAP(terminator_t<reader_t> terminator(reader));
    
    // Load up the OutputInformation, faster than the whole image
    reader->UpdateOutputInformation();
    typename T::Pointer image = reader->GetOutput();
    
    // bounding box in the image space:
    calc_image_bbox<T>(image, image_min[i], image_max[i]);
    
    // Unload the image.
    image = image_t::Pointer(NULL);
  }
}

//----------------------------------------------------------------
// calc_tile_mosaic_bbox
// 
// Calculate the warped image bounding box in the mosaic space.
// 
extern bool
calc_tile_mosaic_bbox(const base_transform_t * mosaic_to_tile,
                      
                      // image space bounding boxes of the tile:
                      const pnt2d_t & tile_min,
                      const pnt2d_t & tile_max,
                      
                      // mosaic space bounding boxes of the tile:
                      pnt2d_t & mosaic_min,
                      pnt2d_t & mosaic_max,
                      
                      // sample points along the image edges:
                      const unsigned int np = 15);

//----------------------------------------------------------------
// calc_tile_mosaic_bbox
// 
// Calculate the warped image bounding box in the mosaic space.
// 
template <typename T>
bool
calc_tile_mosaic_bbox(const base_transform_t * mosaic_to_tile,
                      const T * tile,
                      
                      // mosaic space bounding boxes of the tile:
                      pnt2d_t & mosaic_min,
                      pnt2d_t & mosaic_max,
                      
                      // sample points along the image edges:
                      const unsigned int np = 15)
{
  typename T::SizeType sz = tile->GetLargestPossibleRegion().GetSize();
  typename T::SpacingType sp = tile->GetSpacing();
  
  pnt2d_t tile_min = tile->GetOrigin();
  pnt2d_t tile_max;
  tile_max[0] = tile_min[0] + sp[0] * double(sz[0]);
  tile_max[1] = tile_min[1] + sp[1] * double(sz[1]);
  
  return calc_tile_mosaic_bbox(mosaic_to_tile,
                               tile_min,
                               tile_max,
                               mosaic_min,
                               mosaic_max,
                               np);
}

//----------------------------------------------------------------
// calc_mosaic_bboxes
// 
// Calculate the warped image bounding boxes in the mosaic space.
// 
template <class point_t, class transform_pointer_t>
bool
calc_mosaic_bboxes(const std::vector<transform_pointer_t> & xform,
                   
                   // image space bounding boxes of mosaic tiles::
                   const std::vector<point_t> & image_min,
                   const std::vector<point_t> & image_max,
                   
                   // mosaic space bounding boxes of individual tiles:
                   std::vector<point_t> & mosaic_min,
                   std::vector<point_t> & mosaic_max,
                   
                   // sample points along the image edges:
                   const unsigned int np = 15)
{
  const unsigned int num_images = xform.size();
  mosaic_min.resize(num_images);
  mosaic_max.resize(num_images);
  
  point_t image_point;
  point_t mosaic_point;
  
  // calculate the bounding boxes
  bool ok = true;
  for (unsigned int i = 0; i < num_images; i++)
  {
    ok = ok & calc_tile_mosaic_bbox(xform[i],
                                    image_min[i],
                                    image_max[i],
                                    mosaic_min[i],
                                    mosaic_max[i],
                                    np);
  }
  
  return ok;
}


//----------------------------------------------------------------
// calc_mosaic_bbox
// 
// Calculate the mosaic bounding box (a union of mosaic space
// bounding boxes of the warped tiles).
// 
template <class point_t>
void
calc_mosaic_bbox(// mosaic space bounding boxes of individual mosaic tiles:
                 const std::vector<point_t> & mosaic_min,
                 const std::vector<point_t> & mosaic_max,
                 
                 // mosiac bounding box:
                 point_t & min,
                 point_t & max)
{
  // initialize an empty bounding box:
  min[0] = std::numeric_limits<double>::max();
  min[1] = min[0];
  max[0] = -min[0];
  max[1] = -min[0];
  
  // calculate the bounding box:
  const unsigned int num_images = mosaic_min.size();
  for (unsigned int i = 0; i < num_images; i++)
  {
    // it happens:
    if (mosaic_min[i][0] == std::numeric_limits<double>::max()) continue;
    
    // update the mosaic bounding box:
    update_bbox(min, max, mosaic_min[i]);
    update_bbox(min, max, mosaic_max[i]);
  }
}

//----------------------------------------------------------------
// calc_mosaic_bbox
// 
// Calculate the mosaic bounding box (a union of mosaic space
// bounding boxes of the warped tiles).
// 
template <class image_pointer_t, class transform_pointer_t>
void
calc_mosaic_bbox(const std::vector<transform_pointer_t> & transform,
                 const std::vector<image_pointer_t> & image,
                 
                 // mosaic bounding box:
                 typename image_pointer_t::ObjectType::PointType & min,
                 typename image_pointer_t::ObjectType::PointType & max,
                 
                 // points along the image edges:
                 const unsigned int np = 15)
{
  typedef typename image_pointer_t::ObjectType::Pointer::ObjectType image_t;
  typedef typename image_t::PointType point_t;
  
  // image space bounding boxes:
  std::vector<point_t> image_min;
  std::vector<point_t> image_max;
  calc_image_bboxes<image_pointer_t>(image, image_min, image_max);
  
  // mosaic space bounding boxes:
  std::vector<point_t> mosaic_min;
  std::vector<point_t> mosaic_max;
  
  // FIXME:
  calc_mosaic_bboxes<point_t, transform_pointer_t>(transform,
                                                   image_min,
                                                   image_max,
                                                   mosaic_min,
                                                   mosaic_max,
                                                   np);
  
  // mosiac bounding box:
  calc_mosaic_bbox<point_t>(mosaic_min, mosaic_max, min, max);
}

//----------------------------------------------------------------
// calc_mosaic_bbox_load_images
// 
// Calculate the mosaic bounding box (a union of mosaic space
// bounding boxes of the warped tiles).
// 
template <class image_pointer_t, class transform_pointer_t>
void
calc_mosaic_bbox_load_images(const std::vector<transform_pointer_t> & transform,
                             const std::list<the_text_t> & fn_image,
                             
                             // mosaic bounding box:
                             typename image_pointer_t::ObjectType::PointType & min,
                             typename image_pointer_t::ObjectType::PointType & max,
                             std::vector<typename image_pointer_t::ObjectType::PointType> & mosaic_min,
                             std::vector<typename image_pointer_t::ObjectType::PointType> & mosaic_max,
                             const unsigned int shrink_factor,
                             const double pixel_spacing,
                             
                             // points along the image edges:
                             const unsigned int np = 15)
{
  typedef typename image_pointer_t::ObjectType::Pointer::ObjectType image_t;
  typedef typename image_t::PointType point_t;
  
  // image space bounding boxes:
  std::vector<point_t> image_min;
  std::vector<point_t> image_max;
  
  calc_image_bboxes_load_images<image_pointer_t>(fn_image, 
                                                 image_min, 
                                                 image_max,
                                                 shrink_factor,
                                                 pixel_spacing);
  
  // mosaic space bounding boxes:
  // FIXME:
  calc_mosaic_bboxes<point_t, transform_pointer_t>(transform,
                                                   image_min,
                                                   image_max,
                                                   mosaic_min,
                                                   mosaic_max,
                                                   np);
  
  // mosiac bounding box:
  calc_mosaic_bbox<point_t>(mosaic_min, mosaic_max, min, max);
}


#endif
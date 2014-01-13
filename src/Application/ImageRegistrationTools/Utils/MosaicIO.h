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

#ifndef APPLICATION_IMAGEREGISTRATIONTOOLS_UTILS_MOSAICIO_H
#define APPLICATION_IMAGEREGISTRATIONTOOLS_UTILS_MOSAICIO_H

#include <iostream>
#include <list>
#include <string>
#include <vector>

#include <itkTransformBase.h>
#include <itkCastImageFilter.h>
#include <itkRGBPixel.h>
#include <itkComposeRGBImageFilter.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>

#include <Core/ITKCommon/itkTypes.h>

// TODO: temporary
#include <Application/ImageRegistrationTools/Utils/tmp/the_text.hxx>

//----------------------------------------------------------------
// load_mosaic
//
// Load image filenames and associated ITK transforms from a stream.
// 
extern void
load_mosaic(std::istream & si,
            double & pixel_spacing,
            bool & use_std_mask,
            std::vector<the_text_t> & image,
            std::vector<itk::TransformBase::Pointer> & transform);

//----------------------------------------------------------------
// load_mosaic
// 
// Load tile image names and associated ITK transforms from a stream.
// 
template <class transform_t>
void
load_mosaic(std::istream & si,
            double & pixel_spacing,
            bool & use_std_mask,
            std::list<the_text_t> & images,
            std::vector<typename transform_t::Pointer> & transforms,
            const the_text_t & image_path)
{
  std::vector<the_text_t> image;
  std::vector<itk::TransformBase::Pointer> tbase;
  
  load_mosaic(si, pixel_spacing, use_std_mask, image, tbase);
  transforms.resize(tbase.size());
  for (unsigned int i = 0; i < tbase.size(); i++)
  {
    images.push_back(image[i]);
    transforms[i] = dynamic_cast<transform_t *>(tbase[i].GetPointer());
  }
  
  if ( image_path.is_empty() )
    return;
  
  // TODO: replace with boost path utilities
  //  // Replace global paths for backwards compatibility.
  //  for (std::list<the_text_t>::iterator iter = images.begin(); iter != images.end(); ++iter)
  //  {
  //    IRPath::CleanSlashes( (*iter) );
  //    (*iter) = IRPath::CleanPath( image_path ) + 
  //    IRPath::FilenameFromPath( *iter );
  //  }
}

//----------------------------------------------------------------
// save_mosaic
// 
// Save image filenames and associated ITK transforms to a stream.
// 
extern void
save_mosaic(std::ostream & so,
            const unsigned int & num_images,
            const double & pixel_spacing,
            const bool & use_std_mask,
            const the_text_t * image,
            const itk::TransformBase ** transform);


//----------------------------------------------------------------
// load_transform
// 
// Load an ITK transform of specified type from a stream.
// 
extern itk::TransformBase::Pointer
load_transform(std::istream & si, const std::string & transform_type);

//----------------------------------------------------------------
// load_transform
//
// Load transform type from the stream, then load the transform
// of that type.
// 
extern itk::TransformBase::Pointer
load_transform(std::istream & si);


//----------------------------------------------------------------
// save_transform
// 
// Save an ITK transform to a stream.
// 
extern void
save_transform(std::ostream & so, const itk::TransformBase * t);


//----------------------------------------------------------------
// save_composite
//
// Save an RGB image of images fi and mi registered via
// a given transform. The mi image will be saved in the red channel,
// and fi will be saved in the green and blue channels.
// 
template <typename T>
void
save_composite(const char * filename,
               const T * fi,
               const T * mi,
               const base_transform_t * xform,
               bool blab = true)
{
  typename T::Pointer fi_aligned;
  typename T::Pointer mi_aligned;
  align_fi_mi(fi, mi, xform, fi_aligned, mi_aligned);
  
  typedef itk::CastImageFilter<T, native_image_t> cast_to_native_t;
  typename cast_to_native_t::Pointer fi_native = cast_to_native_t::New();
  typename cast_to_native_t::Pointer mi_native = cast_to_native_t::New();
  fi_native->SetInput(fi_aligned);
  mi_native->SetInput(mi_aligned);
  
  typedef itk::RGBPixel<native_pixel_t> composite_pixel_t;
  typedef itk::Image<composite_pixel_t, 2> composite_image_t;
  typedef itk::ComposeRGBImageFilter<native_image_t, composite_image_t>
  compose_filter_t;
  
  compose_filter_t::Pointer composer = compose_filter_t::New();
  composer->SetInput1(mi_native->GetOutput());
  composer->SetInput2(fi_native->GetOutput());
  composer->SetInput3(fi_native->GetOutput());
  
  typedef itk::ImageFileWriter<composite_image_t> composite_writer_t;
  composite_writer_t::Pointer writer = composite_writer_t::New();
  writer->SetFileName(filename);
  writer->SetInput(composer->GetOutput());
  
  // TODO: log!!!
//  if (blab) cout << "saving " << filename << endl;
  writer->Update();
}

#endif
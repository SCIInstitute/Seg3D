/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2021 Scientific Computing and Imaging Institute,
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

#include <Application/Tools/Algorithm/GrowCutter.h>

//itk
#include <itkImageRegionIterator.h>
#include <itkImageImport.h>

namespace Seg3D
{

//---------------------------------------------------------------------------
GrowCutter::GrowCutter()
{
  this->reset_growcut();
  for ( unsigned int i = 0; i < 3; i++ )
  {
    this->bbox_lower_[i] = 0;
    this->bbox_upper_[i] = 0;
  }
}

//---------------------------------------------------------------------------
GrowCutter::~GrowCutter()
{}

//---------------------------------------------------------------------------
void GrowCutter::set_data_image( typename DataImageType::Pointer data_image )
{
  this->data_image_ = data_image;
}

//---------------------------------------------------------------------------
void GrowCutter::set_foreground_image( typename ImageType::Pointer foreground_image )
{
  this->foreground_image_ = foreground_image;
}

//---------------------------------------------------------------------------
void GrowCutter::set_background_image( typename ImageType::Pointer background_image )
{
  this->background_image_ = background_image;
}

//---------------------------------------------------------------------------
void GrowCutter::set_output_image( typename ImageType::Pointer output_image )
{
  this->output_image_ = output_image;
}

//---------------------------------------------------------------------------
void GrowCutter::execute()
{
  typedef itk::Image< unsigned char, 3 > ImageType;
  typedef itk::Image< short, 3 > DataImageType;
  typedef itk::ImageRegionIterator< ImageType > IteratorType;
  typedef ImageType::RegionType RegionType;

  ImageType::RegionType original_region = this->foreground_image_->GetLargestPossibleRegion();
  RegionType::SizeType img_size = original_region.GetSize();
  RegionType::IndexType img_corner = original_region.GetIndex();

  ImageType::IndexType idx;
  ImageType::IndexType lower = img_corner + img_size;
  ImageType::IndexType upper = img_corner;

  IteratorType fit( this->foreground_image_, this->foreground_image_->GetLargestPossibleRegion() );
  IteratorType bit( this->background_image_, this->background_image_->GetLargestPossibleRegion() );

  fit.GoToBegin();
  bit.GoToBegin();

  // set foreground pixels to 1, background to 2
  for (; !fit.IsAtEnd(); ++fit, ++bit )
  {
    ImageType::PixelType foreground_pixel = fit.Get();
    ImageType::PixelType background_pixel = bit.Get();

    if ( foreground_pixel == 1 || background_pixel == 1 )
    {
      idx = bit.GetIndex();
      // update the bounding box
      for ( unsigned int i = 0; i < 3; i++ )
      {
        if ( lower[i] > idx[i] )
        {
          lower[i] = idx[i];
        }
        if ( upper[i] < idx[i] )
        {
          upper[i] = idx[i];
        }
      }

      // combine fore and back
      if ( foreground_pixel == 1 ) // foreground takes precedence
      {
        bit.Set( 1 );
      }
      else
      {
        if ( background_pixel == 1 )
        {
          bit.Set( 2 );
        }
      }
    }
  }

  // check if bounding box has changed
  bool needs_reset = false;
  for ( unsigned int i = 0; i < 3; i++ )
  {
    if ( this->bbox_lower_[i] != lower[i] || this->bbox_upper_[i] != upper[i] )
    {
      needs_reset = true;
    }
  }

  if ( needs_reset )
  {
    // reset growcut because the bounding box has changed
    this->reset_growcut();
    this->bbox_lower_ = lower;
    this->bbox_upper_ = upper;
    std::cerr << "Bounding box changed, resetting growcut\n";
  }

  // Call GrowCut
  // TODO: Add segmenter initialization flag check. See CarreraSliceEffect.py
  if ( this->initialization_flag_ == false )
  {
    this->fast_grow_cut_->SetSourceVol( data_image_ );
    this->fast_grow_cut_->SetSeedVol( background_image_ );
    this->fast_grow_cut_->SetInitializationFlag( this->initialization_flag_ );
    this->fast_grow_cut_->Initialization();       // This method will set grow cut initialization flag to false
    this->fast_grow_cut_->RunFGC();
    this->initialization_flag_ = true;
  }
  else
  {
    this->fast_grow_cut_->SetSeedVol( background_image_ );
    this->fast_grow_cut_->SetInitializationFlag( this->initialization_flag_ );
    this->fast_grow_cut_->RunFGC();
  }
}

//---------------------------------------------------------------------------
typename GrowCutter::ImageType::Pointer GrowCutter::get_output()
{
  return this->output_image_;
}

//---------------------------------------------------------------------------
void GrowCutter::reset_growcut()
{
  this->fast_grow_cut_ = itkSmartPointer<itkFastGrowCut>::New();
  this->initialization_flag_ = false;
}
}

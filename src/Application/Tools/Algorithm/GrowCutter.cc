#include <Corview/CorBase/Algorithm/GrowCutter.h>

//itk
#include <itkImageRegionIterator.h>

//vtk
#include <vtkImageImport.h>
#include <vtkSmartPointer.h>
#include "vtkImageData.h"

namespace Corview
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
void GrowCutter::set_data_image( itk::Image<short, 3>::Pointer data_image )
{
  this->data_image_ = data_image;
}

//---------------------------------------------------------------------------
void GrowCutter::set_foreground_image( itk::Image<unsigned char, 3>::Pointer foreground_image )
{
  this->foreground_image_ = foreground_image;
}

//---------------------------------------------------------------------------
void GrowCutter::set_background_image( itk::Image<unsigned char, 3>::Pointer background_image )
{
  this->background_image_ = background_image;
}

//---------------------------------------------------------------------------
void GrowCutter::set_output_image( itk::Image<unsigned char, 3>::Pointer output_image )
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

  // Convert the ITK label image to VTK image data
  this->connector1_ = ConnectorType::New();
  this->connector1_->SetInput( this->background_image_ );
  this->connector1_->Update();
  vtkImageData* added_seeds_image = this->connector1_->GetOutput();

  // Convert the ITK source image to VTK image data
  this->connector2_ = DataConnectorType::New();
  this->connector2_->SetInput( this->data_image_ );
  this->connector2_->Update();
  vtkImageData* source_image_ = this->connector2_->GetOutput();

  // Call GrowCut
  // TODO: Add segmenter initialization flag check. See CarreraSliceEffect.py
  if ( this->initialization_flag_ == false )
  {
    this->fast_grow_cut_->SetSourceVol( source_image_ );
    this->fast_grow_cut_->SetSeedVol( added_seeds_image );
    this->fast_grow_cut_->SetInitializationFlag( this->initialization_flag_ );
    this->fast_grow_cut_->Initialization();       // This method will set grow cut initialization flag to false
    this->fast_grow_cut_->RunFGC();
    this->initialization_flag_ = true;
  }
  else
  {
    this->fast_grow_cut_->SetSeedVol( added_seeds_image );
    this->fast_grow_cut_->SetInitializationFlag( this->initialization_flag_ );
    this->fast_grow_cut_->RunFGC();
  }

  // Convert grow cut result to ITK image for display
  this->connector3_ = VTKConnectorType::New();
  this->connector3_->SetInput( added_seeds_image );
  this->connector3_->Update();
  this->output_image_ = this->connector3_->GetOutput();
}

//---------------------------------------------------------------------------
itk::Image<unsigned char, 3>::Pointer GrowCutter::get_output()
{
  return this->output_image_;
}

//---------------------------------------------------------------------------
void GrowCutter::reset_growcut()
{
  this->fast_grow_cut_ = vtkSmartPointer<vtkFastGrowCut>::New();
  this->initialization_flag_ = false;
}
}

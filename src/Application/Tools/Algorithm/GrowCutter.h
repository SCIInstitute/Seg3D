#ifndef CORVIEW_CORBASE_ALGORITHM_GROWCUTTER_H
#define CORVIEW_CORBASE_ALGORITHM_GROWCUTTER_H

#include <boost/shared_ptr.hpp>

#include <itkImage.h>

#include <Corview/CorBase/Algorithm/itkImageToVTKImageFilter.h>
#include <Corview/CorBase/Algorithm/itkVTKImageToImageFilter.h>

//GrowCut
#include <vtkSmartPointer.h>
#include "vtkFastGrowCut.h"

namespace Corview {

class GrowCutter;
typedef boost::shared_ptr<GrowCutter> GrowCutterHandle;

class GrowCutter
{
public:
  GrowCutter();
  ~GrowCutter();

  void set_data_image( itk::Image<short, 3>::Pointer data_image );
  void set_foreground_image( itk::Image<unsigned char, 3>::Pointer foreground_image );
  void set_background_image( itk::Image<unsigned char, 3>::Pointer background_image );
  void set_output_image( itk::Image<unsigned char, 3>::Pointer output_image_ );
  void execute();

  itk::Image<unsigned char, 3>::Pointer get_output();

private:

  void reset_growcut();

  itk::Image<short, 3>::Pointer data_image_;
  itk::Image<unsigned char, 3>::Pointer foreground_image_;
  itk::Image<unsigned char, 3>::Pointer background_image_;
  itk::Image<unsigned char, 3>::Pointer output_image_;
  vtkSmartPointer<vtkFastGrowCut> fast_grow_cut_;
  bool initialization_flag_;

  typedef itk::Image< unsigned char, 3 > ImageType;
  typedef itk::Image< short, 3 > DataImageType;
  typedef itk::VTKImageToImageFilter<ImageType> VTKConnectorType;
  typedef itk::ImageToVTKImageFilter<DataImageType> DataConnectorType;
  typedef itk::ImageToVTKImageFilter<ImageType> ConnectorType;

  ConnectorType::Pointer connector1_;
  DataConnectorType::Pointer connector2_;
  VTKConnectorType::Pointer connector3_;

  itk::Image< unsigned char, 3 >::IndexType bbox_lower_;
  itk::Image< unsigned char, 3 >::IndexType bbox_upper_;
};
}

#endif /* CORVIEW_CORBASE_ALGORITHM_GROWCUTTER_H */

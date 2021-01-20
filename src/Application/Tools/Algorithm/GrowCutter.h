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

#ifndef APPLICATION_ALGORITHM_GROWCUTTER_H
#define APPLICATION_ALGORITHM_GROWCUTTER_H

#include <boost/shared_ptr.hpp>

#include <itkImage.h>

//???
//#include <Application/Tools/Algorithm/itkImageToVTKImageFilter.h>
//#include <Application/Tools/Algorithm/itkVTKImageToImageFilter.h>

//GrowCut
#include <itkSmartPointer.h>
#include <Application/Tools/Algorithm/itkFastGrowCut.h>

namespace Seg3D {

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
  itk::SmartPointer<itkFastGrowCut> fast_grow_cut_;
  bool initialization_flag_;

  typedef itk::Image< unsigned char, 3 > ImageType;
  typedef itk::Image< short, 3 > DataImageType;

  /*typedef itk::VTKImageToImageFilter<ImageType> VTKConnectorType;
  typedef itk::ImageToVTKImageFilter<DataImageType> DataConnectorType;
  typedef itk::ImageToVTKImageFilter<ImageType> ConnectorType;*/

  //ConnectorType::Pointer connector1_;
  //DataConnectorType::Pointer connector2_;
  //VTKConnectorType::Pointer connector3_;

  itk::Image< unsigned char, 3 >::IndexType bbox_lower_;
  itk::Image< unsigned char, 3 >::IndexType bbox_upper_;
};
}

#endif /* APPLICATION_ALGORITHM_GROWCUTTER_H */

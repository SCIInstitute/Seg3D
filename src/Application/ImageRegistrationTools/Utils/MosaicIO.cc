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

#include <Application/ImageRegistrationTools/Utils/MosaicIO.h>

// ITK includes
#include <itkTransformFactoryBase.h>
#include <itkTransformFactory.h>

#include <itkFixedCenterOfRotationAffineTransform.h>
#include <itkIdentityTransform.h>
#include <itkTranslationTransform.h>

// Internal ITK includes
#include <Core/ITKCommon/itkGridTransform.h>
#include <Core/ITKCommon/itkLegendrePolynomialTransform.h>
#include <Core/ITKCommon/itkMeshTransform.h>
#include <Core/ITKCommon/itkRBFTransform.h>
#include <Core/ITKCommon/itkRadialDistortionTransform.h>

#include <iostream>
#include <ostream>
#include <ios>
#include <iomanip>


//----------------------------------------------------------------
// register_transforms
//
static void
register_transforms()
{
  // try to avoid re-registering the same transforms over and over again:
  static bool transforms_registered = false;
  if (transforms_registered) return;
  
  // make sure the transforms I care about are known to the object factory:
  itk::TransformFactoryBase::RegisterDefaultTransforms();
  
  itk::TransformFactory<itk::IdentityTransform<double, 2> >::
  RegisterTransform();
  
  itk::TransformFactory<itk::TranslationTransform<double, 2> >::
  RegisterTransform();
  
  itk::TransformFactory<itk::FixedCenterOfRotationAffineTransform<double,2> >::
  RegisterTransform();
  
  itk::TransformFactory<itk::LegendrePolynomialTransform<double, 1> >::
  RegisterTransform();
  
  itk::TransformFactory<itk::LegendrePolynomialTransform<double, 2> >::
  RegisterTransform();
  
  itk::TransformFactory<itk::LegendrePolynomialTransform<double, 3> >::
  RegisterTransform();
  
  itk::TransformFactory<itk::LegendrePolynomialTransform<double, 4> >::
  RegisterTransform();
  
  itk::TransformFactory<itk::LegendrePolynomialTransform<double, 5> >::
  RegisterTransform();
  
  itk::TransformFactory<itk::GridTransform>::
  RegisterTransform();
  
  itk::TransformFactory<itk::MeshTransform>::
  RegisterTransform();
  
  itk::TransformFactory< itk::RBFTransform>::
  RegisterTransform();
  
  itk::TransformFactory< itk::RadialDistortionTransform<double, 2> >::
  RegisterTransform();
  
  transforms_registered = true;
}


//----------------------------------------------------------------
// load_mosaic
// 
// Load image filenames and associated ITK transforms from a stream.
// 
void
load_mosaic(std::istream & si,
            double & pixel_spacing,
            bool & use_std_mask,
            std::vector<the_text_t> & image,
            std::vector<itk::TransformBase::Pointer> & transform)
{
  // make sure the transforms I care about are known to the object factory:
  register_transforms();
  
  unsigned int num_images = ~0;
  unsigned int i = 0;
  unsigned int version = 0;
  
  // for backwards compatibility assume the standard mask is used:
  use_std_mask = true;
  
  while (si.eof() == false && num_images != i)
  {
    std::string token;
    si >> token;
    
    if (token == "format_version_number:")
    {
      si >> version;
    }
    else if (token == "number_of_images:")
    {
      si >> num_images;
      image.resize(num_images);
      transform.resize(num_images);
    }
    else if (token == "pixel_spacing:")
    {
      si >> pixel_spacing;
    }
    else if (token == "use_std_mask:")
    {
      si >> use_std_mask;
    }
    else if (token == "image:")
    {
      if (version == 0)
      {
        si >> image[i];
      }
      else
      {
        si >> std::ws;
        getline(si, image[i]);
      }
      
      // the next token should be the transform type string:
      std::string next_token;
      si >> next_token;
      
      if (version == 0)
      {
        // Original .mosaic file format kept the filename
        // and the transform type string on the same line,
        // which made filenames with spaces difficult to handle:
        
        while (si.eof() == false)
        {
          itk::LightObject::Pointer tmp =
          itk::ObjectFactoryBase::CreateInstance(next_token.c_str());
          
          if (tmp.GetPointer())
          {
            break;
          }
          
          // NOTE: this is a dirty hack to work around filenames with
          // spaces problem in the original file format.
          
          // assume the string that was just read in is a part of filename:
          image[i] += the_text_t(" ");
          image[i] += the_text_t(next_token.c_str());
          si >> next_token;
        }
      }
      
      transform[i] = load_transform(si, next_token);
      i++;
    }
    else
    {
      std::cerr << "WARNING: unknown token: '" << token << "', ignoring ..." << std::endl;
    }
  }
}


//----------------------------------------------------------------
// save_mosaic
//
// Save image filenames and associated ITK transforms to a stream.
// 
void
save_mosaic(std::ostream & so,
            const unsigned int & num_images,
            const double & pixel_spacing,
            const bool & use_std_mask,
            const the_text_t * image,
            const itk::TransformBase ** transform)
{
  std::ios::fmtflags old_flags = so.setf(std::ios::scientific);
  int old_precision = so.precision();
  so.precision(12);
  
  so << "format_version_number: " << 1 << std::endl
  << "number_of_images: " << num_images << std::endl
  << "pixel_spacing: " << pixel_spacing << std::endl
  << "use_std_mask: " << use_std_mask << std::endl;
  
  for (unsigned int i = 0; i < num_images; i++)
  {
    // Find just the filename to save out to the mosaic.
    so << "image:" << std::endl
       << image[i] << std::endl;
    save_transform(so, transform[i]);
    so << std::endl;
  }
  
  so.setf(old_flags);
  so.precision(old_precision);
}


// TODO: replace assert with exception
//
//----------------------------------------------------------------
// load_transform
//
// Load an ITK transform of specified type from a stream.
// 
itk::TransformBase::Pointer
load_transform(std::istream & si, const std::string & transform_type)
{
  typedef itk::TransformBase::ParametersType params_t;
  
  // make sure the transforms I care about are known to the object factory:
  register_transforms();
  
  // FIXME:
#if 0
//  std::list<std::string> names =
//  itk::TransformFactoryBase::GetFactory()->GetClassOverrideWithNames();
//  for (std::list<std::string>::iterator it = names.begin();
//       it != names.end();
//       ++it)
//  {
//    std::cerr << "FIXME: " << *it << std::endl;
//  }
#endif
  
  itk::LightObject::Pointer tmp =
  itk::ObjectFactoryBase::CreateInstance(transform_type.c_str());
  
  itk::TransformBase::Pointer t =
  dynamic_cast<itk::TransformBase *>(tmp.GetPointer());
  
  if (t.GetPointer() == NULL)
  {
    std::cerr << "could not instantiate " << transform_type
    << ", giving up ..." << std::endl;
    return t;
  }
  t->Register();
  
  // load the variable parameters:
  params_t vp;
  std::string vp_token;
  si >> vp_token;
  if (vp_token != "vp")
  {
    std::cerr << "expected vp, received '" << vp_token
    << "', aborting ..." << std::endl;
    assert(false);
  }
  
  unsigned int num_vp = 0;
  si >> num_vp;
  vp.SetSize(num_vp);
  
  for (unsigned int i = 0; i < num_vp; i++)
  {
#ifdef __APPLE__
    // TODO: still true?
    //
    // NOTE: OSX std::iostream is broken -- it can write out doubles
    // so small that it can't read them back in.  The workaround is
    // to read them in as long doubles, and then cast them down to
    // a double:
    long double tmp = 0;
    si >> tmp;
    if (si.fail()) si.clear();
    vp[i] = double(tmp);
#else
    si >> vp[i];
#endif
  }
  
  // load the fixed parameters:
  params_t fp;
  std::string fp_token;
  si >> fp_token;
  if (fp_token == "fp")
  {
    fp = t->GetFixedParameters();
    unsigned int num_fp = 0;
    si >> num_fp;
    
    if (num_fp != fp.size())
    {
      // some transforms (RBF) have variable number of fixed parameters:
      fp.SetSize(num_fp);
    }
    
    for (unsigned int i = 0; i < num_fp; i++)
    {
#ifdef __APPLE__
      // TODO: still true?
      //
      // NOTE: OSX std::iostream is broken -- it can write out doubles
      // so small that it can't read them back in.  The workaround is
      // to read them in as long doubles, and then cast them down to
      // a double:
      long double tmp = 0;
      si >> tmp;
      if (si.fail()) si.clear();
      fp[i] = double(tmp);
#else
      si >> fp[i];
#endif
    }
  }
  else if (fp_token != "no_fp")
  {
    std::cerr << "unexpected token: '" << fp_token
    << "', aborting ..." << std::endl;
    assert(false);
  }
  
  // set the fixed parameters first:
  try
  {
    t->SetFixedParameters(fp);
  }
  catch (itk::ExceptionObject &)
  {}
  
  // set the variable parameters:
  t->SetParametersByValue(vp);
  
  return t;
}

//----------------------------------------------------------------
// load_transform
// 
itk::TransformBase::Pointer
load_transform(std::istream & si)
{
  // load the transform type:
  std::string transform_type;
  si >> transform_type;
  
  return load_transform(si, transform_type);
}


//----------------------------------------------------------------
// save_transform
//
void
save_transform(std::ostream & so, const itk::TransformBase * t)
{
  typedef itk::TransformBase::ParametersType params_t;
  
  // make sure the transforms I care about are known to the object factory:
  register_transforms();
  
  // save the transform type:
  so << t->GetTransformTypeAsString();
  
  // get the stream precision:
  int prec = so.precision();
  
  // save the variable parameters:
  params_t vp = t->GetParameters();
  const unsigned int num_vp = vp.size();
  so << " vp " << num_vp;
  for (unsigned int i = 0; i < num_vp; i++)
  {
    so << ' ' << std::setw(prec + 7) << vp[i];
  }
  
  // save the fixed parameters:
  try
  {
    params_t fp = t->GetFixedParameters();
    
    const unsigned int num_fp = fp.size();
    so << " fp " << num_fp;
    for (unsigned int i = 0; i < num_fp; i++)
    {
      so << ' ' << std::setw(prec + 7) << fp[i];
    }
  }
  catch (itk::ExceptionObject &)
  {
    so << " no_fp";
  }
}

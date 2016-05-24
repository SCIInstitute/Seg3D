/*
 For more information, please see: http://software.sci.utah.edu
 
 The MIT License
 
 Copyright (c) 2016 Scientific Computing and Imaging Institute,
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

// File         : IRImageLoader.cxx
// Author       : Joel Spaltenstein
// Created      : 2008/02/21
// Copyright    : (C) 2008 University of Utah
// Description  :

// ITK includes:
#include <itkImportImageFilter.h>

// the includes:
#include <Core/ITKCommon/Transform/itkLegendrePolynomialTransform.h>
#include <Core/ITKCommon/the_utils.hxx>
#include <Core/ITKCommon/IRImageLoader.hxx>

#include <Core/Utils/Log.h>

#include <boost/filesystem.hpp>

namespace bfs=boost::filesystem;


//----------------------------------------------------------------
// __sharedIRImageLoader
// 
static IRImageLoader * __sharedIRImageLoader = NULL;

//----------------------------------------------------------------
// __IRImageLoaderSharedImageLoaderLock
// 
static itk::SimpleMutexLock __IRImageLoaderSharedImageLoaderLock;

//----------------------------------------------------------------
// IRImageLoader::sharedImageLoader
// 
IRImageLoader *
IRImageLoader::sharedImageLoader()
{
  __IRImageLoaderSharedImageLoaderLock.Lock();
  if (__sharedIRImageLoader == NULL)
  {
    __sharedIRImageLoader = new IRImageLoader();
  }
  
  __IRImageLoaderSharedImageLoaderLock.Unlock();
  return __sharedIRImageLoader;
}

//----------------------------------------------------------------
// IRImageLoader::IRImageLoader
// 
IRImageLoader::IRImageLoader() :
  SHRINK_FACTOR(1),
  PIXEL_SPACING(1.0),
  _shrinkFactor(SHRINK_FACTOR),
  _pixelSpacing(PIXEL_SPACING)
{}

//----------------------------------------------------------------
// IRImageLoader::~IRImageLoader
// 
IRImageLoader::~IRImageLoader()
{}

//----------------------------------------------------------------
// __IRImageLoaderImageAcessLock
// 
static itk::SimpleMutexLock __IRImageLoaderImageAcessLock;
static itk::SimpleMutexLock __IRImageLoaderMaskAcessLock;

//----------------------------------------------------------------
// IRImageLoader::setTransformList
// 
void
IRImageLoader::setTransformList(const IRTransformVector & transforms,
                                const TheTextVector & imageIDs)
{
  __IRImageLoaderImageAcessLock.Lock();
  
  IRTransformVector::const_iterator transformIter = transforms.begin();
  TheTextVector::const_iterator imageIDIter = imageIDs.begin();
  for (; transformIter != transforms.end(); transformIter++, imageIDIter++)
  {
    _transformations[*imageIDIter] = *transformIter;
  }
  _images = imageMap();
  
  __IRImageLoaderImageAcessLock.Unlock();
}

//----------------------------------------------------------------
// IRImageLoader::setShrinkFactor
// 
void
IRImageLoader::setShrinkFactor(unsigned int shrinkFactor)
{
  _shrinkFactor = shrinkFactor;
}

//----------------------------------------------------------------
// IRImageLoader::shrinkFactor
// 
unsigned int
IRImageLoader::shrinkFactor()
{
  return _shrinkFactor;
}

//----------------------------------------------------------------
// IRImageLoader::setPixelSpacing
// 
void
IRImageLoader::setPixelSpacing(double pixelSpacing)
{
  _pixelSpacing = pixelSpacing;
}

//----------------------------------------------------------------
// IRImageLoader::pixelSpacing
// 
double
IRImageLoader::pixelSpacing()
{
  return _pixelSpacing;
}

//----------------------------------------------------------------
// IRImageLoader::getImageSize
// 
vec2d_t
IRImageLoader::getImageSize(const std::string& imageID,
                            base_transform_t::Pointer transform)
{
  vec2d_t imageSize;
  itk::LegendrePolynomialTransform<double, 1>::Pointer legendrePointer;
  legendrePointer =
  dynamic_cast<itk::LegendrePolynomialTransform<double, 1> *>
  (transform.GetPointer());
  const double SCALE = 2.0;
  
  if (legendrePointer.IsNotNull())
  {
    imageSize[0] = legendrePointer->GetFixedParameters()[2] * SCALE;
    imageSize[1] = legendrePointer->GetFixedParameters()[3] * SCALE;
  }
  else
  {
    // since this version does not rescale the image it is much faster
    image_t::Pointer image = getFullResImage(imageID);
    
    // first find the size of the tile
    pnt2d_t bbox_min;
    pnt2d_t bbox_max;
    calc_image_bbox<image_t>(image, bbox_min, bbox_max);
    imageSize = bbox_max - bbox_min;
    imageSize *= _pixelSpacing;
  }
  
  return imageSize;
}

//----------------------------------------------------------------
// IRImageLoader::getFullResImage
// 
// this does not cache the image
// 
image_t::Pointer
IRImageLoader::getFullResImage(const std::string& imageID)
{
  bfs::path imagePath(imageID);
  if (! bfs::exists(imagePath) )
  {
    std::ostringstream oss;
    oss << "Tile " << imageID << " cannot be found.";
    CORE_LOG_WARNING(oss.str());
  }
  return std_tile<image_t>(imagePath,
                           SHRINK_FACTOR,
                           PIXEL_SPACING);
}

//----------------------------------------------------------------
// IRImageLoader::getImage
// 
image_t::Pointer
IRImageLoader::getImage(const std::string& imageID)
{
  imageCheckLoad(imageID);
  
  __IRImageLoaderImageAcessLock.Lock();
  image_t::Pointer image = _images[imageID];
  __IRImageLoaderImageAcessLock.Unlock();
  
  return image;
}

//----------------------------------------------------------------
// IRImageLoader::getMask
// 
mask_t::Pointer
IRImageLoader::getMask(const std::string& maskID)
{
  maskCheckLoad(maskID);
  
  __IRImageLoaderImageAcessLock.Lock();
  mask_t::Pointer mask = _masks[maskID];
  __IRImageLoaderImageAcessLock.Unlock();
  
  return mask;
}

//----------------------------------------------------------------
// IRImageLoader::imageCheckLoad
// 
void
IRImageLoader::imageCheckLoad(const std::string& imageID)
{
  __IRImageLoaderImageAcessLock.Lock();
  
  if (_images.find(imageID) != _images.end())
  {
    while (_images[imageID].IsNull())
    {
      // if another thread is already loading this same image,
      // spin until it is loaded
      __IRImageLoaderImageAcessLock.Unlock();
      sleep_msec(1000);
      __IRImageLoaderImageAcessLock.Lock();
    }
    
    __IRImageLoaderImageAcessLock.Unlock();
    return;
  }
  
  _images[imageID] = NULL;
  __IRImageLoaderImageAcessLock.Unlock();
  
  image_t::Pointer image = std_tile<image_t>(imageID.c_str(),
                                             _shrinkFactor,
                                             _pixelSpacing);
  
  __IRImageLoaderImageAcessLock.Lock();
  _images[imageID] = image;
  __IRImageLoaderImageAcessLock.Unlock();
}

//----------------------------------------------------------------
// IRImageLoader::maskCheckLoad
// 
void
IRImageLoader::maskCheckLoad(const std::string& maskID)
{
  __IRImageLoaderMaskAcessLock.Lock();
  
  if (_masks.find(maskID) != _masks.end())
  {
    if ( !maskID.empty() )
    {
      while (_masks[maskID].IsNull())
      {
        // if another thread is already loading this same image,
        // spin until it is loaded
        __IRImageLoaderMaskAcessLock.Unlock();
        sleep_msec(1000);
        __IRImageLoaderMaskAcessLock.Lock();
      }
    }
    
    __IRImageLoaderMaskAcessLock.Unlock();
    return;
  }
  
  _masks[maskID] = NULL;
  __IRImageLoaderMaskAcessLock.Unlock();
  
  mask_t::Pointer mask;
  if ( !maskID.empty() )
    mask = std_tile<mask_t>(maskID.c_str(), _shrinkFactor, _pixelSpacing);
  
  __IRImageLoaderMaskAcessLock.Lock();
  _masks[maskID] = mask;
  __IRImageLoaderMaskAcessLock.Unlock();
}

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

// File         : IRImageLoader.hxx
// Author       : Joel Spaltenstein
// Created      : 2008/02/20
// Copyright    : (C) 2008 University of Utah
// Description  :

#ifndef __IR_IMAGE_LOADER_HXX__
#define __IR_IMAGE_LOADER_HXX__

#include <vector>
#include <map>

#include <Core/ITKCommon/common.hxx>
#include <Core/ITKCommon/the_text.hxx>
#include <Core/ITKCommon/Optimizers/itkImageMosaicVarianceMetric.h>

#include <itkTransformBase.h>

class IRTransform;

class IRImageLoader
{
  
  struct lt_the_text
  {
    bool operator()(const std::string& s1, const std::string& s2) const
    {
      the_text_t t1(s1.c_str());
      the_text_t t2(s2.c_str());
      return (t1 < t2);
    }
  };
  
  //typedef std::map<the_text_t, IRTransform*, struct lt_the_text> transformationMap;
  //typedef std::map<the_text_t, image_t::Pointer, struct lt_the_text> imageMap;
  //typedef std::map<the_text_t, mask_t::Pointer, struct lt_the_text> maskMap;
  //typedef std::vector<IRTransform*> IRTransformVector;
  //typedef std::vector<the_text_t> TheTextVector;
  typedef std::map<std::string, IRTransform*, struct lt_the_text> transformationMap;
  typedef std::map<std::string, image_t::Pointer, struct lt_the_text> imageMap;
  typedef std::map<std::string, mask_t::Pointer, struct lt_the_text> maskMap;
  typedef std::vector<IRTransform*> IRTransformVector;
  typedef std::vector<std::string> TheTextVector;
  
public:
  static IRImageLoader* sharedImageLoader();
  
  virtual ~IRImageLoader();
  
  void setTransformList(const IRTransformVector& transforms, const TheTextVector& imageIDs);
  
  void setShrinkFactor(unsigned int shrinkFactor);
  unsigned int shrinkFactor();
  void setPixelSpacing(double pixelSpacing);
  double pixelSpacing();
  
  image_t::Pointer getImage(const std::string& imageID);
  mask_t::Pointer  getMask(const std::string& maskID);
  image_t::Pointer getFullResImage(const std::string& imageID); // this does not cache the image
  vec2d_t getImageSize(const std::string& imageID, base_transform_t::Pointer transform);
  
private:
  IRImageLoader();
  
  void imageCheckLoad(const std::string& imageID); 
  void maskCheckLoad(const std::string& maskID); 

  const unsigned int SHRINK_FACTOR;
  const double PIXEL_SPACING;
  
  unsigned int _shrinkFactor;
  double _pixelSpacing;
  imageMap _images;
  maskMap _masks;
  transformationMap _transformations;
};



#endif

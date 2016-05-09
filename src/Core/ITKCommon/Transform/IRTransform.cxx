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

// File         : IRTransform.cxx
// Author       : Joel Spaltenstein
// Created      : 2008/02/20
// Copyright    : (C) 2008 University of Utah
// Description  : 

// system includes:
#include <stdio.h>

// the includes:
#include <Core/ITKCommon/Transform/IRTransform.hxx>
#include <Core/ITKCommon/Transform/IRConnection.hxx>
#include <Core/ITKCommon/Transform/IRRefineTranslateCanvas.hxx>
#include <Core/ITKCommon/IRImageLoader.hxx>


//----------------------------------------------------------------
// IRTransform::IRTransform
// 
IRTransform::IRTransform(base_transform_t::Pointer transformBase,
                         const bfs::path & imageID,
                         const bfs::path & maskID)
{
  _transformBase = transformBase;
  _imageID = imageID;
  _maskID = maskID;
  _groupID = -1;
  
  IRImageLoader* imageLoader = IRImageLoader::sharedImageLoader();
  vec2d_t tile_size = imageLoader->getImageSize(_imageID.string(), _transformBase);
  pnt2d_t bbox_min;
  bbox_min[0] = bbox_min[1] = 0;
  pnt2d_t bbox_max;
  bbox_max[0] = tile_size[0];
  bbox_max[1] = tile_size[1];
  
  pnt2d_t mosaic_min;
  pnt2d_t mosaic_max;
  calc_tile_mosaic_bbox(transformBase,
                        // image space bounding box of the tile:
                        bbox_min, bbox_max,
                        // mosaic space bounding box of the tile:
                        mosaic_min, mosaic_max, 12);
  
  _position = mosaic_min;
  _size = mosaic_max - mosaic_min;
}

//----------------------------------------------------------------
// IRTransform::~IRTransform
// 
IRTransform::~IRTransform()
{}

//----------------------------------------------------------------
// IRTransform::position
// 
const pnt2d_t &
IRTransform::position() const
{
  return _position;
}

//----------------------------------------------------------------
// IRTransform::size
// 
const vec2d_t &
IRTransform::size() const
{
  return _size;
}

//----------------------------------------------------------------
// IRTransform::groupID
// 
long
IRTransform::groupID() const
{
  return _groupID;
}

//----------------------------------------------------------------
// IRTransform::setGroupID
// 
void
IRTransform::setGroupID(long newID)
{
  _groupID = newID;
}

//----------------------------------------------------------------
// IRTransform::connections
// 
const IRConnectionVector &
IRTransform::connections() const
{
  return _connections;
}

//----------------------------------------------------------------
// IRTransform::overlapsTransform
// 
bool
IRTransform::overlapsTransform(const IRTransform* otherTransform) const
{
  double minX = _position[0];
  double maxX = _position[0] + _size[0];
  double minY = _position[1];
  double maxY = _position[1] + _size[1];
  double width = _size[0];
  double height = _size[1];
  
  double otherMinX = otherTransform->position()[0];
  double otherMaxX = (otherTransform->position()[0] +
                      otherTransform->size()[0]);
  double otherMinY = otherTransform->position()[1];
  double otherMaxY = (otherTransform->position()[1] +
                      otherTransform->size()[1]);
  double otherWidth = otherTransform->size()[0];
  double otherHeight = otherTransform->size()[1];
  
  if (minX > otherMaxX)
  {
    return false;
  }
  
  if (maxX < otherMinX)
  {
    return false;
  }
  
  if (minY > otherMaxY)
  {
    return false;
  }
  
  if (maxY < otherMinY)
  {
    return false;
  }
  
  double x_overlap = (std::min<double>(maxX, otherMaxX) -
                      std::max<double>(minX, otherMinX));
  double y_overlap = (std::min<double>(maxY, otherMaxY) -
                      std::max<double>(minY, otherMinY));
  double overlap = x_overlap * y_overlap;
  double area = (maxX - minX) * (maxY - minY);
  double overlapArea = overlap/area;
  
  
  int sideOverlap = 0;
  if (maxX < otherMinX + (otherWidth * .5))
  {
    sideOverlap++;
  }
  
  if (otherMaxX < minX + (width * .5))
  {
    sideOverlap++;
  }
  
  if (maxY < otherMinY + (otherHeight * .5))
  {
    sideOverlap++;
  }
  
  if (otherMaxY < minY + (height * .5))
  {
    sideOverlap++;
  }
  
  if (sideOverlap > 1)
  {
    // it's a corner overlap
    return false;
  }
  
  if (overlapArea > 0.02)
  {
    return true;
  }
  
  return false;
}

//----------------------------------------------------------------
// IRTransform::trasformOverlapArea
// 
int
IRTransform::trasformOverlapArea(const IRTransform* otherTransform,
                                 pnt2d_t *start,
                                 vec2d_t *size) const
{
  float minX = _position[0];
  float maxX = _position[0] + _size[0];
  float minY = _position[1];
  float maxY = _position[1] + _size[1];
  
  float otherMinX = otherTransform->position()[0];
  float otherMaxX = otherTransform->position()[0] + otherTransform->size()[0];
  float otherMinY = otherTransform->position()[1];
  float otherMaxY = otherTransform->position()[1] + otherTransform->size()[1];
  
  float overlapMinX = maxX;
  float overlapMaxX = minX;
  float overlapMinY = maxY;
  float overlapMaxY = minY;
  
  overlapMinX = std::min<float>(maxX, otherMinX);
  overlapMaxX = std::max<float>(minX, otherMaxX);
  overlapMinY = std::min<float>(maxY, otherMinY);
  overlapMaxY = std::max<float>(minY, otherMaxY);
  
  overlapMinX = std::max<float>(minX, otherMinX);
  overlapMaxX = std::min<float>(maxX, otherMaxX);
  overlapMinY = std::max<float>(minY, otherMinY);
  overlapMaxY = std::min<float>(maxY, otherMaxY);
  
  if (overlapMinX == maxX ||
      overlapMaxX == minX ||
      overlapMinY == maxY ||
      overlapMaxY == minY)
  {
    return -1;
  }
  
  // make the area a it bigger than just the overlap.
  float overlapWidth = overlapMaxX - overlapMinX;
  float overlapHeigth = overlapMaxY - overlapMinY;
  overlapMinX = std::max<float>(overlapMinX - (1.0*overlapWidth), minX);
  overlapMaxX = std::min<float>(overlapMaxX + (1.0*overlapWidth), maxX);
  overlapMinY = std::max<float>(overlapMinY - (1.0*overlapHeigth), minY);
  overlapMaxY = std::min<float>(overlapMaxY + (1.0*overlapHeigth), maxY);
  
  (*start)[0] = overlapMinX - minX;
  (*start)[1] = overlapMinY - minY;
  
  (*size)[0] = overlapMaxX - overlapMinX;
  (*size)[1] = overlapMaxY - overlapMinY;
  
  return 0; // NO_ERR 
}

//----------------------------------------------------------------
// IRTransform::totalTension
// 
vec2d_t
IRTransform::totalTension()
{
  vec2d_t out;
  out[0] = out[1] = 0;
  
  for (IRConnectionVector::iterator iter = _connections.begin();
       iter != _connections.end(); iter++)
  {
    out += (*iter)->tensionOnTransformation(this);
  }
  
  return out;
}

//----------------------------------------------------------------
// IRTransform::releaseTension
// 
void
IRTransform::releaseTension()
{
  vec2d_t tension = totalTension();
  _position[0] += 0.2f*tension[0];
  _position[1] += 0.2f*tension[1];
}

//----------------------------------------------------------------
// IRTransform::addConnection
// 
void
IRTransform::addConnection(IRConnection* connection)
{
  _connections.push_back(connection);
}

//----------------------------------------------------------------
// IRTransform::removeConnection
// 
void
IRTransform::removeConnection(IRConnection* connection)
{
  for (size_t i = 0; i < _connections.size(); i++)
  {
    if (_connections[i] == connection)
    {
      _connections[i] = _connections.back();
      _connections.pop_back();
      return;
    }
  }
}

//----------------------------------------------------------------
// IRTransform::transformBase
// 
base_transform_t::Pointer
IRTransform::transformBase()
{
  itk::LegendrePolynomialTransform<double, 1>::Pointer newTransform =
  itk::LegendrePolynomialTransform<double, 1>::New();
  
  itk::LegendrePolynomialTransform<double, 1>::ParametersType
  variableParameters;
  variableParameters.SetSize(6);
  
  itk::LegendrePolynomialTransform<double, 1>::ParametersType
  fixedParameters;
  fixedParameters.SetSize(4);
  
  variableParameters[0] = 1;
  variableParameters[1] = 0;
  variableParameters[2] = 1;
  variableParameters[3] = 1;
  variableParameters[4] = 1;
  variableParameters[5] = 0;
  fixedParameters[0] = _position[0];
  fixedParameters[1] = _position[1];
  fixedParameters[2] = _size[0] / 2.0f;
  fixedParameters[3] = _size[1] / 2.0f;
  newTransform->SetParameters(variableParameters);
  newTransform->SetFixedParameters(fixedParameters);
  
  base_transform_t::Pointer basePointer(newTransform);
  return basePointer;
}

//----------------------------------------------------------------
// IRTransform::imageID
// 
const bfs::path &
IRTransform::imageID() const
{
  return _imageID;
}

//----------------------------------------------------------------
// IRTransform::imageID
// 
const bfs::path &
IRTransform::maskID() const
{
  return _maskID;
}

// TODO: stream instead

//----------------------------------------------------------------
// IRTransform::printImageID
//
void
IRTransform::printImageID( bool verbose )
{
  if ( verbose )
    printf("%s\n", _imageID.c_str());
}

//----------------------------------------------------------------
// IRTransform::printMaskID
// 
void
IRTransform::printMaskID( bool verbose )
{
  if ( verbose )
    printf("%s\n", _maskID.c_str());
}

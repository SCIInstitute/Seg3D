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

// File         : IRConnection.cxx
// Author       : Joel Spaltenstein
// Created      : 2008/03/11
// Copyright    : (C) 2008 University of Utah
// Description  :

// system includes:
#include <stdio.h>
#include <sstream>

// ITK includes:
#include <itkRegionOfInterestImageFilter.h>

// the includes:
#include <Core/ITKCommon/Transform/IRConnection.hxx>
#include <Core/ITKCommon/Transform/IRTransform.hxx>
#include <Core/ITKCommon/IRImageLoader.hxx>
#include <Core/ITKCommon/mosaic_layout_common.hxx>
#include <Core/ITKCommon/FFT/fft_common.hxx>


//----------------------------------------------------------------
// IRConnection::IRConnection
// 
IRConnection::IRConnection(IRTransform* firstTransform,
                           IRTransform* secondTransform,
                           bool verbose) : 
_firstTransformation(firstTransform),
_secondTransformation(secondTransform),
_matchValue(0.0),
_verbose(verbose)
{
	printMessage( "building an IRConnection between \n", _verbose );
	_firstTransformation->printImageID( _verbose );
  printMessage( "\n", _verbose );
	_secondTransformation->printImageID( _verbose );
  printMessage( "\n", _verbose );
	return;
}

//----------------------------------------------------------------
// IRConnection::~IRConnection
// 
IRConnection::~IRConnection()
{
	return;
}

//----------------------------------------------------------------
// IRConnection::tensionOnTransformation
// 
vec2d_t
IRConnection::tensionOnTransformation(IRTransform * transform)
{
	vec2d_t tension;
	if (transform != _firstTransformation &&
      transform != _secondTransformation)
	{
    printMessage( "IRConnection::tensionOnTransformation: "
                 "Transform can't be found.\n", _verbose );
		tension[0] = 0;
		tension[1] = 0;
		return tension;
	}
	
	tension = (_secondTransformation->position() -
             _firstTransformation->position()) - _idealTransormationOffset; 
	
	if (transform == _secondTransformation)
	{
		tension[0] = -1*tension[0];
		tension[1] = -1*tension[1];
	}
	
	tension[0] = tension[0] * (_matchValue*_matchValue);
	tension[1] = tension[1] * (_matchValue*_matchValue);
	return tension;
}

//----------------------------------------------------------------
// IRConnection::idealTransormationOffset
// 
vec2d_t
IRConnection::idealTransormationOffset()
{
	return _idealTransormationOffset;
}

//----------------------------------------------------------------
// IRConnection::findIdealOffset
// 
int
IRConnection::findIdealOffset(const double maxOffset[], 
                              bool maxOffsetIsPercent,
                              const double blackMaskPercent[],
                              bool doClahe
                              /*const double overlap[]*/)
{
	translate_transform_t::Pointer translationTransform;
	mask_t::ConstPointer mask1;// = IRImageLoader::sharedImageLoader()->
  //getMask(_firstTransformation->maskID());
  mask_t::ConstPointer mask2;// = IRImageLoader::sharedImageLoader()->
  //getMask(_secondTransformation->maskID());
	
	image_t::Pointer image1 = IRImageLoader::sharedImageLoader()->
  getImage(_firstTransformation->imageID());
	image_t::Pointer image2 = IRImageLoader::sharedImageLoader()->
  getImage(_secondTransformation->imageID());
  
  image_t::RegionType region = image1->GetLargestPossibleRegion();
  image_t::SizeType size = region.GetSize();
  
  if ( blackMaskPercent[0] != std::numeric_limits<double>::max() ||
      blackMaskPercent[1] != std::numeric_limits<double>::max() )
  {
    // Fill image1 center with black.
    image_t::IndexType index = region.GetIndex();
    if ( blackMaskPercent[0] != std::numeric_limits<double>::max() )
    {
      index[0] += blackMaskPercent[0] * size[0];
      size[0] *= 1.0 - (blackMaskPercent[0] * 2);
    }
    if ( blackMaskPercent[1] != std::numeric_limits<double>::max() )
    {
      index[1] += blackMaskPercent[1] * size[1];
      size[1] *= 1.0 - (blackMaskPercent[1] * 2);
    }
    
    fill<image_t>(image1, index[0], index[1], size[0], size[1]);
    
    // Fill image2 center with black.
    region = image2->GetLargestPossibleRegion();
    size = region.GetSize();
    index = region.GetIndex();
    if ( blackMaskPercent[0] != std::numeric_limits<double>::max() )
    {
      index[0] += blackMaskPercent[0] * size[0];
      size[0] *= 1.0 - (blackMaskPercent[0] * 2);
    }
    if ( blackMaskPercent[1] != std::numeric_limits<double>::max() )
    {
      index[1] += blackMaskPercent[1] * size[1];
      size[1] *= 1.0 - (blackMaskPercent[1] * 2);
    }
    
    fill<image_t>(image2, index[0], index[1], size[0], size[1]);
  }
  
  image_t::PointType offset_min;
  offset_min[0] = -std::numeric_limits<double>::max();
  offset_min[1] = -std::numeric_limits<double>::max();
  image_t::PointType offset_max;
  offset_max[0] = std::numeric_limits<double>::max();
  offset_max[1] = std::numeric_limits<double>::max();
  
  pnt2d_t pos1 = _firstTransformation->position();
  pnt2d_t pos2 = _secondTransformation->position();
  
  // Limit the transformation distance.
  if ( maxOffset[0] != std::numeric_limits<double>::max() )
  {
    int xOffset = (maxOffsetIsPercent) ? (maxOffset[0] * size[0]) : maxOffset[0];
    offset_min[0] = (pos1[0] - pos2[0]) - xOffset;
    offset_max[0] = (pos1[0] - pos2[0]) + xOffset;
  }
  if ( maxOffset[1] != std::numeric_limits<double>::max() )
  {
    int yOffset = (maxOffsetIsPercent) ? (maxOffset[1] * size[1]) : maxOffset[1];
    offset_min[1] = (pos1[1] - pos2[1]) - yOffset;
    offset_max[1] = (pos1[1] - pos2[1]) + yOffset;
  }
  
	vec2d_t tension;
	int match_attempt = 0;
	bool match_success = false;
	double match_value = 0;
  int total_attempts = doClahe ? 2 : 1;
	
	while (match_success == false && match_attempt < total_attempts)
	{
		switch (match_attempt)
		{
			case 0:
			{
				match_value =
        match_one_pair<image_t, mask_t>(false, // images_were_resampled
                                        false, // use_std_mask,
                                        image1, // fi,
                                        image2, // mi,
                                        mask1, // fi_mask,
                                        mask2, // mi_mask,
                                        .02, // overlap_min,
                                        .70, // overlap_max,
                                        offset_min,
                                        offset_max,
                                        translationTransform // ti
                                        );
				break;
			}
        
			case 1:
			default:
			{
				translate_transform_t::Pointer claheTranslationTransform;
				image2 = smooth<image_t>(image2, 2 /*sigma*/);
				image1 = CLAHE<image_t>(image1,
                                32 /*nx*/,
                                32 /*ny*/,
                                3 /*max_slope*/,
                                256 /*bins*/,
                                0 /*new_min*/,
                                255 /*new_max*/,
                                mask1);
				
				image2 = CLAHE<image_t>(image2,
                                32 /*nx*/,
                                32 /*ny*/,
                                3 /*max_slope*/,
                                256 /*bins*/,
                                0 /*new_min*/,
                                255 /*new_max*/,
                                mask1);
				
				match_value = match_one_pair<image_t, mask_t>
        (false, // resampled
         false, // use_std_mask,
         image1, // fi,
         image2, // mi,
         mask1, // fi_mask,
         mask2, // mi_mask,
         .02, // overlap_min,
         .70, // overlap_max,
         offset_min,
         offset_max,
         claheTranslationTransform // ti
         );
				
				translationTransform = claheTranslationTransform;
				break;
			}
		}
		
		if (!translationTransform.IsNull())
		{
			_idealTransormationOffset[0] = -1.0 * translationTransform->
      GetOffset()[0];
			_idealTransormationOffset[1] = -1.0 * translationTransform->
      GetOffset()[1];
			_matchValue = match_value;
			tension = tensionOnTransformation(_firstTransformation);
		}
		
		if (translationTransform.IsNull() ||
        (translationTransform->GetOffset()[0] == 0.0f &&
         translationTransform->GetOffset()[1] == 0.0f))
		{
      printMessage( "Could not build transform ", _verbose );
			if (match_attempt == 1)
			{
        printMessage( "Using clahe ", _verbose );
			}
			
      printMessage( "between ", _verbose );
			_firstTransformation->printImageID( _verbose );
      printMessage( " and ", _verbose );
			_secondTransformation->printImageID( _verbose );
      printMessage( "\n", _verbose );
			match_success = false;
		}
		else if (sqrtf(tension*tension) > ((_firstTransformation->size()[0] +
                                        _firstTransformation->size()[1]) /
                                       (2.0 * 4.0)))
		{
			// if the distance the image is moved is more than
			// a quarter of the average of the two sides
      printMessage( "A displacement between ", _verbose );
			_firstTransformation->printImageID( _verbose );
			printMessage( " and ", _verbose );
			_secondTransformation->printImageID( _verbose );
			if (match_attempt == 1)
			{
			  printMessage( " using clahe", _verbose );
			}
      
      std::ostringstream msg;
      msg << " of " << the_text_t::number( _idealTransormationOffset[0] ) << ", "
      << the_text_t::number( _idealTransormationOffset[1] ) << " is too great\n";
      printMessage( msg.str().c_str(), _verbose );
			match_success = false;
		}
		else
		{
			printMessage( "the offset between ", _verbose );
			_firstTransformation->printImageID( _verbose );
			printMessage( " and ", _verbose );
			_secondTransformation->printImageID( _verbose );
      
      std::ostringstream msg;
      msg << " is " << the_text_t::number( _idealTransormationOffset[0] ) << ", "
      << the_text_t::number( _idealTransormationOffset[1] ) << " with a match value of "
      << the_text_t::number( match_value ) << std::endl;
      printMessage( msg.str().c_str(), _verbose );
			match_success = true;
		}
    
		match_attempt++;
	}
	
	if (!match_success)
	{
    printMessage( "Gave up on finding a connection between ", _verbose );
		_firstTransformation->printImageID( _verbose );
    printMessage( " and ", _verbose );
		_secondTransformation->printImageID( _verbose );
    printMessage( "\n", _verbose );
		return -1;
	}
  
	return 0;
}

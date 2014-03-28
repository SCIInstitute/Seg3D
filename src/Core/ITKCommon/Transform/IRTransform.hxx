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

// File         : IRTransform.hxx
// Author       : Joel Spaltenstein
// Created      : 2008/02/19
// Copyright    : (C) 2008 University of Utah
// Description  : 

#ifndef __IR_TRANSFORMATION_HXX__
#define __IR_TRANSFORMATION_HXX__

// system includes:
#include <vector>

// the includes:
#include <Core/ITKCommon/common.hxx>

// ITK includes:
#include <itkTransformBase.h>

// forward declarations:
class IRConnection;


//----------------------------------------------------------------
// IRTransform
// 
class IRTransform
{
	typedef std::vector<IRConnection*> IRConnectionVector;
  
public:
	IRTransform(base_transform_t::Pointer transformBase,
              const std::string& imageID,
              const std::string& maskID);
	virtual ~IRTransform();
	
	const pnt2d_t & position() const;
	const vec2d_t & size() const;
	
	long groupID() const;
	void setGroupID(long newID);
	
	const IRConnectionVector & connections() const;
	
	bool overlapsTransform(const IRTransform* otherTransform) const;
	
	// sets start and size to the area that is covered by otherTransfrom,
	// the value returned by this function are with respect the the corner of
	// the transformation.
	// if they don't overlap at all this function returns an error (-1)
	int trasformOverlapArea(const IRTransform* otherTransform,
                          pnt2d_t *start,
                          vec2d_t *size) const;
	
	vec2d_t totalTension();
	void releaseTension();
	
	
	void addConnection(IRConnection* connection);
	void removeConnection(IRConnection* connection);
	base_transform_t::Pointer transformBase();
	const std::string & imageID() const;
	const std::string & maskID() const;
  
	// DEBUG stuff
	void printImageID( bool verbose );
	void printMaskID( bool verbose );
  
private:
	IRConnectionVector _connections;
	base_transform_t::Pointer _transformBase;
	std::string _imageID;
  std::string _maskID;
	
	pnt2d_t _position;
	vec2d_t _size;
  
	// all tiles that are connected share the same groupID once this
	long _groupID;
};


#endif // __IR_TRANSFORMATION_HXX__

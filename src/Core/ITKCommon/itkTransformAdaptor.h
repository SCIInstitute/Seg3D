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

#ifndef CORE_ITKCOMMON_ITKTRANSFORMADAPTER_H
#define CORE_ITKCOMMON_ITKTRANSFORMADAPTER_H

#include <itkTransform.h>

namespace itk
{

template <class TScalarType, unsigned int NInputDimensions=3,  unsigned int NOutputDimensions=3>
class ITK_EXPORT TransformAdapter : public Transform<TScalarType,NInputDimensions,NOutputDimensions>
{
public:
  typedef TransformAdapter Self;
  typedef Transform<TScalarType,NInputDimensions,NOutputDimensions>    Superclass;
  typedef SmartPointer< Self >                              Pointer;
  typedef SmartPointer< const Self >                        ConstPointer;
  
  /** Base inverse transform type. */
  typedef Transform< TScalarType, NOutputDimensions, NInputDimensions > InverseTransformType;
  
  typedef SmartPointer< InverseTransformType > InverseTransformPointer;
  
  /** New method for creating an object using a factory. */
  itkNewMacro(Self);
  
  /** Run-time type information (and related methods). */
  itkTypeMacro( TransformAdapter, Transform );
  
  /** Return an inverse of this transform. If the inverse has not been
   *  implemented, return NULL. The type of the inverse transform
   *  does not necessarily need to match the type of the forward
   *  transform. This allows one to return a numeric inverse transform
   *  instead.
   */
  virtual InverseTransformPointer GetInverse() const
  { return 0; }

protected:
  TransformAdapter():Transform<TScalarType,NInputDimensions,NOutputDimensions>(3,1) 
  {
  } 
  virtual ~TransformAdapter() {}

private:
  TransformAdapter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented
};

}

#endif
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

#ifndef VTKFASTGROWCUT_H
#define VTKFASTGROWCUT_H

//#include "vtkSlicerFastGrowCutSegmenterModuleLogicExport.h"
#include "vtkImageData.h"
#include "FastGrowCut.h"

#include "vtkObject.h"


//#include <QProgressBar>
//#include <QMainWindow>
//#include <QStatusBar>
//#include "qSlicerApplication.h"

const unsigned short SrcDimension = 3;
typedef float DistPixelType; // float type pixel for cost function
typedef short SrcPixelType;
typedef unsigned char LabPixelType;

class vtkFastGrowCut : public vtkObject
{
public:
  static vtkFastGrowCut* New();
  //vtkTypeRevisionMacro( vtkFastGrowCut, vtkObject );
  vtkTypeMacro(vtkFastGrowCut,vtkObject);

  //set parameters of grow cut
  vtkSetObjectMacro( SourceVol, vtkImageData );
  vtkSetObjectMacro( SeedVol, vtkImageData );
  //vtkSetObjectMacro(OutputVol, vtkImageData);

  vtkSetMacro( InitializationFlag, bool );

  //processing functions
  void Initialization();
  void RunFGC();
  void PrintSelf( ostream &os, vtkIndent indent );

protected:
  vtkFastGrowCut();
  virtual ~vtkFastGrowCut();

private:
  //vtk image data (from slicer)
  vtkImageData* SourceVol;
  vtkImageData* SeedVol;

  std::vector<LabPixelType> m_imSeedVec;
  std::vector<LabPixelType> m_imLabVec;
  std::vector<SrcPixelType> m_imSrcVec;
  std::vector<long> m_imROI;

  //logic code
  FGC::FastGrowCut<SrcPixelType, LabPixelType>* m_fastGC;

  //state variables
  bool InitializationFlag;
};
#endif // ifndef VTKFASTGROWCUT_H

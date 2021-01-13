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

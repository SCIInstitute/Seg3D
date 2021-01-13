#include <iostream>
#include "vtkFastGrowCut.h"

#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"

#include "itkImage.h"
#include "itkTimeProbe.h"

//#include "FastGrowCut.h"

//vtkCxxRevisionMacro( vtkFastGrowCut, "$Revision$" ); //necessary?
vtkStandardNewMacro( vtkFastGrowCut ); //for the new() macro

//----------------------------------------------------------------------------

vtkFastGrowCut::vtkFastGrowCut()
{

  SourceVol = NULL;
  SeedVol = NULL;
  m_fastGC = NULL;
}

vtkFastGrowCut::~vtkFastGrowCut()
{

  //these functions decrement reference count on the vtkImageData's (incremented by the SetMacros)
  if ( this->SourceVol )
  {
    this->SetSourceVol( NULL );
  }

  if ( this->SeedVol )
  {
    this->SetSeedVol( NULL );
  }

  if ( m_fastGC != NULL )
  {
    delete m_fastGC;
  }
}

void vtkFastGrowCut::Initialization()
{

  InitializationFlag = false;
  if ( m_fastGC == NULL )
  {
    m_fastGC = new FGC::FastGrowCut<SrcPixelType, LabPixelType>();
  }
}

void vtkFastGrowCut::RunFGC()
{

  itk::TimeProbe timer;

  timer.Start();

  //QProgressBar* computationProgressBar =  new QProgressBar;
  //qSlicerApplication::application()->mainWindow()->statusBar()->addPermanentWidget(computationProgressBar);
  std::cerr << "InitializationFlag: " << InitializationFlag << std::endl;
  // Find ROI
  if ( !InitializationFlag )
  {
    FGC::FindVTKImageROI<LabPixelType>( SeedVol, m_imROI );
    std::cerr << "image ROI = [" << m_imROI[0] << "," << m_imROI[1] << "," << m_imROI[2] << ";"  \
              << m_imROI[3] << "," << m_imROI[4] << "," << m_imROI[5] << "]" << std::endl;
    // SB: Find the ROI from the seed volume in the source volume and store it in m_imSrcVec
    FGC::ExtractVTKImageROI<SrcPixelType>( SourceVol, m_imROI, m_imSrcVec );
  }
  // SB: Store the ROI from the seed volume in m_imSeedVec
  FGC::ExtractVTKImageROI<LabPixelType>( SeedVol, m_imROI, m_imSeedVec );

  // Initialize FastGrowCut
  std::vector<long> imSize( 3 );
  for ( int i = 0; i < 3; i++ )
  {
    imSize[i] = m_imROI[i + 3] - m_imROI[i];
  }
  m_fastGC->SetSourceImage( m_imSrcVec );
  m_fastGC->SetSeedlImage( m_imSeedVec );
  m_fastGC->SetImageSize( imSize );
  m_fastGC->SetWorkMode( InitializationFlag );

//     computationProgressBar->setValue(10);

  // Do Segmentation
  m_fastGC->DoSegmentation();
  m_fastGC->GetForegroundmage( m_imLabVec );

//    computationProgressBar->setValue(90);

  // Update result. SB: Seed volume is replaced with grow cut result
  FGC::UpdateVTKImageROI<LabPixelType>( m_imLabVec, m_imROI, SeedVol );

//    computationProgressBar->setValue(100);
//delete computationProgressBar;
  timer.Stop();

  if ( !InitializationFlag )
  {
    std::cout << "Initial fast GrowCut segmentation time: " << timer.GetMeanTime() << " seconds\n";
  }
  else
  {
    std::cout << "Adaptive fast GrowCut segmentation time: " << timer.GetMeanTime() << " seconds\n";
  }
}

void vtkFastGrowCut::PrintSelf( ostream &os, vtkIndent indent )
{
  std::cout << "This function has been found" << std::endl;
}

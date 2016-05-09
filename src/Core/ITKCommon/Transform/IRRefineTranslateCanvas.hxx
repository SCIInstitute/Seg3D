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

// File         : IRRefineTranslateCanvas.hxx
// Author       : Joel Spaltenstein
// Created      : 2008/02/20
// Copyright    : (C) 2008 University of Utah
// Description  :

#ifndef __IR_REFINE_TRANSLATE_CANVAS_HXX__
#define __IR_REFINE_TRANSLATE_CANVAS_HXX__

// system includes:
#include <vector>
#include <list>

// ITK includes:
#include <itkTransformBase.h>

#include <Core/ITKCommon/common.hxx>

// boost:
#include <boost/filesystem.hpp>

namespace bfs=boost::filesystem;

// forward declarations:
class IRTransform;
class IRConnection;

//----------------------------------------------------------------
// IRConnectionVector
// 
typedef std::vector<IRConnection*> IRConnectionVector;


//----------------------------------------------------------------
// IRRefineTranslateCanvas
// 
class IRRefineTranslateCanvas
{
  typedef std::vector<IRTransform*> IRTransformationVector;
  typedef std::vector<base_transform_t::Pointer> TrasformBasePointerVector;
  typedef std::vector<bfs::path> TheTextVector;
  typedef std::list<bfs::path> TheTextList;
  
public:
  
  IRRefineTranslateCanvas();
  virtual ~IRRefineTranslateCanvas();
  
  // Refuse offsets bigger than this number.
  void setMaxOffset(const double maxOffset[], bool isPercent);
  
  // Black out the center to force better fft matchings.
  void setBlackMaskPercent(const double blackMaskPercent[]);
  
  // takes ownership of the trans
  void setBaseTransforms(const TrasformBasePointerVector& transforms,
                         const TheTextVector& imageIDs,
                         const TheTextVector& maskIDs);
  
  // Should we try a clahe run if the other run fails?
  void doClahe( bool doClahe );
  
  void fillTransformAndImageIDVectors(TrasformBasePointerVector& transforms,
                                      TheTextVector& imageIDs,
                                      TheTextVector& maskIDs);
  
  // silly other version that uses a list because fo some reason
  // paul likes using lists for the list of image IDs........
  void fillTransformAndImageIDVectors(TrasformBasePointerVector& transforms,
                                      TheTextList& imageIDs,
                                      TheTextList& maskIDs);
  
  // based on the current positions of each of the transforms
  // (which, when this is called will be the positions in the
  // .mosaic file that was loaded) this routine creates IRConnections
  // for each overlap and puts them in _preProcessedConnectionVector,
  // the connections are not added to the individual transforms
  // until findIdealTransformationOffsets is called
  void buildConnections( bool verbose = false );
  
  // initializes the groupIDs so that all the connected transforms
  // have the same groupID
  void fillGroupIDs();
  
  // removes transforms that are in a group that make up less than
  // cutoffPercentage of the mosaic
  void pruneSmallGroups(float cutoffPercentage);
  
  void findIdealTransformationOffsets(unsigned int numThreads);
  
  void releaseTensionOnSystem();
  
  float systemEnergy();
  float maximumTension();
  float maximumPullOnTransformation();
  
  // what is the signature of a thread entry again? keep on trying t
  void findOffsetsThreadEntry();
  
private:
  void addProcessedConnection(IRConnection* connection);
  
  // returns NULL if there are no more entries to process
  IRConnection* getConectionToProcess();
  
  void setTransformAndNeighborsToGroupID(IRTransform* transform,
                                         long groupID);
  
  void removeTransformsWithGroupID(long groupID);
  
  // will be populated by buildConnections()
  IRConnectionVector _preProcessedConnectionVector;
  
  IRTransformationVector _transformationVector;
  IRConnectionVector _connectionVector;
  
  double initialSize;
  
  bool   maxOffsetIsPercent;
  double maxOffset[2];
  double blackMaskPercent[2];
  bool   _doClahe;
};


#endif // __IR_REFINE_TRANSLATE_CANVAS_HXX__

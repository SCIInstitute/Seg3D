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

// File         : IRRefineTranslateCanvas.cxx
// Author       : Joel Spaltenstein
// Created      : 2008/02/20
// Copyright    : (C) 2008 University of Utah
// Description  :

// system includes:
#include <math.h>
#include <algorithm>
#include <stack>

// the includes:
#include <Core/ITKCommon/ThreadUtils/the_thread_pool.hxx>
#include <Core/ITKCommon/Transform/IRRefineTranslateCanvas.hxx>
#include <Core/ITKCommon/Transform/IRTransform.hxx>
#include <Core/ITKCommon/Transform/IRConnection.hxx>


//----------------------------------------------------------------
// IRRefineTranslateCanvas::IRRefineTranslateCanvas
// 
IRRefineTranslateCanvas::IRRefineTranslateCanvas()
{
  this->maxOffset[0] = std::numeric_limits<double>::max();
  this->maxOffset[1] = std::numeric_limits<double>::max();
  maxOffsetIsPercent = false;
  _doClahe = true;
}


//----------------------------------------------------------------
// IRRefineTranslateCanvas::~IRRefineTranslateCanvas
// 
IRRefineTranslateCanvas::~IRRefineTranslateCanvas()
{}

//----------------------------------------------------------------
// IRRefineTranslateCanvas::setBaseTransforms
// 
void
IRRefineTranslateCanvas::
setBaseTransforms(const TrasformBasePointerVector & transforms,
                  const TheTextVector& imageIDs,
                  const TheTextVector& maskIDs)
{
	TrasformBasePointerVector::const_iterator transformIter =
  transforms.begin();
	
	TheTextVector::const_iterator imageIDIter = imageIDs.begin();
	TheTextVector::const_iterator maskIDIter = maskIDs.begin();
	for (; transformIter != transforms.end(); transformIter++, imageIDIter++, maskIDIter++)
	{
		_transformationVector.
    push_back(new IRTransform(*transformIter, *imageIDIter, *maskIDIter));
	}
}
//----------------------------------------------------------------
// IRRefineTranslateCanvas::setMaxOffset
// 
void
IRRefineTranslateCanvas::setMaxOffset(const double maxOffset[], bool isPercent)
{
  this->maxOffset[0] = maxOffset[0];
  this->maxOffset[1] = maxOffset[1];
  this->maxOffsetIsPercent = isPercent;
}

//----------------------------------------------------------------
// IRRefineTranslateCanvas::setBlackMaskPercent
// 
void
IRRefineTranslateCanvas::setBlackMaskPercent(const double blackMaskPercent[])
{
  this->blackMaskPercent[0] = blackMaskPercent[0];
  this->blackMaskPercent[1] = blackMaskPercent[1];
}

//----------------------------------------------------------------
// IRRefineTranslateCanvas::doClahe
// 
void
IRRefineTranslateCanvas::doClahe( bool _doClahe )
{
  this->_doClahe = _doClahe;
}


//----------------------------------------------------------------
// IRRefineTranslateCanvas::fillTransformAndImageIDVectors
// 
void
IRRefineTranslateCanvas::
fillTransformAndImageIDVectors(TrasformBasePointerVector& transforms,
                               TheTextVector& imageIDs,
                               TheTextVector& maskIDs)
{
	transforms.clear();
	imageIDs.clear();
	maskIDs.clear();
	for (IRTransformationVector::iterator iter = _transformationVector.begin();
       iter != _transformationVector.end(); ++iter)
	{
		imageIDs.push_back((*iter)->imageID());
		maskIDs.push_back((*iter)->maskID());
		transforms.push_back((*iter)->transformBase());
	}
}

//----------------------------------------------------------------
// IRRefineTranslateCanvas::fillTransformAndImageIDVectors
// 
// silly other version that uses a list because fo some reason
// paul likes using lists for the image IDs........
// 
void
IRRefineTranslateCanvas::
fillTransformAndImageIDVectors(TrasformBasePointerVector& transforms,
                               TheTextList& imageIDs,
                               TheTextList& maskIDs)
{
	transforms.clear();
	imageIDs.clear();
  maskIDs.clear();
	for (IRTransformationVector::iterator iter = _transformationVector.begin();
       iter != _transformationVector.end(); ++iter)
	{
    if ( (*iter)->connections().size() != 0 )
    {
		  imageIDs.push_back((*iter)->imageID());
		  maskIDs.push_back((*iter)->maskID());
		  transforms.push_back((*iter)->transformBase());
    }
	}
}

//----------------------------------------------------------------
// IRRefineTranslateCanvas::buildConnections
// 
void
IRRefineTranslateCanvas::buildConnections( bool verbose )
{
	_preProcessedConnectionVector.clear();
  
	IRTransformationVector::const_iterator transformIter1, transformIter2;
	for (transformIter1 = _transformationVector.begin();
       transformIter1 != _transformationVector.end(); transformIter1++)
	{
		for (transformIter2 = transformIter1;
         transformIter2 != _transformationVector.end(); transformIter2++)
		{
			if (transformIter1 == transformIter2)
			{
				continue;
			}
			
			if ((*transformIter1)->overlapsTransform(*transformIter2))
			{
        
				_preProcessedConnectionVector.push_back(new IRConnection(*transformIter1,
                                                                 *transformIter2,
                                                                 verbose));
        initialSize = _preProcessedConnectionVector.size();
			}
		}
	}
	
	return;
}

//----------------------------------------------------------------
// IRRefineTranslateCanvas::fillGroupIDs
// 
void
IRRefineTranslateCanvas::fillGroupIDs()
{
	long groupID = 0;
	for (IRTransformationVector::iterator iter = _transformationVector.begin();
       iter != _transformationVector.end(); ++iter)
	{
		if ((*iter)->groupID() == -1)
		{
			setTransformAndNeighborsToGroupID((*iter), groupID);
			groupID++;
		}
	}
}

//----------------------------------------------------------------
// IRRefineTranslateCanvas::pruneSmallGroups
//
// removes transforms that are in a group that make up
// less than cutoffPercentage of the mosaic
// 
void
IRRefineTranslateCanvas::pruneSmallGroups(float cutoffPercentage)
{
	long largestGroupID = -1;
	for (IRTransformationVector::iterator iter = _transformationVector.begin();
       iter != _transformationVector.end(); ++iter)
	{
		largestGroupID = std::max<long>(largestGroupID, (*iter)->groupID());
	}
	
	std::vector<float> groupCounts;
	groupCounts.resize(largestGroupID + 1);
	for (std::vector<float>::iterator iter = groupCounts.begin();
       iter != groupCounts.end(); ++iter)
	{
		*iter = 0.0f;
	}
	
	for (IRTransformationVector::iterator iter = _transformationVector.begin();
       iter != _transformationVector.end(); ++iter)
	{
		groupCounts[(*iter)->groupID()] += 1.0f;
	}
	
	float transformCount = _transformationVector.size();
	
	for (size_t i = 0; i < groupCounts.size(); i++)
	{
		if (groupCounts[i] / transformCount < cutoffPercentage)
		{
			removeTransformsWithGroupID(i);
		}
	}
}

//----------------------------------------------------------------
// IRTransaction
// 
class IRTransaction : public the_transaction_t
{
public:
	IRTransaction(IRRefineTranslateCanvas * canvas):
  canvas_(canvas)
	{}
  
	// virtual:
	void execute(the_thread_interface_t * thread)
	{
		the_terminator_t terminator("IRTransaction");
		canvas_->findOffsetsThreadEntry();
	}
	
	IRRefineTranslateCanvas * canvas_;
};

//----------------------------------------------------------------
// IRRefineTranslateCanvas::findIdealTransformationOffsets
// 
void
IRRefineTranslateCanvas::findIdealTransformationOffsets(unsigned int numThreads)
{
	the_thread_pool_t thread_pool(numThreads);
	thread_pool.set_idle_sleep_duration(50); // 50 usec
	
	for(unsigned int i = 0; i < numThreads; i++)
	{
		IRTransaction * t = new IRTransaction(this);
		thread_pool.push_back(t);
	}
	
	thread_pool.start();
	thread_pool.wait();
}

//----------------------------------------------------------------
// IRRefineTranslateCanvas::releaseTensionOnSystem
// 
void
IRRefineTranslateCanvas::releaseTensionOnSystem()
{
	for (IRTransformationVector::iterator iter = _transformationVector.begin();
       iter != _transformationVector.end(); ++iter)
	{
		(*iter)->releaseTension();
	}
}

//----------------------------------------------------------------
// IRRefineTranslateCanvas::systemEnergy
// 
float
IRRefineTranslateCanvas::systemEnergy()
{
	float energy = 0;
	for (IRConnectionVector::iterator iter = _connectionVector.begin();
       iter != _connectionVector.end(); ++iter)
	{
		vec2d_t tension =
    (*iter)->tensionOnTransformation((*iter)->firstTransformation());
		
		energy += tension*tension;
	}
	return energy;
}

//----------------------------------------------------------------
// IRRefineTranslateCanvas::maximumTension
// 
float
IRRefineTranslateCanvas::maximumTension()
{
	float maxTension = 0;
	for (IRConnectionVector::iterator iter = _connectionVector.begin();
       iter != _connectionVector.end(); ++iter)
	{
		vec2d_t tension =
    (*iter)->tensionOnTransformation((*iter)->firstTransformation());
		
		maxTension = std::max<float>(sqrtf(tension*tension), maxTension);
	}
	return maxTension;
}

//----------------------------------------------------------------
// IRRefineTranslateCanvas::maximumPullOnTransformation
// 
float
IRRefineTranslateCanvas::maximumPullOnTransformation()
{
	float maxTension = 0;
	for (IRTransformationVector::iterator iter = _transformationVector.begin();
       iter != _transformationVector.end(); ++iter)
	{
		vec2d_t tension = (*iter)->totalTension();
		maxTension = std::max<float>(sqrtf(tension*tension), maxTension);
	}
	return maxTension;
}

//----------------------------------------------------------------
// __IRRefineTranslateCanvasConnectionLock
// 
static itk::SimpleMutexLock __IRRefineTranslateCanvasConnectionLock;

//----------------------------------------------------------------
// IRRefineTranslateCanvas::findOffsetsThreadEntry
// 
void
IRRefineTranslateCanvas::findOffsetsThreadEntry()
{
	while (true)
	{
		IRConnection * connection = getConectionToProcess();
		if (!connection) break;
		
		if (connection->findIdealOffset(maxOffset, maxOffsetIsPercent, blackMaskPercent, _doClahe) == 0)
		{
			connection->firstTransformation()->addConnection(connection);
			connection->secondTransformation()->addConnection(connection);
			addProcessedConnection(connection);
		}
	}
}

//----------------------------------------------------------------
// IRRefineTranslateCanvas::addProcessedConnection
// 
void
IRRefineTranslateCanvas::addProcessedConnection(IRConnection* connection)
{
	__IRRefineTranslateCanvasConnectionLock.Lock();
	_connectionVector.push_back(connection);
	__IRRefineTranslateCanvasConnectionLock.Unlock();
	return;
}

//----------------------------------------------------------------
// IRRefineTranslateCanvas::getConectionToProcess
//
// returns NULL if there are no more entries to process
// 
IRConnection*
IRRefineTranslateCanvas::getConectionToProcess()
{
  double vectorSize = 1;
  
	__IRRefineTranslateCanvasConnectionLock.Lock();
	IRConnection* connection = NULL;
	if (_preProcessedConnectionVector.size() > 0)
	{
		// by picking out  random connection we have less of a chance
		// to keep on picking connection with the same trasformation
		// and thus not having good multithreaded performance because
		// multiple threads are waiting for the same image to load
		int element;
		element = rand() % _preProcessedConnectionVector.size();
		connection = _preProcessedConnectionVector[element];
		
		_preProcessedConnectionVector[element] =
    _preProcessedConnectionVector.back();
		
		_preProcessedConnectionVector.pop_back();
    
    vectorSize = _preProcessedConnectionVector.size();
    
		// for now give them in order since the tiles these
		// are now preloaded and are BIG
		//		connection = _preProcessedConnectionVector.back();
		//		_preProcessedConnectionVector.pop_back();
	}
	__IRRefineTranslateCanvasConnectionLock.Unlock();
  
  double task_percent = (initialSize - vectorSize) / initialSize;
  set_minor_progress(task_percent, 0.9);
	return connection;
}

//----------------------------------------------------------------
// IRRefineTranslateCanvas::setTransformAndNeighborsToGroupID
// 
void
IRRefineTranslateCanvas::
setTransformAndNeighborsToGroupID(IRTransform* transform, long groupID)
{
	std::stack<IRTransform*> transformStack;
	
	transformStack.push(transform);
	
	while (!transformStack.empty())
	{
		IRTransform* currentTransform = transformStack.top();
		transformStack.pop();
		
		currentTransform->setGroupID(groupID);
		
		IRConnectionVector connections = currentTransform->connections();
		for (IRConnectionVector::iterator iter = connections.begin();
         iter != connections.end(); ++iter)
		{
			IRTransform* neighbor = (*iter)->firstTransformation();
			if (currentTransform == neighbor)
			{
				neighbor = (*iter)->secondTransformation();
			}
			
			if (neighbor->groupID() != groupID)
			{
				if (neighbor->groupID() != -1)
				{
					printf("Error! why was this groupID aready set!");
				}
				
				transformStack.push(neighbor);
			}
		}
	}	
}

//----------------------------------------------------------------
// IRRefineTranslateCanvas::removeTransformsWithGroupID
// 
void
IRRefineTranslateCanvas::removeTransformsWithGroupID(long groupID)
{
	IRTransformationVector newTransVector;
	for (IRTransformationVector::iterator transIter =
			 _transformationVector.begin();
       transIter != _transformationVector.end();
       ++transIter)
	{
		if ((*transIter)->groupID() == groupID)
		{
			IRConnectionVector connections = (*transIter)->connections();
			for (IRConnectionVector::iterator connIter = connections.begin();
           connIter != connections.end(); ++connIter)
			{
				IRTransform* neighbor = (*connIter)->firstTransformation();
				if ((*transIter) == neighbor)
				{
					neighbor = (*connIter)->secondTransformation();
				}
				
				neighbor->removeConnection(*connIter);
				(*transIter)->removeConnection(*connIter);
        
				for (size_t i = 0; i < _connectionVector.size(); i++)
				{
					if (_connectionVector[i] == *connIter)
					{
						_connectionVector[i] = _connectionVector.back();
						_connectionVector.pop_back();
						break;
					}
				}
        
				delete *connIter;
			}
			delete *transIter;
		}
		else
		{
			newTransVector.push_back(*transIter);
		}
	}
	_transformationVector = newTransVector;
}
